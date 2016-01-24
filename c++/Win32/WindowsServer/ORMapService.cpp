////////////////////////////////////////////////////////////////////// 
// Generic Windows Service
// 
// Originally posted as XYNTService on codeproject.com.
//
// Usage:
//
//    Run [ORMapService -i] to install the service.
//    View the service in Start->Settings->Control Panel->Admin Tools->Services.
//    It will be configured to start automatically, by default.
//    You can start it immediately after installing by right-clicking and selecting Start.
//    The server will use the settings in the ORMapService.ini file, see that for details.
//
// Changes:
//
// MDM	7/6/2005 5:22:29 PM
// Renamed              To
//
//    XYNTService.dsw      ORMapService.dsw
//    XYNTService.dsp      ORMapService.dsp
//    XYNTService.cpp      ORMapService.cpp
//    XYNTService.ini      ORMapService.ini
//
// Then searched and replaced XYNT with ORMap within the files.
// Then updated ORMapService.ini to start ORMap with -loadflowserver 
// and -autologin parameters.
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <windows.h>
#include <winbase.h>
#include <winsvc.h>
#include <process.h>


const int nBufferSize = 500;
char pServiceName[nBufferSize+1];
char pExeFile[nBufferSize+1];
char pInitFile[nBufferSize+1];
char pLogFile[nBufferSize+1];
int nProcCount = 0;
PROCESS_INFORMATION* pProcInfo = 0;

SERVICE_STATUS          serviceStatus; 
SERVICE_STATUS_HANDLE   hServiceStatusHandle; 

VOID WINAPI ORMapServiceMain( DWORD dwArgc, LPTSTR *lpszArgv );
VOID WINAPI ORMapServiceHandler( DWORD fdwControl );

CRITICAL_SECTION myCS;

void WriteLog(char* pFile, char* pMsg)
{
	// write error or other information into log file
	::EnterCriticalSection(&myCS);
	try
	{
		SYSTEMTIME oT;
		::GetLocalTime(&oT);
		FILE* pLog = fopen(pFile,"a");
		fprintf(pLog,"%02d/%02d/%04d, %02d:%02d:%02d\r\n    %s",oT.wMonth,oT.wDay,oT.wYear,oT.wHour,oT.wMinute,oT.wSecond,pMsg); 
		fclose(pLog);
	} catch(...) {}
	::LeaveCriticalSection(&myCS);
}

////////////////////////////////////////////////////////////////////// 
//
// Configuration Data and Tables
//

SERVICE_TABLE_ENTRY   DispatchTable[] = 
{ 
	{pServiceName, ORMapServiceMain}, 
	{NULL, NULL}
};


// helper functions

BOOL StartProcess(int nIndex) 
{ 
	// start a process with given index
	STARTUPINFO startUpInfo = { sizeof(STARTUPINFO),NULL,"",NULL,0,0,0,0,0,0,0,STARTF_USESHOWWINDOW,0,0,NULL,0,0,0};  
	char pItem[nBufferSize+1];
	sprintf(pItem,"Process%d\0",nIndex);
	char pCommandLine[nBufferSize+1];
	GetPrivateProfileString(pItem,"CommandLine","",pCommandLine,nBufferSize,pInitFile);
	char pUserInterface[nBufferSize+1];
	GetPrivateProfileString(pItem,"UserInterface","N",pUserInterface,nBufferSize,pInitFile);
	BOOL bUserInterface = (pUserInterface[0]=='y'||pUserInterface[0]=='Y'||pUserInterface[0]=='1')?TRUE:FALSE;
	char CurrentDesktopName[512];
	// set the correct desktop for the process to be started
	if(bUserInterface)
	{
		startUpInfo.wShowWindow = SW_SHOW;
		startUpInfo.lpDesktop = NULL;
	}
	else
	{
		HDESK hCurrentDesktop = GetThreadDesktop(GetCurrentThreadId());
		DWORD len;
		GetUserObjectInformation(hCurrentDesktop,UOI_NAME,CurrentDesktopName,MAX_PATH,&len);
		startUpInfo.wShowWindow = SW_HIDE;
		startUpInfo.lpDesktop = CurrentDesktopName;
	}
	// create the process
	char pWorkingDir[nBufferSize+1];
	GetPrivateProfileString(pItem,"WorkingDir","",pWorkingDir,nBufferSize,pInitFile);
	if(CreateProcess(NULL,pCommandLine,NULL,NULL,TRUE,NORMAL_PRIORITY_CLASS,NULL,strlen(pWorkingDir)==0?NULL:pWorkingDir,&startUpInfo,&pProcInfo[nIndex]))
	{
		char pPause[nBufferSize+1];
		GetPrivateProfileString(pItem,"PauseStart","100",pPause,nBufferSize,pInitFile);
		Sleep(atoi(pPause));
		return TRUE;
	}
	else
	{
		long nError = GetLastError();
		char pTemp[121];
		sprintf(pTemp,"Failed to start program '%s', error code = %d\n", pCommandLine, nError); 
		WriteLog(pLogFile, pTemp);
		return FALSE;
	}
}

