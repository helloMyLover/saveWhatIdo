
//CpuUsage.cpp
#include "StdAfx.h"
#include <windows.h>
#include "CPUusage.h"
#include <strsafe.h>
//#define USE_DEPRECATED_FUNCS
void ErrorMsg(LPTSTR lpszFunction);
BOOL SetPrivilege(HANDLE hProcess, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);

#ifdef USE_DEPRECATED_FUNCS
#define SystemBasicInformation                    0
#define SystemPerformanceInformation            2
#define SystemTimeInformation                    3
#define SystemProcessorPerformanceInformation    8
#define ProcessTimes                            4

#define Li2Double(x) ((double)((x).HighPart) * 4.294967296E9 + (double)((x).LowPart))

typedef struct
{
    DWORD dwUnknown1;
    ULONG uKeMaximumIncrement;
    ULONG uPageSize;
    ULONG uMmNumberOfPhysicalPages;
    ULONG uMmLowestPhysicalPage;
    ULONG uMmHighestPhysicalPage;
    ULONG uAllocationGranularity;
    PVOID pLowestUserAddress;
    PVOID pMmHighestUserAddress;
    ULONG uKeActiveProcessors;
    BYTE bKeNumberProcessors;
    BYTE bUnknown2;
    WORD wUnknown3;
} SYSTEM_BASIC_INFORMATION;

typedef struct
{
    LARGE_INTEGER liIdleTime;
    DWORD dwSpare[312];
} SYSTEM_PERFORMANCE_INFORMATION;

typedef struct
{
    LARGE_INTEGER liKeBootTime;
    LARGE_INTEGER liKeSystemTime;
    LARGE_INTEGER liExpTimeZoneBias;
    ULONG uCurrentTimeZoneId;
    DWORD dwReserved;
} SYSTEMTEXTIME_INFORMATION;

typedef struct
        _SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION
{
    LARGE_INTEGER IdleTime;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER Reserved1[2];
    ULONG Reserved2;
} SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION;

typedef struct _KERNEL_USERTEXTIMES
{
    LARGE_INTEGER CreateTime;
    LARGE_INTEGER ExitTime;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
} KERNEL_USERTEXTIMES, *PKERNEL_USERTEXTIMES;

typedef LONG (WINAPI *PROCNTQSI)(UINT, PVOID, ULONG, PULONG);
PROCNTQSI NtQuerySystemInformation;

typedef LONG (WINAPI *PROCNTQIP)(HANDLE, UINT, PVOID, ULONG, PULONG);
PROCNTQIP NtQueryInformationProcess;

ULONGLONG CpuUsage::GetSystemNonIdleTimes()
{
    SYSTEM_PERFORMANCE_INFORMATION SysPerfInfo;
    SYSTEMTEXTIME_INFORMATION SysTimeInfo;
    SYSTEM_BASIC_INFORMATION SysBaseInfo;
    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION SysProcPerfInfo[32];
    LONG status;
    NtQuerySystemInformation = (PROCNTQSI)GetProcAddress(GetModuleHandle(TEXT("ntdll")), "NtQuerySystemInformation");
    if (!NtQuerySystemInformation)
        return 0;
    status = NtQuerySystemInformation(SystemBasicInformation, &SysBaseInfo, sizeof(SysBaseInfo), NULL);
    if (status != NO_ERROR)
    {
        MessageBox(TEXT("FailSystemInfo"));
        return 0;
    }
    status = NtQuerySystemInformation(SystemProcessorPerformanceInformation, SysProcPerfInfo, sizeof(SysProcPerfInfo), NULL);
    if(status != NO_ERROR) return 0;
    int nProcessors = SysBaseInfo.bKeNumberProcessors; //机器内部CPU的个数
    ULONGLONG ullSysTotal = 0;
    for(int i = 0; i < nProcessors; i++)
    {
        ullSysTotal += SysProcPerfInfo[i].KernelTime.QuadPart + SysProcPerfInfo[i].UserTime.QuadPart;
    }
    return ullSysTotal;
}

ULONGLONG CpuUsage::GetProcessNonIdleTimes()
{
    KERNEL_USERTEXTIMES KernelUserTimes;
    ::ZeroMemory(&KernelUserTimes, sizeof(KernelUserTimes));
    NtQueryInformationProcess = (PROCNTQIP)GetProcAddress(GetModuleHandle(TEXT("ntdll")), "NtQueryInformationProcess");
    LONG status = NtQueryInformationProcess(m_hProcess, ProcessTimes, &KernelUserTimes, sizeof(KernelUserTimes), NULL);
    if(status == 0)
    {
        ErrorExit(TEXT("GetProcessNonIdleTimes"));
        return 0;
    }
    return KernelUserTimes.KernelTime.QuadPart + KernelUserTimes.UserTime.QuadPart;

}

#endif

