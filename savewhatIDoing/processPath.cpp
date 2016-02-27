//Taking a Snapshot and Viewing Processes
//The following simple console application obtains a list of running processes. First, the GetProcessList function takes a snapshot of currently executing processes in the system using CreateToolhelp32Snapshot, and then it walks through the list recorded in the snapshot using Process32First and Process32Next. For each process in turn, GetProcessList calls the ListProcessModules function which is described in Traversing the Module List, and the ListProcessThreads function which is described in Traversing the Thread List. 
//
//A simple error-reporting function, printError, displays the reason for any failures, which usually result from security restrictions.

#include "stdafx.h"
#include <stdio.h>
#include "processPath.h"
#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <string>

//  Forward declarations:
//BOOL GetProcessList();
//BOOL ListProcessModules( DWORD dwPID );
//BOOL ListProcessThreads( DWORD dwOwnerPID );
//void printError( TCHAR* msg );

CMyProcess::CMyProcess()
{
	int i;
}


BOOL CMyProcess::GetProcessList()
{
  HANDLE hProcessSnap;
  HANDLE hProcess;
  PROCESSENTRY32 pe32;
  DWORD dwPriorityClass;

  // Take a snapshot of all processes in the system.
  hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
  if( hProcessSnap == INVALID_HANDLE_VALUE )
  {
    printError( _T("CreateToolhelp32Snapshot (of processes)") );
    return( FALSE );
  }

  // Set the size of the structure before using it.
  pe32.dwSize = sizeof( PROCESSENTRY32 );

  // Retrieve information about the first process,
  // and exit if unsuccessful
  if( !Process32First( hProcessSnap, &pe32 ) )
  {
    printError( _T("Process32First") ); // Show cause of failure
    CloseHandle( hProcessSnap );    // Must clean up the
                                    //   snapshot object!
    return( FALSE );
  }

  // Now walk the snapshot of processes, and
  // display information about each process in turn
  do
  {
    printf( "\n\n"
      "=====================================================" );
    printf( "\nPROCESS NAME:  %s", pe32.szExeFile );
    printf( "\n"
      "-----------------------------------------------------" );

    // Retrieve the priority class.
    dwPriorityClass = 0;
    hProcess = OpenProcess(
      PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID );
    if( hProcess == NULL )
      printError( _T("OpenProcess") );
    else
    {
		dwPriorityClass = GetPriorityClass( hProcess );
		if( !dwPriorityClass )
			printError( _T("GetPriorityClass") );
        CloseHandle( hProcess );
    }

    printf( "\n  process ID        = 0x%08X", pe32.th32ProcessID );
    //printf( "\n  thread count      = %d",   pe32.cntThreads );
    //printf( "\n  parent process ID = 0x%08X",
    //  pe32.th32ParentProcessID );
    //printf( "\n  Priority Base     = %d", pe32.pcPriClassBase );
    //if( dwPriorityClass )
    //  printf( "\n  Priority Class    = %d", dwPriorityClass );

    // List the modules and threads associated with this process
    ListProcessModules( pe32.th32ProcessID );
    ListProcessThreads( pe32.th32ProcessID );

  } while( Process32Next( hProcessSnap, &pe32 ) );

  CloseHandle( hProcessSnap );
  return( TRUE );
}


BOOL CMyProcess::ListProcessModules( DWORD dwPID )
{
  HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
  MODULEENTRY32 me32;

  // Take a snapshot of all modules in the specified process.
  hModuleSnap = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, dwPID );
  if( hModuleSnap == INVALID_HANDLE_VALUE )
  {
    printError( _T("CreateToolhelp32Snapshot (of modules)") );
    return( FALSE );
  }

  // Set the size of the structure before using it.
  me32.dwSize = sizeof( MODULEENTRY32 );

  // Retrieve information about the first module,
  // and exit if unsuccessful
  if( !Module32First( hModuleSnap, &me32 ) )
  {
	  printError( _T("Module32First") ); // Show cause of failure
	  CloseHandle( hModuleSnap );    // Must clean up the
                                   //   snapshot object!
	  return( FALSE );
  }

  // Now walk the module list of the process,
  // and display information about each module
  do
  {
    printf( "\n\n     MODULE NAME:     %s",
      me32.szModule );
    printf( "\n     executable     = %s",
      me32.szExePath );
    printf( "\n     process ID     = 0x%08X",
      me32.th32ProcessID );
    printf( "\n     ref count (g)  =     0x%04X",
      me32.GlblcntUsage );
    printf( "\n     ref count (p)  =     0x%04X",
      me32.ProccntUsage );
    printf( "\n     base address   = 0x%08X",
      (DWORD) me32.modBaseAddr );
    printf( "\n     base size      = %d",
      me32.modBaseSize );

  } while( Module32Next( hModuleSnap, &me32 ) );

  CloseHandle( hModuleSnap );
  return( TRUE );
}

BOOL CMyProcess::ListProcessThreads( DWORD dwOwnerPID ) 
{ 
  HANDLE hThreadSnap = INVALID_HANDLE_VALUE; 
  THREADENTRY32 te32; 
 
  // Take a snapshot of all running threads  
  hThreadSnap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 ); 
  if( hThreadSnap == INVALID_HANDLE_VALUE ) 
    return( FALSE ); 
 
  // Fill in the size of the structure before using it. 
  te32.dwSize = sizeof(THREADENTRY32 ); 
 
  // Retrieve information about the first thread,
  // and exit if unsuccessful
  if( !Thread32First( hThreadSnap, &te32 ) ) 
  {
    printError( _T("Thread32First") ); // Show cause of failure
    CloseHandle( hThreadSnap );    // Must clean up the
                                   //   snapshot object!
    return( FALSE );
  }
  // Now walk the thread list of the system,
  // and display information about each thread
  // associated with the specified process
  do 
  { 
    if( te32.th32OwnerProcessID == dwOwnerPID )
    {
      printf( "\n\n     THREAD ID      = 0x%08X",
        te32.th32ThreadID ); 
      printf( "\n     base priority  = %d", te32.tpBasePri ); 
      printf( "\n     delta priority = %d", te32.tpDeltaPri ); 
    }
  } while( Thread32Next(hThreadSnap, &te32 ) ); 

  CloseHandle( hThreadSnap );
  return( TRUE );
}

void CMyProcess::printError( TCHAR* msg )
{
  DWORD eNum;
  TCHAR sysMsg[256];
  TCHAR* p;

  eNum = GetLastError( );
  FormatMessage(
         FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
         NULL, eNum,
         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default lang.
         sysMsg, 256, NULL );

  // Trim the end of the line and terminate it with a null
  p = sysMsg;
  while( ( *p > 31 ) || ( *p == 9 ) )
    ++p;
  do { *p-- = 0; } while( ( p >= sysMsg ) &&
                          ( ( *p == '.' ) || ( *p < 33 ) ) );

  // Display the message
  printf( "\n  WARNING: %s failed with error %d (%s)",
    msg, eNum, sysMsg );
}

