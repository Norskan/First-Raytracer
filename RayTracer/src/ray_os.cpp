/*
Windows
*/
#include <windows.h>

#define DebuggerBreak() DebugBreak()

static U64 GetCPUTicks() {
    LARGE_INTEGER result;
    BOOL success = QueryPerformanceCounter(&result);
    
    if(!success) {
        DWORD errorCode = GetLastError();
        printf("WIN_API error occured: %lu\n", errorCode);
    }
    
    return (U64)result.QuadPart;
}

static U64 GetCPUFrequency() {
    LARGE_INTEGER result;
    BOOL success = QueryPerformanceFrequency(&result);
    
    if(!success) {
        DWORD errorCode = GetLastError();
        printf("WIN_API error occured: %lu\n", errorCode);
    }
    
    return (U64)result.QuadPart;
}

static U64 GetTimeStamp() {
    U64 ticks = GetCPUTicks();
    U64 frequency = GetCPUFrequency();
    
    //to microseconds
    return ticks  * 1000000 / frequency;
}