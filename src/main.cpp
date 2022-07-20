#include <cstring>
#include <ctime>
#include <iterator>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>

LONG WINAPI noita_exception_handler(EXCEPTION_POINTERS* exception_pointers)
{
    auto process = GetCurrentProcess();
    auto process_id = GetCurrentProcessId();

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

    MINIDUMP_EXCEPTION_INFORMATION exception_info{
        GetCurrentThreadId(),
        exception_pointers,
        true,
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

    return EXCEPTION_EXECUTE_HANDLER;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    switch(fdwReason) {
        case DLL_PROCESS_ATTACH:
            SetUnhandledExceptionFilter(noita_exception_handler);
            break;

        default:
            break;
    }

    return TRUE;
}
