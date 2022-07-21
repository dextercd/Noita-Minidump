#include <cstring>
#include <ctime>
#include <iterator>
#include <charconv>
#include <iostream>
#include <stdexcept>
#include <string>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>

#include "dump_communication.hpp"

using namespace std::literals;

const char* get_env(const char* envvar)
{
    auto envval = std::getenv(envvar);
    if (!envval)
        throw std::runtime_error{"Environment variable "s + envvar + " does not exist."};
}

template<class T>
T int_from_env(const char* envvar)
{
    auto envval = get_env(envvar);

    T value{};
    auto result = std::from_chars(envval, envval + std::strlen(envval), value);
    if (result.ec != std::errc{})
        throw std::system_error{std::make_error_code(result.ec)};

    return value;
}

HANDLE read_env_handle(const char* envvar)
{
    auto handle_int = int_from_env<std::uintptr_t>(envvar);
    return reinterpret_cast<HANDLE>(handle_int);
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

    auto dump_info_ptr = int_from_env<std::uintptr_t>("NoitaDumpPTR");

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

void run()
{
    auto process_id = int_from_env<DWORD>("NoitaPID");
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
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR lpCmdLine, INT nCmdShow)
{
    try {
        run();
    } catch(const std::exception& exc) {
        std::cerr << "Fatal error: " << exc.what() << ".\n";
        return 1;
    } catch(...) {
        std::cerr << "Unknown fatal error.\n";
        return 2;
    }

    return 0;
}
