// savewhatIDoing.cpp : �������̨Ӧ�ó������ڵ㡣
/*
2016.1.20  v1.0
���ܣ����浱ǰ���湤�����ݣ���ȡϵͳ���Ѵ򿪵��������ȡ������д򿪵���ҳ���Լ�����д򿪵��ĵ����´ο���ǰ������Щ���ݡ�

ջ�ṹ�����浱ǰ����λ�ã�
������ֶ�ģʽ����һ���Ի����з�����Ҫ�򿪵�������������ҳ���Լ��ĵ���

��ô��ȡ����Щ���Adminsterʹ�ã�
errror createprocessʧ��;������2����ĳ����ʱ��Ҫ�����������򡣶��豻�������򲻴�

�б�cpu�����ʸ��Լ��û�Ȩ�޵������������ȡ�����֣���� �Լ�����·��
�����������̱���ȡ��Ӧ�Ľ��̵�cpu��������ʣ��洢��һ���ı�data�У��б�ţ����̾��������·����ʹ��Ƶ�ʵĹ��ƣ����ڼӣ������ȴ򿪲�����
	
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
	printf("Enter N ���ػ�,Enter Y����������Сд�ް���\n");
	scanf("%c",type);

	if(type == 'Y' || type == 'y')
	{
		//�ж�Ϊ�����󣬼���ʼ�½����� 
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
		printf("��windows�ϴ򿪿ṷ\n");
		//system("C:\Program Files (x86)\kuwo\kuwomusic\8.0.3.1_UG6\bin\KwMusic.exe");
		LPTSTR szCmdline = _tcsdup(TEXT("I:\\QQ\\qqmusic\\QQMusic1224.17.46.6\\QQMusic.exe -L -S"));
		_bstr_t bstr;
		bstr=szCmdline;
		string str=bstr;
		fs<<str<<flush;  //flushˢ�»�����������������д������豸���ļ�  
		cout<<str<<endl;  //cout<<fs.rdbuf(); ��ӡfs�е�����
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
		//�ж�Ϊ׼���ػ�����ʼ��¼ϵͳ��Ӧ�еĽ��̣�or ��һ��ʼ�ͼ�¼ϵͳ���еĽ��̣�
		PROCESSENTRY32 pe32;
		pe32.dwSize=sizeof(pe32);
		HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS ,0);
		if(hProcessSnap == INVALID_HANDLE_VALUE)
		{
			printf("C reateToolhelp32Snapshot����ʧ��\n");
			return -1;
		}

		//��ʼ��
		BOOL bMore=::Process32First(hProcessSnap,&pe32);
		const char* name="QQMusic.exe";
		const WCHAR* wc = (pe32.szExeFile) ;
		_bstr_t b(wc);  //wcharת��Ϊchar
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
		//�˳�����Խ���cpuʹ��Ч��
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
				//printf("CPUʹ����: %d%%\n",cpu);
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