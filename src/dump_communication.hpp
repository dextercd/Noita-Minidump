#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct dump_communication {
    DWORD thread_id;
    EXCEPTION_POINTERS* exception_pointers;
};
