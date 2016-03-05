// savewhatIDoing.cpp : 定义控制台应用程序的入口点。
/*
2016.1.20  v1.0
功能：保存当前桌面工作内容，读取系统中已打开的软件，读取浏览器中打开的网页，以及软件中打开的文档，下次开机前，打开这些内容。

栈结构，保存当前鼠标的位置，
可添加手动模式，在一个对话框中放入想要打开的软件和浏览器网页，以及文档。

怎么读取有哪些软件Adminster使用？
errror createprocess失败;错误码2运行某程序时需要调用其他程序。而需被调动程序不存

判别cpu利用率高以及用户权限的软件出来，获取其名字，句柄 以及绝对路径
遍历整个进程表，获取相应的进程的cpu最高利用率，存储在一个文本data中，有编号，进程句柄，绝对路径，使用频率的估计（后期加），优先打开参数级
	
*/

#include "stdafx.h"
#include <conio.h>
#include <stdlib.h>
#include <Windows.h>
#include <TlHelp32.h>
#include "CpuUsage.h"
#include "CpuUsage2.h"
//#include "processPath.h"
#include <comdef.h>
#include <dos.h>
#include <vector>
#include <fstream>
#include <string>
#include <AtlBase.h>
#include <AtlConv.h>
using namespace std;


int _tmain(int argc, _TCHAR* argv[])
{
	char type;
	printf("Enter N 即关机,Enter Y即开机，大小写无碍。\n");
	scanf("%c",type);

	if(type == 'Y' || type == 'y')
	{
		//判断为开机后，即开始新建进程 
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		fstream fs("saveSoft.txt",ios::in|ios::out);
		if(fs.bad())
		{
			printf("open file failure");
		}

		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		ZeroMemory( &pi, sizeof(pi) );
		printf("It's a beginning\n");
		printf("在windows上打开酷狗\n");
		//system("C:\Program Files (x86)\kuwo\kuwomusic\8.0.3.1_UG6\bin\KwMusic.exe");
		LPTSTR szCmdline = _tcsdup(TEXT("I:\\QQ\\qqmusic\\QQMusic1224.17.46.6\\QQMusic.exe -L -S"));
		_bstr_t bstr;
		bstr=szCmdline;
		string str=bstr;
		fs<<str<<flush;  //flush刷新缓冲区，即数据真正写到输出设备或文件  
		cout<<str<<endl;  //cout<<fs.rdbuf(); 打印fs中的内容
		//CreateProcess(NULL,szCmdline,);
		int temp=0;
		if( !(CreateProcess( NULL,   // No module name (use command line)
			szCmdline,        // Command line
			NULL,           // Process handle not inheritable
			NULL,           // Thread handle not inheritable
			FALSE,          // Set handle inheritance to FALSE
			0,              // No creation flags
			NULL,           // Use parent's environment block
			NULL,           // Use parent's starting directory 
			&si,            // Pointer to STARTUPINFO structure
			&pi ))           // Pointer to PROCESS_INFORMATION structure
		) 
		{
			printf( "CreateProcess failed (%d).\n", GetLastError() );
			getch();
			return 1;
		}

		    // Close process and thread handles. 
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
	}
	else if(type == 'N' || type == 'n')
	{
			// Wait until child process exits.
	 //   WaitForSingleObject( pi.hProcess, INFINITE );
		//判断为准备关机，则开始记录系统中应有的进程，or 从一开始就记录系统中有的进程，
		PROCESSENTRY32 pe32;
		pe32.dwSize=sizeof(pe32);
		HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS ,0);
		if(hProcessSnap == INVALID_HANDLE_VALUE)
		{
			printf("C reateToolhelp32Snapshot调用失败\n");
			return -1;
		}

		//初始化
		BOOL bMore=::Process32First(hProcessSnap,&pe32);
		const char* name="QQMusic.exe";
		const WCHAR* wc = (pe32.szExeFile) ;
		_bstr_t b(wc);  //wchar转换为char
		const char* c = b;
		int Pid=0;
		while(bMore)
		{
			wc = (pe32.szExeFile) ;
			_bstr_t b(wc);    
			c = b;
			while(!strcmp(b,name))
			{
				printf("%u , %u , %u , %u , %u , %ls , %lu , %u , %u , %u\n\n"
								,pe32.cntThreads
								,pe32.cntUsage
								,pe32.dwFlags
								,pe32.dwSize
								,pe32.pcPriClassBase//4
								,pe32.szExeFile  //6
								,pe32.th32DefaultHeapID
								,pe32.th32ModuleID
								,pe32.th32ParentProcessID
								,pe32.th32ProcessID);
				Pid=pe32.th32ProcessID;
				break;
			}
			bMore=::Process32Next(hProcessSnap,&pe32);
		
		}
		//此程序测试进程cpu使用效率
		unsigned long long cpu =0;
		CpuUsage cu(Pid);
		unsigned long long appTime=cu.GetUsageEx();
		printf("%d 2\n ",pe32.th32ProcessID);
		SYSTEMTIME st;
		while(true)
		{		
			GetLocalTime(&st);
			if(appTime != 0 && cpu !=0)
			{	
				cpu = get_cpu_usage(Pid);
				//printf("CPU使用率: %d%%\n",cpu);
				printf("Process(pid:%d) uses %I64d%% ,%I64d%% cpu at %02d:%02d.%02d\n", Pid, appTime, cpu,st.wHour, st.wMinute, st.wSecond);
			 //  ::Sleep(5000);
			}
			appTime=cu.GetUsageEx();
		}
		::CloseHandle(hProcessSnap);
	}
	getch();
	return 0;
}



////main.cpp
//
//#include "cpuusage.h"
//#include <fstream>
//#include <cstdlib>
//#include <cstdio>
//using namespace std;
//
//const int second = 1000;
//
//int main(int argc, char* argv[])
//{
//    if(argc != 2)
//    {
//        printf("Use the toolkit like: <toolkit name> <pid>\n");
//        return 0;
//    }
//    DWORD dwProcId = atoi(argv[1]);
//    CpuUsage cu(dwProcId);
//    SYSTEMTIME st;
//    while(true)
//    {
//        GetLocalTime(&st);
//        printf("Process(pid:%d) uses %I64d%% cpu at %02d:%02d.%02d\n", dwProcId, cu.GetUsageEx(), st.wHour, st.wMinute, st.wSecond);
//        ::Sleep(second);
//    }
//}