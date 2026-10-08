#include <windows.h>
#include <stdio.h>
#include "zlib\zlib.h"

typedef struct _stack_t {
  struct _stack_t *next;
  char text[1]; // this should be the length of string_size
} stack_t;

int popstring(char *str); // 0 on success, 1 on empty stack
void pushstring(char *str);
int LogMessage(const char *pStr);
int SetStatus(const char *pStr);


enum
{
INST_0,         // $0
INST_1,         // $1
INST_2,         // $2
INST_3,         // $3
INST_4,         // $4
INST_5,         // $5
INST_6,         // $6
INST_7,         // $7
INST_8,         // $8
INST_9,         // $9
INST_R0,        // $R0
INST_R1,        // $R1
INST_R2,        // $R2
INST_R3,        // $R3
INST_R4,        // $R4
INST_R5,        // $R5
INST_R6,        // $R6
INST_R7,        // $R7
INST_R8,        // $R8
INST_R9,        // $R9
INST_CMDLINE,   // $CMDLINE
INST_INSTDIR,   // $INSTDIR
INST_OUTDIR,    // $OUTDIR
INST_EXEDIR,    // $EXEDIR
__INST_LAST
};

char *getuservariable(int varnum);


HINSTANCE g_hInstance;
HWND g_hwndParent, g_hwndDlg, g_hwndList;
int g_stringsize;
stack_t **g_stacktop;
char *g_variables;

#include <commctrl.h>

void __declspec(dllexport) extract(HWND hwndParent, int string_size, 
                                      char *variables, stack_t **stacktop)
{
	g_hwndParent=hwndParent;
	g_stringsize=string_size;
	g_stacktop=stacktop;
	g_variables=variables;
	g_hwndDlg = g_hwndList = 0;
	
	// do your stuff here
	/* {
	char buf[1024];
	wsprintf(buf,"$0=%s\n",getuservariable(INST_0));
	g_hwndDlg=FindWindowEx(g_hwndParent,NULL,"#32770",NULL);
	if (g_hwndDlg)
	{
	  g_hwndList=FindWindowEx(g_hwndDlg,NULL,"SysListView32",NULL);
	}
	}*/
	
	//Extract file to destination
	{
		char destination[MAX_PATH+1];
		char source[MAX_PATH+1];
		HANDLE hDest, hSrc;
		
		popstring(destination);
		popstring(source);
	
		hDest=CreateFile(destination, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
		if (hDest==INVALID_HANDLE_VALUE)
		{
			pushstring("Could not open destination file");
			return;
		}

		hSrc=CreateFile(source, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
		if (hSrc==INVALID_HANDLE_VALUE)
		{
			CloseHandle(hDest);
			pushstring("Could not open input file");
			return;
		}

		
		#ifdef NSIS_COMPRESS_USE_ZLIB
		{
			z_stream inflate_stream;
			DWORD rlen, wlen;
			char inbuffer[4096], outbuffer[4096];

			inflateInit(&inflate_stream);
			
			if (!ReadFile(hSrc, inbuffer, 4096, &rlen, 0))
				rlen=0;
			while (rlen>0)
			{
				inflate_stream.next_in = inbuffer;
				inflate_stream.avail_in = rlen;
				for (;;)
				{
					int err;
					int u;
					inflate_stream.next_out = outbuffer;
					inflate_stream.avail_out = 4096;
					
					err=inflate(&inflate_stream);
					
					if (err<0)
					{
						CloseHandle(hDest);
						CloseHandle(hSrc);
						pushstring("Inflate failed, maybe file is corrupt");
						return;
					}
					
					u=(char*)inflate_stream.next_out - outbuffer;
					
					if (!u) break;
					
					if (!WriteFile(hDest, outbuffer, u, &wlen, 0))
						wlen=0;
					if (wlen!=u)
					{
						CloseHandle(hDest);
						CloseHandle(hSrc);
						pushstring("Could not write to destination file");
						return;
					}
					
					if (err==Z_STREAM_END)
					{
						CloseHandle(hDest);
						CloseHandle(hSrc);
						pushstring("success");
					}
				}
				if (!ReadFile(hSrc, inbuffer, 4096, &rlen, 0))
					rlen=0;
			}
		}
		#endif //NSIS_COMPRESS_USE_ZLIB
		CloseHandle(hDest);
		CloseHandle(hSrc);
		pushstring("success");
	}
}
/*
int SetStatus(const char *pStr)
{
	HWND hwndCtrl=GetDlgItem(g_hwndDlg, 1006);
	if (hwndCtrl)
	{
		SetWindowText(hwndCtrl, pStr);
	}
	LogMessage(pStr);
	return 0;
}

int LogMessage(const char *pStr)
{
	if (!g_hwndList)
		return -1;

	{
		LVITEM item={0};
		int nItemCount=SendMessage(g_hwndList, LVM_GETITEMCOUNT, 0, 0);
		item.mask=LVIF_TEXT;
		item.pszText=pStr;
		item.cchTextMax=6;
		item.iItem=nItemCount;
		return SendMessage(g_hwndList, LVM_INSERTITEM, 0, (LPARAM)&item);
	}
}
*/
BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
  g_hInstance=hInst;
	return TRUE;
}


// utility functions (not required but often useful)
int popstring(char *str)
{
  stack_t *th;
  if (!g_stacktop || !*g_stacktop) return 1;
  th=(*g_stacktop);
  lstrcpy(str,th->text);
  *g_stacktop = th->next;
  GlobalFree((HGLOBAL)th);
  return 0;
}

void pushstring(char *str)
{
  stack_t *th;
  if (!g_stacktop) return;
  th=(stack_t*)GlobalAlloc(GPTR,sizeof(stack_t)+g_stringsize);
  lstrcpyn(th->text,str,g_stringsize);
  th->next=*g_stacktop;
  *g_stacktop=th;
}

char *getuservariable(int varnum)
{
  if (varnum < 0 || varnum >= __INST_LAST) return NULL;
  return g_variables+varnum*g_stringsize;
}