void EndProcess(int nIndex) 
{	
	// end a program started by the service
	char pItem[nBufferSize+1];
	sprintf(pItem,"Process%d\0",nIndex);
	char pPause[nBufferSize+1];
	GetPrivateProfileString(pItem,"PauseEnd","100",pPause,nBufferSize,pInitFile);
	int nPauseEnd = atoi(pPause);
	if(nIndex>=0&&nIndex<nProcCount)
	{
		if(pProcInfo[nIndex].hProcess)
		{
			// post a WM_QUIT message first
			PostThreadMessage(pProcInfo[nIndex].dwThreadId,WM_QUIT,0,0);
			// sleep for a while so that the process has a chance to terminate itself
			::Sleep(nPauseEnd>0?nPauseEnd:50);
			// terminate the process by force
			TerminateProcess(pProcInfo[nIndex].hProcess,0);
		}
	}
}

BOOL BounceProcess(char* pName, int nIndex) 
{ 
	// bounce the process with given index
	SC_HANDLE schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS); 
	if (schSCManager==0) 
	{
		long nError = GetLastError();
		char pTemp[121];
		sprintf(pTemp, "OpenSCManager failed, error code = %d\n", nError);
		WriteLog(pLogFile, pTemp);
	}
	else
	{
		// open the service
		SC_HANDLE schService = OpenService( schSCManager, pName, SERVICE_ALL_ACCESS);
		if (schService==0) 
		{
			long nError = GetLastError();
			char pTemp[121];
			sprintf(pTemp, "OpenService failed, error code = %d\n", nError); 
			WriteLog(pLogFile, pTemp);
		}
		else
		{
			// call ControlService to invoke handler
			SERVICE_STATUS status;
			if(nIndex>=0&&nIndex<128)
			{
				if(ControlService(schService,(nIndex|0x80),&status))
				{
					CloseServiceHandle(schService); 
					CloseServiceHandle(schSCManager); 
					return TRUE;
				}
				long nError = GetLastError();
				char pTemp[121];
				sprintf(pTemp, "ControlService failed, error code = %d\n", nError); 
				WriteLog(pLogFile, pTemp);
			}
			else
			{
				char pTemp[121];
				sprintf(pTemp, "Invalid argument to BounceProcess: %d\n", nIndex); 
				WriteLog(pLogFile, pTemp);
			}
			CloseServiceHandle(schService); 
		}
		CloseServiceHandle(schSCManager); 
	}
	return FALSE;
}

