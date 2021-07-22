#if !defined(WIN32_DEFINES_H)

// @NOTE: stuff from windows.h goes here so we don't have to include the header file

#define MEM_COMMIT  0x00001000
#define MEM_RESERVE 0x00002000
#define PAGE_READWRITE 0x04

#define MEM_RELEASE 0x00008000

extern "C"
{
    typedef int BOOL;
    typedef unsigned long DWORD;
    typedef long LONG;
    typedef long long LONGLONG;
    typedef void *LPVOID;
    typedef size_t SIZE_T;
    typedef unsigned int UINT;

    typedef union _LARGE_INTEGER {
      struct {
        DWORD LowPart;
        LONG  HighPart;
      } DUMMYSTRUCTNAME;
      struct {
        DWORD LowPart;
        LONG  HighPart;
      } u;
      LONGLONG QuadPart;
    } LARGE_INTEGER;

    BOOL QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount);
    BOOL QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency);
    LPVOID VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
    BOOL VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType);
    void ExitProcess(UINT uExitCode);

    unsigned char _BitScanForward(unsigned long *Index, unsigned long Mask);
};

#define WIN32_DEFINES_H
#endif
