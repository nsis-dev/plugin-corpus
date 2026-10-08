/*
History
=======
0.2 - 20100422 AndersK
*Fixed stupid varsync missing code bug

0.1 - 20090829 AndersK
*Initial public version

*/

#if (defined(_MSC_VER) && !defined(_DEBUG))
	#pragma comment(linker,"/opt:nowin98")
	#pragma comment(linker,"/ignore:4078")
	#pragma comment(linker,"/merge:.rdata=.text")
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#if 0
#define TRACE1A(fmt,p1) do{char b[4444];wsprintfA(b,fmt,p1);OutputDebugStringA(b);}while(0)
#else
#define TRACE1A(fmt,p1)
#endif

#define NSISCALL WINAPI
#define NSISGLOBALVARCOUNT (1+30) //#define state_click_next          g_usrvars[30]
#define NSISCONVGLOBALVARCOUNT (20) //don't convert pluginsdir etc back from ansi
typedef struct _stack_tA {struct _stack_tA *next;char text[1];} stack_tA;
typedef struct _stack_tW {struct _stack_tW *next;WCHAR text[1];} stack_tW;

typedef struct {
  int autoclose;
  int all_user_var;
  int exec_error;
  int abort;
  int exec_reboot;
  int reboot_called;
  int XXX_cur_insttype;
  int plugin_api_version;
  int silent;
  int instdir_error;
  int rtl;
  int errlvl;
  int alter_reg_view;
  int status_update;
} exec_flags_type;

typedef struct {
  exec_flags_type*exec_flags;
  void*ExecuteCodeSegment;
  void (NSISCALL *validate_filename)(WCHAR*);
  int (NSISCALL *RegisterPluginCallback)(HMODULE,void*callback);
} extra_parameters;

__forceinline void*NSISCALL MemAlloc(SIZE_T cb) {return GlobalAlloc(LPTR,cb);}
__forceinline void NSISCALL MemFree(void* p) {if (p)GlobalFree(p);}

//HMODULE g_hmodule;
bool g_StayLoaded;
WCHAR*g_bufW;
extra_parameters*g_pXP;



void NSISCALL validate_filenameA(char*ansi) 
{
	UINT cch=(lstrlenA(ansi)+1)*sizeof(WCHAR);//this is a bit ugly
	MultiByteToWideChar(0,0,ansi,-1,g_bufW,cch);
	g_pXP->validate_filename(g_bufW);
}

UINT_PTR __cdecl NSISPluginCallback(/*UINT Event*/) 
{
/*	switch(Event) 
	{
	case NSPIM_UNLOAD:
		break;
	}
*/	return NULL;
}

int NSISCALL RegisterPluginCallbackA(HMODULE hmod,void*callback) 
{
	//if (g_pXP->RegisterPluginCallback(g_hmodule,NSISPluginCallback) >= 0) 
	{
		int ret=g_pXP->RegisterPluginCallback(hmod,callback);//there is nothing string specific about callbacks (for now anyway)
		if (ret >= 0)g_StayLoaded=true;
		return ret;
	}
	//return -1;
}

#define GetVarW(idx,vars,strCCH) ( ((LPWSTR)vars)+((idx)*(strCCH)) )
#define GetVarA(idx,vars,strCCH) ( ((char*)vars)+((idx)*(strCCH)) )

/*LPWSTR*/void GetStackTopW(LPWSTR dest,_stack_tW**ppSTW) 
{
	if (!*ppSTW) 
	{
		*dest=NULL;
		return;
	}
	stack_tW*pStW;
	pStW=*ppSTW;
	lstrcpyW(dest,(*ppSTW)->text);
	*ppSTW=(*ppSTW)->next;
	MemFree(pStW);
//	return dest;
}

UINT StrWToUINT(LPWSTR s) 
{
	UINT val=0;
	for(;*s;++s) 
	{
		if (!(*s>=L'0'||*s<=L'9'))break;//return 0;
		val*=10;
		val+=*s-L'0';
	}
	return val;
}