BOOL KillService(char* pName) 
{ 
	// kill service with given name
	SC_HANDLE schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS); 
	if (schSCManager==0) 
	{
		long nError = GetLastError();
		char pTemp[121];
		sprintf(pTemp, "OpenSCManager failed, error code = %d\n", nError);
		WriteLog(pLogFile, pTemp);
	}
	else
	{
		// open the service
		SC_HANDLE schService = OpenService( schSCManager, pName, SERVICE_ALL_ACCESS);
		if (schService==0) 
		{
			long nError = GetLastError();
			char pTemp[121];
			sprintf(pTemp, "OpenService failed, error code = %d\n", nError);
			WriteLog(pLogFile, pTemp);
		}
		else
		{
			// call ControlService to kill the given service
			SERVICE_STATUS status;
			if(ControlService(schService,SERVICE_CONTROL_STOP,&status))
			{
				CloseServiceHandle(schService); 
				CloseServiceHandle(schSCManager); 
				return TRUE;
			}
			else
			{
				long nError = GetLastError();
				char pTemp[121];
				sprintf(pTemp, "ControlService failed, error code = %d\n", nError);
				WriteLog(pLogFile, pTemp);
			}
			CloseServiceHandle(schService); 
		}
		CloseServiceHandle(schSCManager); 
	}
	return FALSE;
}

