//
// Copyright (C) Anders Kjersem. Licensed under the zlib/libpng license
//

#include "InetBgDL.h"

#define USERAGENT _T("NSIS InetBgDL (Mozilla)")

#define STATUS_COMPLETEDALL 0
#define STATUS_INITIAL 202 // 202 = Accepted
#define STATUS_CONNECTING STATUS_INITIAL //102
#define STATUS_DOWNLOADING STATUS_INITIAL
#define STATUS_ERR_GETLASTERROR 418 //HTTP: I'm a teapot: Win32 error code in $3
#define STATUS_ERR_LOCALFILEWRITEERROR 450 //HTTP: MS parental control extension
#define STATUS_ERR_CANCELLED 499

typedef DWORD FILESIZE_T; // Limit to 4GB for now...
#define FILESIZE_UNKNOWN (-1)


HINSTANCE g_hInst;
NSIS::stack_t*g_pLocations = NULL;
HANDLE g_hThread = NULL;
volatile UINT g_FilesTotal = 0;
volatile UINT g_FilesCompleted = 0;
volatile UINT g_Status = STATUS_INITIAL;
volatile FILESIZE_T g_cbCurrXF;
volatile FILESIZE_T g_cbCurrTot = FILESIZE_UNKNOWN;
CRITICAL_SECTION g_CritLock;
UINT g_N_CCH;
PTSTR g_N_Vars;

#define NSISPI_INITGLOBALS(N_CCH,N_Vars) do { \
	g_N_CCH = N_CCH; \
	g_N_Vars = N_Vars; \
	} while(0)

#define Stats_SetWin32Err(ec) do{ g_cbCurrXF=(ec); }while(0)

#define ONELOCKTORULETHEMALL
#ifdef ONELOCKTORULETHEMALL
#define TaskLock_Initialize() InitializeCriticalSection(&g_CritLock)
#define TaskLock_Delete() DeleteCriticalSection(&g_CritLock)
#define TaskLock_AcquireExclusive() EnterCriticalSection(&g_CritLock)
#define TaskLock_ReleaseExclusive() LeaveCriticalSection(&g_CritLock)
#define StatsLock_Initialize()
#define StatsLock_Delete()
#define StatsLock_AcquireExclusive() TaskLock_AcquireExclusive()
#define StatsLock_ReleaseExclusive() TaskLock_ReleaseExclusive()
#define StatsLock_AcquireShared() StatsLock_AcquireExclusive()
#define StatsLock_ReleaseShared() StatsLock_ReleaseExclusive()
#endif


static PTSTR NSIS_SetRegStr(UINT Reg,PCTSTR Value)
{
	PTSTR s = g_N_Vars + (Reg * g_N_CCH);
	lstrcpy(s,Value);
	return s;
}
#define NSIS_SetRegStrEmpty(r) NSIS_SetRegStr(r,_T(""))
static void NSIS_SetRegUINT(UINT Reg,UINT Value)
{
	TCHAR buf[32];
	wsprintf(buf,_T("%u"),Value);
	NSIS_SetRegStr(Reg,buf);
}
#define StackFreeItem(pI) GlobalFree(pI)
static NSIS::stack_t* StackPopItem(NSIS::stack_t**ppST) 
{
	if (*ppST) 
	{
		NSIS::stack_t*pItem = *ppST;
		*ppST = pItem->next;
		return pItem;
	}
	return NULL;
}

#ifdef PLUGIN_DEBUG
static void DumpWinInetDebugInfo(HINTERNET hInetSes)
{
	UINT32 u32;
	DWORD cbio;
	BOOL bRet;

	static bool once = false;
	if (!once)
	{
		++once;
		INTERNET_VERSION_INFO ivi;
		cbio = sizeof(ivi), bRet = InternetQueryOption(hInetSes,INTERNET_OPTION_VERSION,&ivi,&cbio);
		TRACE2(_T("INTERNET_OPTION_VERSION=%u.%u"),ivi.dwMajorVersion,ivi.dwMinorVersion);

		cbio = sizeof(u32), bRet = InternetQueryOption(hInetSes,INTERNET_OPTION_CONNECT_TIMEOUT,&u32,&cbio);
		TRACE2(bRet?_T("%s=%u"):_T("%s=?"),_T("INTERNET_OPTION_CONNECT_TIMEOUT"),u32);
		cbio = sizeof(u32), bRet = InternetQueryOption(hInetSes,INTERNET_OPTION_SEND_TIMEOUT,&u32,&cbio);
		TRACE2(bRet?_T("%s=%u"):_T("%s=?"),_T("INTERNET_OPTION_SEND_TIMEOUT"),u32);
		cbio = sizeof(u32), bRet = InternetQueryOption(hInetSes,INTERNET_OPTION_RECEIVE_TIMEOUT,&u32,&cbio);
		TRACE2(bRet?_T("%s=%u"):_T("%s=?"),_T("INTERNET_OPTION_RECEIVE_TIMEOUT"),u32);
		cbio = sizeof(u32), bRet = InternetQueryOption(hInetSes,INTERNET_OPTION_CONNECT_RETRIES,&u32,&cbio);
		TRACE2(bRet?_T("%s=%u"):_T("%s=?"),_T("INTERNET_OPTION_CONNECT_RETRIES"),u32);
	}
	cbio = sizeof(u32), bRet = InternetQueryOption(hInetSes,INTERNET_OPTION_CONNECTED_STATE,&u32,&cbio);
	TRACE2(bRet?_T("%s=%u"):_T("%s=?"),_T("INTERNET_OPTION_CONNECTED_STATE"),u32);
	cbio = sizeof(u32), bRet = InternetQueryOption(hInetSes,INTERNET_OPTION_IGNORE_OFFLINE,&u32,&cbio);
	TRACE2(bRet?_T("%s=%u"):_T("%s=?"),_T("INTERNET_OPTION_IGNORE_OFFLINE"),u32);
}
#endif

