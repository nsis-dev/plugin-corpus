/*
TODO:
=====
*BgWorker::QueueWork [/NewThread] workfunc completedcallbackfunc & BgWorker::WaitForWork workfunc & BgWorker::WaitForAll

HISTORY:
========
20091001 (AndersK)
	*Initial version
*/


#if (defined(_MSC_VER) && !defined(_DEBUG))
	#pragma comment(linker,"/opt:nowin98")
	#pragma comment(linker,"/ignore:4078")
	#pragma comment(linker,"/merge:.rdata=.text")
#endif

#include <Windows.h>
#include <WINNT.H>

#define NSISCALL __stdcall
typedef struct {
  LPVOID xxx1;//exec_flags_type *exec_flags;
  int (NSISCALL *ExecuteCodeSegment)(int, HWND);
  void (NSISCALL *validate_filename)(char *);
  int (NSISCALL *RegisterPluginCallback)(HMODULE,LPVOID);
} extra_parameters;

#define N_r0 0
#define N_v_tmp 25
#define N_v_pluginsdir 26
#define N_VarPtrA(varid,NVars,NCCH) ( (( char*)(NVars)) + ((varid) * (NCCH)) )
#define N_VarPtrW(varid,NVars,NCCH) ( ((WCHAR*)(NVars)) + ((varid) * (NCCH)) )


typedef struct 
{
	HWND hwndNSIS;
	int CCH;
	char*Vars;
} NSIS,*PNSIS;

bool _NSIS_PtrIsUnicodeFullPath(NSIS &nsis,UINT varid) 
{
	WCHAR* varW=N_VarPtrW(varid,nsis.Vars,nsis.CCH);
	if (!IsBadReadPtr(varW,nsis.CCH)) 
	{
		//Let's hope $xxx is always "?:\*" or "\\*"
		return (varW[0]==L'\\' && varW[1]==L'\\') || (varW[0]>='A' && varW[0]<='z' && varW[1]==L':');
	}
	return false;
}

bool NSIS_IsUnicode(NSIS &nsis)
{
	if (nsis.hwndNSIS)return !!IsWindowUnicode(nsis.hwndNSIS);

/*#if defined(_M_IX86) && defined(_MSC_VER)
	BYTE* pPEBLDRDATA;
	__asm {
		;assume fs:nothing
		mov eax, fs:[30h]		;PEB
		mov eax, [eax+0Ch]		;PEB_LDR_DATA
		mov pPEBLDRDATA,eax
	};
	BYTE* pLibList=pPEBLDRDATA[sizof(void*)*3];
#endif*/
#if 1
	char*xfunc="lstrcatW";
	LPBYTE pBase=(LPBYTE)GetModuleHandle(NULL);
	PIMAGE_NT_HEADERS pINTH=(PIMAGE_NT_HEADERS) (pBase+((PIMAGE_DOS_HEADER)pBase)->e_lfanew);
	DWORD importRVA=pINTH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc=(PIMAGE_IMPORT_DESCRIPTOR)(((char*)pBase)+(UINT_PTR)importRVA);
	while(pImportDesc->FirstThunk) 
	{
		PSTR pszImportModuleName =(PSTR)(((char*)pBase)+(UINT_PTR)pImportDesc->Name);
		if (0==lstrcmpiA(pszImportModuleName,"KERNEL32.DLL") && pImportDesc->OriginalFirstThunk) 
		{
			PIMAGE_THUNK_DATA pIAT=(PIMAGE_THUNK_DATA)(((char*)pBase)+(UINT_PTR)pImportDesc->FirstThunk);
			PIMAGE_THUNK_DATA pINT=(PIMAGE_THUNK_DATA)(((char*)pBase)+(UINT_PTR)pImportDesc->OriginalFirstThunk);
			while(pIAT->u1.Function) 
			{
				if (!IMAGE_SNAP_BY_ORDINAL(pINT->u1.Ordinal))
				{
					PIMAGE_IMPORT_BY_NAME pImportName=(PIMAGE_IMPORT_BY_NAME)(((char*)pBase)+(UINT_PTR)(pINT->u1.AddressOfData));
					if (0==lstrcmpiA((char*)pImportName->Name,xfunc))return TRUE;
					
				}
				++pIAT;
				++pINT;
			}
		}
		++pImportDesc;
	}
#else
	return _NSIS_PtrIsUnicodeFullPath(nsis,N_v_pluginsdir) || _NSIS_PtrIsUnicodeFullPath(nsis,N_v_tmp);
#endif
	return FALSE;
}