BOOL RunService(char* pName, int nArg, char** pArg) 
{ 
	// run service with given name
	SC_HANDLE schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS); 
	if (schSCManager==0) 
	{
		long nError = GetLastError();
		char pTemp[121];
		sprintf(pTemp, "OpenSCManager failed, error code = %d\n", nError);
		WriteLog(pLogFile, pTemp);
	}
	else
	{
		// open the service
		SC_HANDLE schService = OpenService( schSCManager, pName, SERVICE_ALL_ACCESS);
		if (schService==0) 
		{
			long nError = GetLastError();
			char pTemp[121];
			sprintf(pTemp, "OpenService failed, error code = %d\n", nError);
			WriteLog(pLogFile, pTemp);
		}
		else
		{
			// call StartService to run the service
			if(StartService(schService,nArg,(const char**)pArg))
			{
				CloseServiceHandle(schService); 
				CloseServiceHandle(schSCManager); 
				return TRUE;
			}
			else
			{
				long nError = GetLastError();
				char pTemp[121];
				sprintf(pTemp, "StartService failed, error code = %d\n", nError);
				WriteLog(pLogFile, pTemp);
			}
			CloseServiceHandle(schService); 
		}
		CloseServiceHandle(schSCManager); 
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////// 
//
// This routine gets used to start your service
//
VOID WINAPI ORMapServiceMain( DWORD dwArgc, LPTSTR *lpszArgv )
{
	DWORD   status = 0; 
    DWORD   specificError = 0xfffffff; 
 
    serviceStatus.dwServiceType        = SERVICE_WIN32; 
    serviceStatus.dwCurrentState       = SERVICE_START_PENDING; 
    serviceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE; 
    serviceStatus.dwWin32ExitCode      = 0; 
    serviceStatus.dwServiceSpecificExitCode = 0; 
    serviceStatus.dwCheckPoint         = 0; 
    serviceStatus.dwWaitHint           = 0; 
 
    hServiceStatusHandle = RegisterServiceCtrlHandler(pServiceName, ORMapServiceHandler); 
    if (hServiceStatusHandle==0) 
    {
		long nError = GetLastError();
		char pTemp[121];
		sprintf(pTemp, "RegisterServiceCtrlHandler failed, error code = %d\n", nError);
		WriteLog(pLogFile, pTemp);
        return; 
    } 
 
    // Handle error condition 
	status = GetLastError(); 
    if (status!=NO_ERROR) 
	{ 
        serviceStatus.dwCurrentState       = SERVICE_STOPPED; 
        serviceStatus.dwCheckPoint         = 0; 
        serviceStatus.dwWaitHint           = 0; 
        serviceStatus.dwWin32ExitCode      = status; 
        serviceStatus.dwServiceSpecificExitCode = specificError; 
        SetServiceStatus(hServiceStatusHandle, &serviceStatus); 
        return; 
    } 
 
    // Initialization complete - report running status 
    serviceStatus.dwCurrentState       = SERVICE_RUNNING; 
    serviceStatus.dwCheckPoint         = 0; 
    serviceStatus.dwWaitHint           = 0;  
    if(!SetServiceStatus(hServiceStatusHandle, &serviceStatus)) 
    { 
		long nError = GetLastError();
		char pTemp[121];
		sprintf(pTemp, "SetServiceStatus failed, error code = %d\n", nError);
		WriteLog(pLogFile, pTemp);
    } 

	for(int i=0;i<nProcCount;i++)
	{
		pProcInfo[i].hProcess = 0;
		StartProcess(i);
	}
}

////////////////////////////////////////////////////////////////////// 
//
// This routine responds to events concerning your service, like start/stop
//
VOID WINAPI ORMapServiceHandler(DWORD fdwControl)
{
	switch(fdwControl) 
	{
		case SERVICE_CONTROL_STOP:
		case SERVICE_CONTROL_SHUTDOWN:
			serviceStatus.dwWin32ExitCode = 0; 
			serviceStatus.dwCurrentState  = SERVICE_STOPPED; 
			serviceStatus.dwCheckPoint    = 0; 
			serviceStatus.dwWaitHint      = 0;
			// terminate all processes started by this service before shutdown
			{
				for(int i=nProcCount-1;i>=0;i--)
				{
					EndProcess(i);
				}			
				if (!SetServiceStatus(hServiceStatusHandle, &serviceStatus))
				{ 
					long nError = GetLastError();
					char pTemp[121];
					sprintf(pTemp, "SetServiceStatus failed, error code = %d\n", nError);
					WriteLog(pLogFile, pTemp);
				}
			}
			return; 
		case SERVICE_CONTROL_PAUSE:
			serviceStatus.dwCurrentState = SERVICE_PAUSED; 
			break;
		case SERVICE_CONTROL_CONTINUE:
			serviceStatus.dwCurrentState = SERVICE_RUNNING; 
			break;
		case SERVICE_CONTROL_INTERROGATE:
			break;
		default: 
			// bounce processes started by this service
			if(fdwControl>=128&&fdwControl<256)
			{
				int nIndex = fdwControl&0x7F;
				// bounce a single process
				if(nIndex>=0&&nIndex<nProcCount)
				{
					EndProcess(nIndex);
					StartProcess(nIndex);
				}
				// bounce all processes
				else if(nIndex==127)
				{
					for(int i=nProcCount-1;i>=0;i--)
					{
						EndProcess(i);
					}
					for(i=0;i<nProcCount;i++)
					{
						StartProcess(i);
					}
				}
			}
			else
			{
				long nError = GetLastError();
				char pTemp[121];
				sprintf(pTemp,  "Unrecognized opcode %d\n", fdwControl);
				WriteLog(pLogFile, pTemp);
			}
	};
    if (!SetServiceStatus(hServiceStatusHandle,  &serviceStatus)) 
	{ 
		long nError = GetLastError();
		char pTemp[121];
		sprintf(pTemp, "SetServiceStatus failed, error code = %d\n", nError);
		WriteLog(pLogFile, pTemp);
    } 
}


////////////////////////////////////////////////////////////////////// 
//
// Uninstall
//
VOID UnInstall(char* pName)
{
	SC_HANDLE schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS); 
	if (schSCManager==0) 
	{
		long nError = GetLastError();
		char pTemp[121];
		sprintf(pTemp, "OpenSCManager failed, error code = %d\n", nError);
		WriteLog(pLogFile, pTemp);
	}
	else
	{
		SC_HANDLE schService = OpenService( schSCManager, pName, SERVICE_ALL_ACCESS);
		if (schService==0) 
		{
			long nError = GetLastError();
			char pTemp[121];
			sprintf(pTemp, "OpenService failed, error code = %d\n", nError);
			WriteLog(pLogFile, pTemp);
		}
		else
		{
			if(!DeleteService(schService)) 
			{
				char pTemp[121];
				sprintf(pTemp, "Failed to delete service %s\n", pName);
				WriteLog(pLogFile, pTemp);
			}
			else 
			{
				char pTemp[121];
				sprintf(pTemp, "Service %s removed\n",pName);
				WriteLog(pLogFile, pTemp);
			}
			CloseServiceHandle(schService); 
		}
		CloseServiceHandle(schSCManager);	
	}
}