static void Reset() 
{
	TRACEA("InetBgDl: Reset()\n");
	StatsLock_AcquireExclusive();
	g_FilesTotal = 0; // This causes the Task thread to exit the transfer loop
	StatsLock_ReleaseExclusive();

	if (WAIT_OBJECT_0 != WaitForSingleObject(g_hThread,1000*10))
	{
		TRACE1(_T("InetBgDl: terminating g_hThread=%x\n"),g_hThread);
		TerminateThread(g_hThread,ERROR_OPERATION_ABORTED);
	}
	TaskLock_AcquireExclusive();
	CloseHandle(g_hThread);
	g_hThread = NULL;
#ifndef ONELOCKTORULETHEMALL
	StatsLock_AcquireExclusive();
#endif
	g_FilesTotal = 0;
	g_FilesCompleted = 0;
	g_Status = STATUS_INITIAL;
#ifndef ONELOCKTORULETHEMALL
	StatsLock_ReleaseExclusive();
#endif
	for (NSIS::stack_t*pTmpTast,*pTask = g_pLocations; pTask;)
	{
		pTmpTast = pTask, pTask = pTask->next;
		StackFreeItem(pTmpTast);
	}
	g_pLocations = NULL;
	TaskLock_ReleaseExclusive();
}

UINT_PTR __cdecl NSISPluginCallback(UINT Event) 
{
	switch(Event) 
	{
	case NSPIM_UNLOAD:
		Reset();
		TaskLock_Delete();
		StatsLock_Delete();
		break;
	}
	return NULL;
}

