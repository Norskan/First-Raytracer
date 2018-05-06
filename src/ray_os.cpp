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

static U32 GetCPUCores() {
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    U32 processorCount = sysinfo.dwNumberOfProcessors;
    
    return processorCount;
}

typedef DWORD (*ThreadFunction)(void* data);
void CreateMultipleThreadsAndWait(ThreadFunction threadFunction, RayTraceSectionRowData* threadDataList, int dataCount) {
    //NOTE(ans): not freed atm
    HANDLE* nativeHandles = (HANDLE*)malloc(sizeof(HANDLE) * dataCount);
    
#if DEBUG_DISABLE_PARALLEL_THREADING
    for(int i = 0; i < dataCount; ++i) {
        RayTraceSectionRowData* threadData = threadDataList + i;
        
        DWORD threadId;
        HANDLE threadHandle = CreateThread(NULL,
                                           0,
                                           (LPTHREAD_START_ROUTINE)threadFunction,
                                           threadData,
                                           0,
                                           &threadId);
        
        WaitForSingleObject(threadHandle, INFINITE);
    }
#else
    
    for(int i = 0; i < dataCount; ++i) {
        RayTraceSectionRowData* threadData = threadDataList + i;
        
        DWORD threadId;
        *(nativeHandles + i) = CreateThread(NULL,
                                            0,
                                            (LPTHREAD_START_ROUTINE)threadFunction,
                                            threadData,
                                            0,
                                            &threadId);
    }
    
    WaitForMultipleObjects(dataCount,nativeHandles, true, INFINITE);
#endif
}