////////////////////////////////////////////////////////////////////// 
//
// Install
//
VOID Install(char* pPath, char* pName) 
{  
	SC_HANDLE schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_CREATE_SERVICE); 
	if (schSCManager==0) 
	{
		long nError = GetLastError();
		char pTemp[121];
		sprintf(pTemp, "OpenSCManager failed, error code = %d\n", nError);
		WriteLog(pLogFile, pTemp);
	}
	else
	{
		SC_HANDLE schService = CreateService
		( 
			schSCManager,	/* SCManager database      */ 
			pName,			/* name of service         */ 
			pName,			/* service name to display */ 
			SERVICE_ALL_ACCESS,        /* desired access          */ 
			SERVICE_WIN32_OWN_PROCESS|SERVICE_INTERACTIVE_PROCESS , /* service type            */ 
			SERVICE_AUTO_START,      /* start type              */ 
			SERVICE_ERROR_NORMAL,      /* error control type      */ 
			pPath,			/* service's binary        */ 
			NULL,                      /* no load ordering group  */ 
			NULL,                      /* no tag identifier       */ 
			NULL,                      /* no dependencies         */ 
			NULL,                      /* LocalSystem account     */ 
			NULL
		);                     /* no password             */ 
		if (schService==0) 
		{
			long nError =  GetLastError();
			char pTemp[121];
			sprintf(pTemp, "Failed to create service %s, error code = %d\n", pName, nError);
			WriteLog(pLogFile, pTemp);
		}
		else
		{
			char pTemp[121];
			sprintf(pTemp, "Service %s installed\n", pName);
			WriteLog(pLogFile, pTemp);
			CloseServiceHandle(schService); 
		}
		CloseServiceHandle(schSCManager);
	}	
}

