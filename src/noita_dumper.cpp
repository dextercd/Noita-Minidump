#include <charconv>
#include <cstring>
#include <ctime>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <system_error>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>

#include "dump_communication.hpp"

using namespace std::literals;

[[noreturn]]
void throw_windows_last_error(const char* what)
{
    auto last_error = GetLastError();
    std::error_code ec{(int)last_error, std::system_category()};
    throw std::system_error{ec, what};
}

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

std::string datetime_string(std::time_t t)
{
    // year month day hour minutes seconds
    // 4 +  2 +   2 + 2 +  2 +     2       = 14
    // With separators and '\0'            = 20

    // Year could technically have more digits, so use a larger buffer for future-proofing
    char buffer[99];
    std::strftime(buffer, sizeof(buffer), "%FT%H-%M-%S", std::gmtime(&t));

    return buffer;
}

void dump_process(
        DWORD process_id, HANDLE process,
        DWORD thread_id,
        EXCEPTION_POINTERS* exception_pointers,
        const char* output_path)
{
    auto dmp_file = CreateFileA(
        output_path,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_WRITE | FILE_SHARE_READ,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (dmp_file == INVALID_HANDLE_VALUE)
        throw_windows_last_error("Couldn't open .dmp file");

    MINIDUMP_EXCEPTION_INFORMATION exception_info{thread_id, exception_pointers, true};

    if (!MiniDumpWriteDump(
            process, process_id,
            dmp_file,
            MiniDumpWithDataSegs,
            &exception_info,
            nullptr,
            nullptr)
    ) {
        throw_windows_last_error("Couldn't write mini dump");
    };

    CloseHandle(dmp_file);
}

void run()
{
    auto dump_info_ptr = int_from_env<std::uintptr_t>("NoitaDumpPTR");
    auto crash_event = read_env_handle("NoitaCrashEvent");
    auto dump_finished_event = read_env_handle("NoitaDumpFinishedEvent");
    auto process_id = int_from_env<DWORD>("NoitaPID");

    auto process = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | SYNCHRONIZE,
        false, process_id);

    if (process == nullptr)
        throw_windows_last_error("Couldn't open Noita process");

    const HANDLE handles[2]{process, crash_event};
    auto wait_result = WaitForMultipleObjects(std::size(handles), handles, false, INFINITE);

    if (wait_result == WAIT_FAILED)
        throw_windows_last_error("Couldn't wait on dump request or program exit");

    auto triggered_handle = handles[wait_result - WAIT_OBJECT_0];

    if (triggered_handle == process)
        return;

    dump_communication recv;
    if (!ReadProcessMemory(
            process,
            reinterpret_cast<void*>(dump_info_ptr),
            &recv, sizeof(recv),
            nullptr)
    ) {
        throw_windows_last_error("Couldn't read dump info from Noita process");
    }

    auto t = std::time(nullptr);
    std::string dump_name = "crashes\\minidump_" + datetime_string(t) + ".dmp";

    dump_process(process_id, process, recv.thread_id, recv.exception_pointers, dump_name.c_str());
    SetEvent(dump_finished_event);
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
