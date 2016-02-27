#ifndef PROCESSPATH_20160205
#define PROCESSPATH_20160205
#include "stdafx.h"
#include <windows.h>
#include <tlhelp32.h>
#include <conio.h>
#include <stdlib.h>
#include <string>
using namespace std;
class CMyProcess
{
public:
	CMyProcess();
//	~CMyProcess();
	BOOL GetProcessList();
	BOOL ListProcessModules( DWORD dwPID );
	BOOL ListProcessThreads( DWORD dwOwnerPID );
	void printError( TCHAR* msg );
private:
	HANDLE processHANDLE;
	string processPath;
	string processName;

};

//BOOL GetProcessList( );

#endif