extern "C" void __declspec(dllexport) __cdecl Call(HWND hwndNSIS,int strCCH,WCHAR*varsW,stack_tW**ppSTW,extra_parameters*pXPW) 
{
	g_StayLoaded=false;
	g_pXP=pXPW;

	bool calledExport=false;
	LPWSTR bufW;
	FARPROC dllExportFunc;
	UINT i,stackcount;
	extra_parameters xpA;
	stack_tA*pSTA=NULL,*pPrevSTA=NULL;
	stack_tW*pPrevSW=NULL;
	HMODULE hDll=NULL;
	char*varsA=(char*)MemAlloc( (sizeof(char)*strCCH*NSISGLOBALVARCOUNT) + (sizeof(WCHAR)*strCCH) );//one extra string for internal use
	if (!varsA)goto fail;

	g_bufW=(LPWSTR) ( varsA+((NSISGLOBALVARCOUNT+1)*strCCH) );

	xpA.exec_flags=pXPW->exec_flags;
	xpA.ExecuteCodeSegment=pXPW->ExecuteCodeSegment;
	*(void**)&xpA.validate_filename=validate_filenameA;
	xpA.RegisterPluginCallback=RegisterPluginCallbackA;

	//save the plugin filename & export and remove "our" stack items
	GetStackTopW(bufW=g_bufW,ppSTW);
	GetStackTopW((LPWSTR)varsA,ppSTW);

	//a plugin path/name starting with * means /NOUNLOAD
	if (*bufW=='*') 
	{
		g_StayLoaded=true;
		++bufW;
	}

	TRACE1A("CallAnsiPlugin Load |%ws|\n",g_bufW);
	hDll=LoadLibraryW(bufW);
	if (!hDll)goto fail;

	WideCharToMultiByte(0,0,(LPWSTR)varsA,-1,(LPSTR)g_bufW,strCCH,0,0);
	if (!(dllExportFunc=GetProcAddress(hDll,(LPSTR)g_bufW)))goto fail;

	TRACE1A("CallAnsiPlugin |%s|\n",g_bufW);
	
	//stack count
	GetStackTopW(g_bufW,ppSTW);
	stackcount=StrWToUINT(g_bufW);

	for(i=stackcount;i;--i) 
	{
		stack_tA*pSA=(stack_tA*)MemAlloc(sizeof(stack_tA)+(strCCH*sizeof(char)));
		if (!pSA)goto fail;
		GetStackTopW(g_bufW,ppSTW);
		WideCharToMultiByte(0,0,g_bufW,-1,pSA->text,strCCH,0,0);

		TRACE1A("stackA:reversePush: |%s|\n",pSA->text);

		//pSA->next=NULL;
		if (!pSTA)
			pSTA=pSA;
		else
			pPrevSTA->next=pSA;
		pPrevSTA=pSA;
	}

	for(i=0;i<NSISGLOBALVARCOUNT;++i) 
	{
		WideCharToMultiByte(0,0,varsW+(i*strCCH),-1,varsA+(i*strCCH),sizeof(char)*strCCH,0,0);
//		char b[4444];wsprintfA(b,"%d: |%s<<<%ws|\n",i,GetVarA(i,varsA,strCCH),GetVarW(i,varsW,strCCH));OutputDebugStringA(b);
	}


	TRACE1A("CallAnsiPlugin calling func %X\n",dllExportFunc);
	((void(__cdecl*)(HWND,int,char*,stack_tA**,extra_parameters*))dllExportFunc)(hwndNSIS,strCCH,varsA,&pSTA,&xpA);
	calledExport=true;

	goto ___ret;
fail:
	pXPW->exec_flags->exec_error=!0;
	MessageBoxA(hwndNSIS,"CallAnsiPlugin error",0,0);
___ret:
	if (!g_StayLoaded)FreeLibrary(hDll);
	
	for(i=0;calledExport && i<NSISCONVGLOBALVARCOUNT;++i) 
	{
		MultiByteToWideChar(0,0,GetVarA(i,varsA,strCCH),-1,GetVarW(i,varsW,strCCH),strCCH-1);
//		char b[4444];wsprintfA(b,"%d: |%s>>>%ws|\n",i,GetVarA(i,varsA,strCCH),GetVarW(i,varsW,strCCH));OutputDebugStringA(b);
	}
	
	
	MemFree(varsA);
	
	for(;pSTA;) 
	{
		//add back to real stack
		if (calledExport)
		{
			stack_tW*pSW=(stack_tW*)MemAlloc(sizeof(stack_tW)+(strCCH*sizeof(WCHAR)));
			if (pSW) 
			{
				MultiByteToWideChar(0,0,pSTA->text,-1,pSW->text,strCCH);
				if (!pPrevSW) 
				{
					pSW->next=*ppSTW;
					*ppSTW=pSW;
				}
				else 
				{
					pSW->next=pPrevSW->next;
					pPrevSW->next=pSW;
				}
				pPrevSW=pSW;
			}
		}
		pPrevSTA=pSTA;
		pSTA=pSTA->next;
		MemFree(pPrevSTA);
	}
}

extern "C" BOOL WINAPI _DllMainCRTStartup(HANDLE hInst,ULONG ul_reason_for_call,LPVOID lpReserved)
{
	//g_hmodule=(HINSTANCE)hInst;
	return TRUE;
}