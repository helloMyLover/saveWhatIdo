
//CpuUsage.h
#ifndef _CPU_USAGE_H_
#define _CPU_USAGE_H_

#include <windows.h>

class CpuUsage
{
public:
    CpuUsage(DWORD dwProcessID);
    ULONGLONG GetUsageEx();
    ULONGLONG GetSystemNonIdleTimes();
    ULONGLONG GetProcessNonIdleTimes();
private:
    ULONGLONG SubtractTimes(const FILETIME& ftA, const FILETIME& ftB);
    ULONGLONG AddTimes(const FILETIME& ftA, const FILETIME& ftB);
    bool EnoughTimePassed();
    inline bool IsFirstRun() const { return (m_dwLastRun == 0); }

    //system total times
    FILETIME m_ftPrevSysKernel;
    FILETIME m_ftPrevSysUser;

    //process times
    FILETIME m_ftPrevProcKernel;
    FILETIME m_ftPrevProcUser;

    ULONGLONG m_ullPrevSysNonIdleTime;//这个变量和后面的便利记录上次获取的非idle的系统cpu时间和进程cpu时间.
    ULONGLONG m_ullPrevProcNonIdleTime;//这个类只绑定一个进程, 在构造函数里面初始化进来..

    ULONGLONG m_nCpuUsage;
    ULONGLONG m_dwLastRun;
    DWORD m_dwProcessID;
    HANDLE m_hProcess;
    volatile LONG m_lRunCount;
};

#endif


