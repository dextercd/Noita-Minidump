#include <cstring>
#include <ctime>
#include <iterator>
#include <charconv>
#include <iostream>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>

#include "dump_communication.hpp"

HANDLE read_env_handle(const char* envvar)
{
    auto envval = std::getenv(envvar);
    if (!envval)
        std::cerr << "Environment variable: " <<envvar << " does not exist.\n";

    std::uintptr_t handle_int = 0;
    std::from_chars(envval, envval + std::strlen(envval), handle_int);
    return reinterpret_cast<HANDLE>(handle_int);
}

DWORD read_env_dword(const char* envvar)
{
    auto envval = std::getenv(envvar);
    if (!envval)
        std::cerr << "Environment variable: " <<envvar << " does not exist.\n";

    DWORD dword = 0;
    std::from_chars(envval, envval + std::strlen(envval), dword);
    return dword;
}

void dump_noita(DWORD process_id, HANDLE process)
{
    std::time_t t = std::time(nullptr);
    char name[300] = "crashes\\minidump_";
    char* left = name + std::strlen(name);
    left += std::strftime(left, std::end(name) - left, "%FT%H-%M-%S", std::gmtime(&t));
    std::strcpy(left, ".dmp");

    auto file = CreateFileA(
        name,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_WRITE | FILE_SHARE_READ,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    auto ptrstr = std::getenv("NoitaDumpPTR");
    std::uintptr_t dump_info_ptr = 0;
    std::from_chars(ptrstr, ptrstr + std::strlen(ptrstr), dump_info_ptr);

    dump_communication recv;
    ReadProcessMemory(
        process,
        reinterpret_cast<void*>(dump_info_ptr),
        &recv, sizeof(recv),
        nullptr
    );

    MINIDUMP_EXCEPTION_INFORMATION exception_info{
        recv.thread_id,
        recv.exception_pointers,
        true
    };

    MiniDumpWriteDump(
        process, process_id,
        file,
        MiniDumpWithDataSegs,
        &exception_info,
        nullptr,
        nullptr
    );

    CloseHandle(file);

    auto dump_finished_event = read_env_handle("NoitaDumpFinishedEvent");
    SetEvent(dump_finished_event);
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR lpCmdLine, INT nCmdShow)
{
    DWORD process_id = read_env_dword("NoitaPID");
    auto process = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | SYNCHRONIZE,
        false, process_id);

    auto crash_event = read_env_handle("NoitaCrashEvent");

    const HANDLE handles[2]{
        process,
        crash_event
    };

    auto wait_result = WaitForMultipleObjects(
        std::size(handles),
        handles,
        false,
        INFINITE
    );

    std::cout << wait_result << ": " << GetLastError() << "\n";

    auto triggered_handle = handles[wait_result - WAIT_OBJECT_0];

    if (triggered_handle == crash_event)
        dump_noita(process_id, process);

    return 0;
}