void WorkerProc(void* pParam)
{
	char pCheckProcess[nBufferSize+1];
	GetPrivateProfileString("Settings","CheckProcess","60",pCheckProcess, nBufferSize,pInitFile);
	int nCheckProcess = atoi(pCheckProcess);
	while(nCheckProcess>0&&nProcCount>0)
	{
		::Sleep(1000*60*nCheckProcess);
		for(int i=0;i<nProcCount;i++)
		{
			char pItem[nBufferSize+1];
			sprintf(pItem,"Process%d\0",i);
			char pRestart[nBufferSize+1];
			GetPrivateProfileString(pItem,"Restart","No",pRestart,nBufferSize,pInitFile);
			if(pRestart[0]=='Y'||pRestart[0]=='y'||pRestart[0]=='1')
			{
				DWORD dwCode;
				if(::GetExitCodeProcess(pProcInfo[i].hProcess, &dwCode))
				{
					if(dwCode!=STILL_ACTIVE)
					{
						if(StartProcess(i))
						{
							char pTemp[121];
							sprintf(pTemp, "Restarted process %d\n", i);
							WriteLog(pLogFile, pTemp);
						}
					}
				}
				else
				{
					long nError = GetLastError();
					char pTemp[121];
					sprintf(pTemp, "GetExitCodeProcess failed, error code = %d\n", nError);
					WriteLog(pLogFile, pTemp);
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////// 
//
// Standard C Main
//
void main(int argc, char *argv[] )
{
	// initialize global critical section
	::InitializeCriticalSection(&myCS);
	// initialize variables for .exe, .ini, and .log file names
	char pModuleFile[nBufferSize+1];
	DWORD dwSize = GetModuleFileName(NULL,pModuleFile,nBufferSize);
	pModuleFile[dwSize] = 0;
	if(dwSize>4&&pModuleFile[dwSize-4]=='.')
	{
		sprintf(pExeFile,"%s",pModuleFile);
		pModuleFile[dwSize-4] = 0;
		sprintf(pInitFile,"%s.ini",pModuleFile);
		sprintf(pLogFile,"%s.log",pModuleFile);
	}
	else
	{
		sprintf(pExeFile,"%s",argv[0]);
		sprintf(pInitFile,"%s","ORMapService.ini");
		sprintf(pLogFile,"%s","ORMapService.log");
	}
	// read service name from .ini file
	GetPrivateProfileString("Settings","ServiceName","ORMapService",pServiceName,nBufferSize,pInitFile);
	// read program count from .ini file
	char pCount[nBufferSize+1];
	GetPrivateProfileString("Settings","ProcCount","",pCount,nBufferSize,pInitFile);
	nProcCount = atoi(pCount);
	// initialize process information array
	if(nProcCount>0)
	{
		pProcInfo = new PROCESS_INFORMATION[nProcCount];
	}
	// uninstall service if switch is "-u"
	if(argc==2&&_stricmp("-u",argv[1])==0)
	{
		UnInstall(pServiceName);
	}
	// install service if switch is "-i"
	else if(argc==2&&_stricmp("-i",argv[1])==0)
	{			
		Install(pExeFile, pServiceName);
	}
	// bounce service if switch is "-b"
	else if(argc==2&&_stricmp("-b",argv[1])==0)
	{			
		KillService(pServiceName);
		RunService(pServiceName,0,NULL);
	}
	// bounce a specifc program if the index is supplied
	else if(argc==3&&_stricmp("-b",argv[1])==0)
	{
		int nIndex = atoi(argv[2]);
		if(BounceProcess(pServiceName, nIndex))
		{
			char pTemp[121];
			sprintf(pTemp, "Bounced process %d.\n", nIndex);
			WriteLog(pLogFile, pTemp);
		}
		else
		{
			char pTemp[121];
			sprintf(pTemp, "Failed to bounce process %d.\n", nIndex);
			WriteLog(pLogFile, pTemp);
		}
	}
	// kill a service with given name
	else if(argc==3&&_stricmp("-k",argv[1])==0)
	{
		if(KillService(argv[2]))
		{
			char pTemp[121];
			sprintf(pTemp, "Killed service %s.\n", argv[2]);
			WriteLog(pLogFile, pTemp);
		}
		else
		{
			char pTemp[121];
			sprintf(pTemp, "Failed to kill service %s.\n", argv[2]);
			WriteLog(pLogFile, pTemp);
		}
	}
	// run a service with given name
	else if(argc>=3&&_stricmp("-r",argv[1])==0)
	{
		if(RunService(argv[2], argc>3?(argc-3):0,argc>3?(&(argv[3])):NULL))
		{
			char pTemp[121];
			sprintf(pTemp, "Ran service %s.\n", argv[2]);
			WriteLog(pLogFile, pTemp);
		}
		else
		{
			char pTemp[121];
			sprintf(pTemp, "Failed to run service %s.\n", argv[2]);
			WriteLog(pLogFile, pTemp);
		}
	}
	// assume user is starting this service 
	else 
	{
		// start a worker thread to check for dead programs (and restart if necessary)
		if(_beginthread(WorkerProc, 0, NULL)==-1)
		{
			long nError = GetLastError();
			char pTemp[121];
			sprintf(pTemp, "_beginthread failed, error code = %d\n", nError);
			WriteLog(pLogFile, pTemp);
		}
		// pass dispatch table to service controller
		if(!StartServiceCtrlDispatcher(DispatchTable))
		{
			long nError = GetLastError();
			char pTemp[121];
			sprintf(pTemp, "StartServiceCtrlDispatcher failed, error code = %d\n", nError);
			WriteLog(pLogFile, pTemp);
		}
		// you don't get here unless the service is shutdown
	}
	// clean up
	delete []pProcInfo;
	::DeleteCriticalSection(&myCS);
}