void StrRawInplaceWToA(char*s) 
{
	WCHAR*w=(WCHAR*)s;
	for(;;) 
	{
		*s=(char)*w;
		if (!*w)break;
		++w;
		++s;
	}
}

UINT StrAToUINT(LPSTR s) 
{
	UINT val=0;
	for(;*s;++s) 
	{
		if (!(*s>='0'||*s<='9'))return -1;
		val*=10;
		val+=*s-'0';
	}
	return val;
}


typedef struct 
{
	extra_parameters*pXP;
	UINT Addr;
} CALLANDWAITTHREADDATA;

DWORD CALLBACK CallAndWaitThreadProc(LPVOID param) 
{
	CALLANDWAITTHREADDATA*pCAWTD=(CALLANDWAITTHREADDATA*)param;
	return pCAWTD->pXP->ExecuteCodeSegment(pCAWTD->Addr,NULL);
}


extern "C" void __declspec(dllexport) __cdecl CallAndWait(HWND hwndNSIS,int N_CCH,char*N_Vars,LPVOID ppStackTop,extra_parameters*pXP)
{
	NSIS nsis={hwndNSIS,N_CCH,N_Vars};
	bool unicode=NSIS_IsUnicode(nsis);

#ifdef DEBUG
	PostMessage(FindWindowA("dbgviewClass",0),WM_COMMAND,40020,0);SetForegroundWindow(FindWindowA("dbgviewClass",0));
	OutputDebugStringA(unicode?"U\n":"A\n");//MessageBoxA(hwndNSIS,buf,0,MB_TOPMOST);
#endif
	
	char*r0A;
	if (unicode) 
	{
		r0A=(char*)N_VarPtrW(N_r0,N_Vars,N_CCH);
		StrRawInplaceWToA(r0A);
	}
	else
		r0A=N_VarPtrA(N_r0,N_Vars,N_CCH);

	CALLANDWAITTHREADDATA cawtd={pXP,StrAToUINT(r0A)-1};
	DWORD tid;
	HANDLE hThread=CreateThread(0,0,CallAndWaitThreadProc,&cawtd,0,&tid);
	if (hThread)
	{
		SetThreadPriority(hThread,THREAD_PRIORITY_IDLE);
		bool loopMore=true;
		while(loopMore) 
		{
			DWORD wait=MsgWaitForMultipleObjects(1,&hThread,false,INFINITE,QS_ALLINPUT|QS_ALLPOSTMESSAGE);
			if (WAIT_OBJECT_0+1==wait) 
			{
				MSG msg;
				while ( loopMore && PeekMessage(&msg,NULL,0,0,PM_REMOVE) ) 
				{
					if (msg.message == WM_QUIT) 
					{
						PostMessage(msg.hwnd,msg.message,msg.wParam,msg.lParam);
						loopMore=false;
					}
					else 
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}
			}
			else 
				loopMore=false;
		}
		CloseHandle(hThread);
	}
	else 
	{
		pXP->ExecuteCodeSegment(cawtd.Addr,NULL);
	}
}

extern "C" BOOL WINAPI _DllMainCRTStartup(HANDLE hInst,UINT Reason,LPVOID lpReserved)
{
	return TRUE;
}