static DWORD CALLBACK TaskThreadProc(LPVOID ThreadParam)
{
	NSIS::stack_t *pURL,*pFile;
	HINTERNET hInetSes = NULL;
	HANDLE hLocalFile;
	bool completedFile = false;

startnexttask:
	hLocalFile = NULL;
	pFile = NULL;
	TaskLock_AcquireExclusive();
	pURL = g_pLocations;
	if (pURL)
	{
		pFile = pURL->next;
		g_pLocations = pFile->next;
	}
#ifndef ONELOCKTORULETHEMALL
	StatsLock_AcquireExclusive();
#endif
	if (completedFile) ++g_FilesCompleted;
	completedFile = false;
	g_Status = STATUS_INITIAL;
	Stats_SetWin32Err(ERROR_SUCCESS);
	g_cbCurrXF = 0;
	g_cbCurrTot = FILESIZE_UNKNOWN;
	if (!pURL)
	{
		if (g_FilesTotal)
		{
			if (g_FilesTotal == g_FilesCompleted) g_Status = STATUS_COMPLETEDALL;
		}
	}
#ifndef ONELOCKTORULETHEMALL
	StatsLock_ReleaseExclusive();
#endif
	TaskLock_ReleaseExclusive();

	if (!pURL)
	{
		if (false)
		{
diegle:
			DWORD gle = GetLastError();
			StatsLock_AcquireExclusive();
			//TODO? if (ERROR_INTERNET_EXTENDED_ERROR==gle) InternetGetLastResponseInfo(...)
			Stats_SetWin32Err(gle);
			g_Status = STATUS_ERR_GETLASTERROR;
			//if (pFile) DeleteFile(pFile->text);
			StatsLock_ReleaseExclusive();
		}
		InternetCloseHandle(hInetSes);
		CloseHandle(hLocalFile);
		StackFreeItem(pURL);
		StackFreeItem(pFile);
		TRACEA("InetBgDl: TaskThreadProc returning...\n");
		return 0;
	}

	if (!hInetSes)
	{
		hInetSes = InternetOpen(USERAGENT,INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL,0);
		if (!hInetSes)
		{
			TRACE1(_T("InetBgDl: InternetOpen failed, GLE=%u\n"),GetLastError());
			goto diegle;
		}
#ifdef PLUGIN_DEBUG
		DumpWinInetDebugInfo(hInetSes);
#endif

		//msdn.microsoft.com/library/default.asp?url=/workshop/components/offline/offline.asp#Supporting Offline Browsing in Applications and Components
		DWORD cbio = sizeof(ULONG), Opt32;
		if (InternetQueryOption(hInetSes,INTERNET_OPTION_CONNECTED_STATE,&Opt32,&cbio))
		{
			if (INTERNET_STATE_DISCONNECTED_BY_USER&Opt32)
			{
				INTERNET_CONNECTED_INFO ci = {INTERNET_STATE_CONNECTED, 0};
				InternetSetOption(hInetSes,INTERNET_OPTION_CONNECTED_STATE,&ci,sizeof(ci));
			}
		}
	}

	DWORD ec = ERROR_SUCCESS;
	hLocalFile = CreateFile(pFile->text,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_DELETE,NULL,CREATE_ALWAYS,0,NULL);
	if (INVALID_HANDLE_VALUE == hLocalFile) goto diegle;

	const DWORD IOUFTPFlags = INTERNET_FLAG_PASSIVE;
	const DWORD IOURedirFlags = INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS;
	const DWORD IOUCacheFlags = INTERNET_FLAG_RESYNCHRONIZE | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_PRAGMA_NOCACHE;
	const DWORD IOUCookieFlags = INTERNET_FLAG_NO_COOKIES;
	DWORD IOUFlags = IOUFTPFlags | IOURedirFlags | IOUCacheFlags | IOUCookieFlags | INTERNET_FLAG_NO_UI | INTERNET_FLAG_EXISTING_CONNECT;
	{
		const TCHAR chTmp = pURL->text[6];
		pURL->text[6] = 0;
		if (0==lstrcmpi(_T("HTTPS:"),pURL->text)) IOUFlags |= INTERNET_FLAG_SECURE;
		pURL->text[6] = chTmp;
	}
	TRACE2(_T("InetBgDl: InternetOpenUrl(%s,%#x)\n"),pURL->text,IOUFlags);
	HINTERNET hInetFile = InternetOpenUrl(hInetSes,pURL->text,NULL,0,IOUFlags,NULL);
	if (!hInetFile) goto diegle;

	FILESIZE_T cbThisFile;
	DWORD cbio = sizeof(cbThisFile);
	if (!HttpQueryInfo(hInetFile,HTTP_QUERY_CONTENT_LENGTH|HTTP_QUERY_FLAG_NUMBER,&cbThisFile,&cbio,NULL))
	{
		cbThisFile = FILESIZE_UNKNOWN;
		TRACEA("InetBgDl: Filesize unknown...\n"); 
	}

#ifdef INETBGDL_BLDCONF_LONG_RECEIVETIMEOUT
	DWORD Opt32 = 15*60*1000;
	InternetSetOption(hInetFile,INTERNET_OPTION_RECEIVE_TIMEOUT,&Opt32,sizeof(Opt32));
#endif

#ifdef INETBGDL_BLDCONF_UPDATEHTTPSTATUS
	DWORD httpcode;
	cbio = sizeof(DWORD);
	if (HttpQueryInfo(hInetFile,HTTP_QUERY_STATUS_CODE|HTTP_QUERY_FLAG_NUMBER,&httpcode,&cbio,NULL))
	{
		StatsLock_AcquireExclusive();
		g_Status = httpcode;
		StatsLock_ReleaseExclusive();
	}
#endif

	for(;;)
	{
#if PLUGIN_DEBUG
		const UINT cbBufXF = 512;
		BYTE bufXF[cbBufXF];
#else
		const UINT cbBufXF = 1024 * sizeof(TCHAR);
		BYTE*bufXF = (BYTE*) pURL->text; // NOTE: Cannot use pURL as buffer if you want to TRACE the URL!
#endif
		DWORD cbio,cbXF;
		BOOL retXF = InternetReadFile(hInetFile,bufXF,cbBufXF,&cbio);
		if (!retXF)
		{
			ec = GetLastError();
			TRACE1(_T("InetBgDl: InternetReadFile failed, GLE=%u\n"),ec);
			break;
		}
		if (0==cbio)
		{
			ASSERT(ERROR_SUCCESS==ec);
			// EOF or broken connection?
			// TODO: Can InternetQueryDataAvailable detect this?
			
			TRACE2(_T("InetBgDl: InternetReadFile true with cbio=0, cbThisFile=%d, GLE=%u\n"),cbThisFile,GetLastError());
			if (FILESIZE_UNKNOWN != cbThisFile)
			{
				// Reading without StatsLock is ok in this thread...
				if (g_cbCurrXF != cbThisFile) ec = ERROR_BROKEN_PIPE;
			}
			break;
		}

		if (0==g_FilesTotal)
		{
			TRACEA("InetBgDl: 0==g_FilesTotal, aborting transfer loop...\n");
			ec = ERROR_CANCELLED;
			break;
		}

		cbXF = cbio;
		if (cbXF)
		{
			retXF = WriteFile(hLocalFile,bufXF,cbXF,&cbio,NULL);
			if (!retXF || cbXF!=cbio)
			{
				ec = GetLastError();
				break;
			}
			
			StatsLock_AcquireExclusive();
			if (FILESIZE_UNKNOWN != cbThisFile) g_cbCurrTot = cbThisFile;
			g_cbCurrXF += cbXF;
			StatsLock_ReleaseExclusive();
		}
	}

	TRACE2(_T("InetBgDl: TaskThreadProc processed %s, ec=%u\n"),pURL->text,ec);
	InternetCloseHandle(hInetFile);
	CloseHandle(hLocalFile);
	StackFreeItem(pURL);
	StackFreeItem(pFile);
	if (ERROR_SUCCESS == ec)
	{
		++completedFile;
	}
	else
	{
		SetLastError(ec);
		goto diegle;
	}
	goto startnexttask;
}

