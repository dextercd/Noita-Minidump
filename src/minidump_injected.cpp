#include <string>
#include <iostream>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>

#include "dump_communication.hpp"

dump_communication dump_info{};
DWORD dump_process_id;
HANDLE crash_event;
HANDLE dump_finished_event;

LONG WINAPI noita_exception_handler(EXCEPTION_POINTERS* exception_pointers)
{
    dump_info.thread_id = GetCurrentThreadId();
    dump_info.exception_pointers = exception_pointers;

    SetEvent(crash_event);
    WaitForSingleObject(dump_finished_event, 3000);

    return EXCEPTION_EXECUTE_HANDLER;
}

void spawn_dumper_helper()
{
    SECURITY_ATTRIBUTES event_sec_attr{
        .nLength = sizeof(event_sec_attr),
        .lpSecurityDescriptor = nullptr,
        .bInheritHandle = true,
    };
    crash_event = CreateEvent(&event_sec_attr, true, false, nullptr);
    dump_finished_event = CreateEvent(&event_sec_attr, true, false, nullptr);

    SetEnvironmentVariable("NoitaPID", std::to_string(GetCurrentProcessId()).c_str());
    SetEnvironmentVariable("NoitaDumpPTR",
        std::to_string(reinterpret_cast<std::uintptr_t>(&dump_info)).c_str());
    SetEnvironmentVariable("NoitaCrashEvent",
        std::to_string(reinterpret_cast<std::uintptr_t>(crash_event)).c_str());
    SetEnvironmentVariable("NoitaDumpFinishedEvent",
        std::to_string(reinterpret_cast<std::uintptr_t>(dump_finished_event)).c_str());

    STARTUPINFOA startup_info{
        .cb = sizeof(startup_info),
        .lpReserved = nullptr,
        .lpDesktop = nullptr,
        .lpTitle = nullptr,
        .dwX = 0, .dwY = 0,
        .dwXSize = 0, .dwYSize = 0,
        .dwXCountChars = 0, .dwYCountChars = 0,
        .dwFillAttribute = 0,
        .dwFlags = 0,
        .wShowWindow = 0,
        .cbReserved2 = 0,
        .lpReserved2 = nullptr,
        .hStdInput = nullptr,
        .hStdOutput = nullptr,
        .hStdError = nullptr,
    };
    PROCESS_INFORMATION proc_info;

    auto create_proc_res = CreateProcessA(
        "mods\\minidump\\noita_dumper.exe",
        nullptr,
        nullptr,
        nullptr,
        true,
        0,
        nullptr,
        nullptr,
        &startup_info,
        &proc_info
    );

    if (!create_proc_res)
        std::cerr << "Couldn't create process: " << create_proc_res << "\n";
    else
        std::cerr << "Dump process created!: " << create_proc_res << "\n";

    dump_process_id = proc_info.dwProcessId;

    CloseHandle(proc_info.hProcess);
    CloseHandle(proc_info.hThread);
}

extern "C" __declspec(dllexport)
void init_minidump()
{
    static bool initialised = false;
    if (initialised)
        return;

    initialised = true;

    spawn_dumper_helper();
    SetUnhandledExceptionFilter(noita_exception_handler);
}