CpuUsage::CpuUsage(DWORD dwProcessID)
    : m_nCpuUsage(0),
      m_dwLastRun(0),
      m_lRunCount(0),
      m_dwProcessID(dwProcessID),
      m_ullPrevProcNonIdleTime(0),
      m_ullPrevSysNonIdleTime(0)
{
    HANDLE hProcess = GetCurrentProcess();
    SetPrivilege(hProcess, SE_DEBUG_NAME, TRUE);

    m_hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION , TRUE, m_dwProcessID);
    if(m_hProcess == 0)
    {
        ErrorMsg(TEXT("OpenProcess"));
    }
    ZeroMemory(&m_ftPrevSysKernel, sizeof(FILETIME));
    ZeroMemory(&m_ftPrevSysUser, sizeof(FILETIME));

    ZeroMemory(&m_ftPrevProcKernel, sizeof(FILETIME));
    ZeroMemory(&m_ftPrevProcUser, sizeof(FILETIME));
}


ULONGLONG CpuUsage::SubtractTimes(const FILETIME &ftA, const FILETIME &ftB)
{
    LARGE_INTEGER a, b;
    a.LowPart = ftA.dwLowDateTime;
    a.HighPart = ftA.dwHighDateTime;

    b.LowPart = ftB.dwLowDateTime;
    b.HighPart = ftB.dwHighDateTime;

    return a.QuadPart - b.QuadPart;
}

ULONGLONG CpuUsage::AddTimes(const FILETIME &ftA, const FILETIME &ftB)
{
    LARGE_INTEGER a, b;
    a.LowPart = ftA.dwLowDateTime;
    a.HighPart = ftA.dwHighDateTime;

    b.LowPart = ftB.dwLowDateTime;
    b.HighPart = ftB.dwHighDateTime;

    return a.QuadPart + b.QuadPart;
}

bool CpuUsage::EnoughTimePassed()
{
    const int minElapsedMS = 250;//milliseconds

    ULONGLONG dwCurrentTickCount = GetTickCount();
    return (dwCurrentTickCount - m_dwLastRun) > minElapsedMS;
}
#ifndef USE_DEPRECATED_FUNCS

ULONGLONG CpuUsage::GetSystemNonIdleTimes()
{
    FILETIME ftSysIdle, ftSysKernel, ftSysUser;
    if(!GetSystemTimes(&ftSysIdle, &ftSysKernel, &ftSysUser))
    {
        ErrorMsg(TEXT("GetSystemTimes"));
        return 0;
    }
    return AddTimes(ftSysKernel, ftSysUser);
}


ULONGLONG CpuUsage::GetProcessNonIdleTimes()
{
    FILETIME ftProcCreation, ftProcExit, ftProcKernel, ftProcUser;
    if(!GetProcessTimes(m_hProcess, &ftProcCreation, &ftProcExit, &ftProcKernel, &ftProcUser) && false)
    {
        ErrorMsg(TEXT("GetProcessNonIdleTimes"));
        return 0;
    }
    return AddTimes(ftProcKernel, ftProcUser);
}
#endif

ULONGLONG CpuUsage::GetUsageEx()
{
    ULONGLONG nCpuCopy = m_nCpuUsage;
    if (::InterlockedIncrement(&m_lRunCount) == 1)
    {
        if (!EnoughTimePassed())
        {
            ::InterlockedDecrement(&m_lRunCount);
            return nCpuCopy;
        }
        ULONGLONG ullSysNonIdleTime = GetSystemNonIdleTimes();
        ULONGLONG ullProcNonIdleTime = GetProcessNonIdleTimes();
        if (!IsFirstRun())
        {
            ULONGLONG ullTotalSys = ullSysNonIdleTime - m_ullPrevSysNonIdleTime;
            if(ullTotalSys == 0)
            {
                ::InterlockedDecrement(&m_lRunCount);
                return nCpuCopy;
            }
            m_nCpuUsage = ULONGLONG((ullProcNonIdleTime - m_ullPrevProcNonIdleTime) * 100.0 / (ullTotalSys));
            m_ullPrevSysNonIdleTime = ullSysNonIdleTime;
            m_ullPrevProcNonIdleTime = ullProcNonIdleTime;
        }
        m_dwLastRun = (ULONGLONG)GetTickCount();
        nCpuCopy = m_nCpuUsage;
    }
    ::InterlockedDecrement(&m_lRunCount);
    return nCpuCopy;
}

BOOL SetPrivilege(HANDLE hProcess, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
    HANDLE hToken;
    if(!OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES, &hToken))
    {
        ErrorMsg(TEXT("OpenProcessToken"));
        return FALSE;
    }
    LUID luid;
    if(!LookupPrivilegeValue(NULL, lpszPrivilege, &luid))
    {
        ErrorMsg(TEXT("LookupPrivilegeValue"));
        return FALSE;
    }
    TOKEN_PRIVILEGES tkp;
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = luid;
    tkp.Privileges[0].Attributes = (bEnablePrivilege) ? SE_PRIVILEGE_ENABLED : FALSE;
    if(!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL))
    {
        ErrorMsg(TEXT("AdjustTokenPrivileges"));
        return FALSE;
    }
    return TRUE;
}

void ErrorMsg(LPTSTR lpszFunction)
{
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        LANG_USER_DEFAULT,
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
                                      (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
    StringCchPrintf((LPTSTR)lpDisplayBuf,
                    LocalSize(lpDisplayBuf) / sizeof(TCHAR),
                    TEXT("%s failed with error %d: %s"),
                    lpszFunction, dw, lpMsgBuf);
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw);
}