NSISPIEXPORTFUNC Get(HWND hwndNSIS,UINT N_CCH,TCHAR*N_Vars,NSIS::stack_t**ppST,NSIS::xparams_t*pX)
{
	pX->RegisterPluginCallback(g_hInst,NSISPluginCallback);
	for (;;)
	{
		NSIS::stack_t*pURL = StackPopItem(ppST);
		if (!pURL) break;

		if (0==lstrcmpi(pURL->text,_T("/END")))
		{
freeurlandexit:
			StackFreeItem(pURL);
			break;
		}
		if (0==lstrcmpi(pURL->text,_T("/RESET")))
		{
			StackFreeItem(pURL);
			Reset();
			continue;
		}
		
		NSIS::stack_t*pFile = StackPopItem(ppST);
		if (!pFile) goto freeurlandexit;

		TaskLock_AcquireExclusive();

		pFile->next = NULL;
		pURL->next = pFile;
		NSIS::stack_t*pTasks = g_pLocations;
		if (pTasks)
		{
			while(pTasks->next) pTasks = pTasks->next;
			pTasks->next = pURL;
		}
		else
		{
			g_pLocations = pURL;
		}

		if (!g_hThread)
		{
			DWORD tid;tid;
			g_hThread = CreateThread(NULL,0,TaskThreadProc,NULL,0,sizeof(TCHAR)>1?NULL:&tid);
			TRACE1(_T("InetBgDl: Started worker thread %x...\n"),g_hThread);
			SetThreadPriority(g_hThread,THREAD_PRIORITY_LOWEST);
		}

#ifndef ONELOCKTORULETHEMALL
		StatsLock_AcquireExclusive();
#endif
		++g_FilesTotal;
#ifndef ONELOCKTORULETHEMALL
		StatsLock_ReleaseExclusive();
#endif
		TaskLock_ReleaseExclusive();
	}
}

NSISPIEXPORTFUNC GetStats(HWND hwndNSIS,UINT N_CCH,TCHAR*N_Vars,NSIS::stack_t**ppST,NSIS::xparams_t*pX)
{
	NSISPI_INITGLOBALS(N_CCH,N_Vars); 
	StatsLock_AcquireShared();
	NSIS_SetRegUINT(0,g_Status);
	NSIS_SetRegUINT(1,g_FilesCompleted);
	NSIS_SetRegUINT(2,g_FilesTotal-g_FilesCompleted);
	NSIS_SetRegUINT(3,g_cbCurrXF);
	NSIS_SetRegStrEmpty(4);
	if (FILESIZE_UNKNOWN != g_cbCurrTot) NSIS_SetRegUINT(4,g_cbCurrTot);
	StatsLock_ReleaseShared();
}

EXTERN_C BOOL WINAPI _DllMainCRTStartup(HMODULE hInst,UINT Reason,LPVOID pCtx)
{
	if (DLL_PROCESS_ATTACH==Reason)
	{
		g_hInst=hInst;
		TaskLock_Initialize();
		StatsLock_Initialize();
	}
	return TRUE;
}
