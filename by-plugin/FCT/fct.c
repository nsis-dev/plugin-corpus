/***************************************************
* FILE NAME: fct.c
*
* PURPOSE:
*    NSIS plug-in Find_and_Close_or_Terminate.
*
* CHANGE HISTORY
*
* $LOG$
*
* Author              Date          Modifications
* Takhir Bedertdinov  Nov 21 2005   Original
*        Moscow, Russia, ineum@narod.ru
* Takhir Bedertdinov  Jan  7 2005   Partial class 
*        name support
* --------//--------  Jul 24 2006   /SCCLOSE and
*        /MSGONLY options
**************************************************/

#include <windows.h>
#include <fcntl.h>
#include <stdio.h>
#include <io.h>
#include <sys\stat.h>
#include "..\ExDll\exdll.h"

#define WT "/WT"
#define WTP "/WTP"
#define WC "/WC"
#define WCP "/WCP"
#define T_O "/TIMEOUT"
#define UDATA "/UDATA"
#define QUESTION "/QUESTION"
#define ASYNC "/ASYNC"
#define MSGONLY "/MSGONLY"
#define SCCLOSE "/SCCLOSE"
#define DEF_TO 1000


HWND parwnd = NULL;

typedef struct _threadParams {
   char *wnd_caption;
   char *wnd_class;
   char *question;
   DWORD dwTimeout;
   DWORD dwUdata;
   int dwAlive;
   BOOL fTitlePart;
   BOOL fClassPart;
   BOOL fMsgOnly;
   BOOL fScClose;
} threadParams, *pthreadParams;


char *my_strstr(char *i, char *s) {
  if (lstrlen(i)>=lstrlen(s)) while (i[lstrlen(s)-1])  {
    int l=lstrlen(s)+1;
    char *ii=i;
    char *is=s;
    while (--l>0) {
      if (*ii != *is) break;
      ii++;
      is++;
    }
    if (l==0) return i;
    i++;
  }
  return NULL;
}

DWORD my_atoui(char *s)
{
  unsigned int v=0;
  char m=10;
  char t='9';

  if (*s == '0')
  {
    s++; // skip over 0
    if (s[0] >= '0' && s[0] <= '7')
    {
      m=8; // octal
      t='7';
    }
    if ((s[0] & ~0x20) == 'X')
    {
      m=16; // hex
      s++; // advance over 'x'
    }
  }

  for (;;)
  {
    int c=*s++;
    if (c >= '0' && c <= t) c-='0';
    else if (m==16 && (c & ~0x20) >= 'A' && (c & ~0x20) <= 'F') c = (c & 7) + 9;
    else break;
    v*=m;
    v+=c;
  }
  return v;
}


/*****************************************************
 * FUNCTION NAME: winCheck() callback
 * PURPOSE: 
 *    checks enumerated windows class/caption/user_data/process
 * SPECIAL CONSIDERATIONS:
 *    
 *****************************************************/
BOOL CALLBACK winCheck(HWND hWnd, LPARAM lParam)
{
   DWORD rslt = 0;
   HANDLE hProcess = NULL;
   DWORD iProcess;
   char *s = GlobalAlloc(GPTR, g_stringsize); 
   pthreadParams ptp = (pthreadParams)lParam;
   BOOL code = TRUE;

// this is installer' window
   if(hWnd == parwnd)
      return TRUE;
   GetClassName(hWnd, s, g_stringsize);
   if(*(ptp->wnd_class) != 0 &&
      ((ptp->fClassPart && my_strstr(s, ptp->wnd_class) == NULL) ||
      (!(ptp->fClassPart) && lstrcmp(s, ptp->wnd_class) != 0)))
      return TRUE;
   GetWindowText(hWnd, s, g_stringsize);
   if(*(ptp->wnd_caption) != 0 &&
      ((ptp->fTitlePart && my_strstr(s, ptp->wnd_caption) == NULL) ||
      (!(ptp->fTitlePart) && lstrcmp(s, ptp->wnd_caption) != 0)))
      return TRUE;
   if(ptp->dwUdata != 0 &&
      (unsigned)GetWindowLong(hWnd, GWL_USERDATA) != ptp->dwUdata)
      return TRUE;
   GetWindowThreadProcessId(hWnd, &iProcess);
   if((hProcess = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, iProcess)) == NULL)
   {
// we have not terminate permissions. so what we can SYNC exit on message only
      ptp->fMsgOnly = TRUE;
      hProcess = OpenProcess(SYNCHRONIZE, FALSE, iProcess);
   }
   if(ptp->fScClose)
      PostMessage(hWnd, WM_SYSCOMMAND, 0xF060, 0);
//      SendMessageTimeout(hWnd, WM_SYSCOMMAND, 0xF060, 0, SMTO_NORMAL, ptp->dwTimeout, &rslt);
   else
      PostMessage(hWnd, WM_CLOSE, 0, 0);
//      SendMessageTimeout(hWnd, WM_CLOSE, 0, 0, SMTO_NORMAL, ptp->dwTimeout, &rslt);
   if(hProcess)
   {
      if(WaitForSingleObject(hProcess, ptp->dwTimeout) != WAIT_OBJECT_0 &&
         !ptp->fMsgOnly)
      {
         rslt = *(ptp->question) ? MessageBox(parwnd, ptp->question, "Setup", MB_YESNOCANCEL|MB_ICONWARNING) : IDYES;
         switch(rslt)
         {
            case IDYES:
               if(TerminateProcess(hProcess, 0xFFFFFFFF) == 0)
                  ptp->dwAlive++;
               break;
            case IDCANCEL:// cancelled by user, enum loop abort flag
               code = FALSE;
               ptp->dwAlive = -1;
               break;
            case IDNO:// might be closed by user, so we can move forward
               if(WaitForSingleObject(hProcess, 0) != WAIT_OBJECT_0)
                  ptp->dwAlive++;
               break;
         }
      }
      CloseHandle(hProcess);
   }
   else
   {
// could not open process ?
      Sleep(ptp->dwTimeout);
      if(IsWindow(hWnd)) ptp->dwAlive++;
   }

	return code;
}


/*****************************************************
 * FUNCTION NAME: closeApp()
 * PURPOSE: 
 *    this thread calls EnumWindows
 * SPECIAL CONSIDERATIONS:
 *    No NSIS specifics, I hope
 *****************************************************/
int __stdcall closeApp(void *pp)
{
   int rslt;
   pthreadParams ptp = (pthreadParams)pp;

   EnumWindows(winCheck, (LPARAM)pp);
   GlobalFree(ptp->question);
   GlobalFree(ptp->wnd_caption);
   GlobalFree(ptp->wnd_class);
   rslt = ptp->dwAlive;
   GlobalFree(ptp);
   return rslt;
}


/*****************************************************
 * FUNCTION NAME: wait()
 * PURPOSE: 
 *    waits for thread exit and closes handle
 * SPECIAL CONSIDERATIONS:
 *    tested with my consApp.exe
 *****************************************************/
void __declspec(dllexport) wait(HWND hwndParent,
                                int string_size,
                                char *variables,
                                stack_t **stacktop,
                                extra_parameters *extra)
{
   int rslt = 0;
   char exitCode[16];
   HANDLE hThread;

   popstring(exitCode);
   hThread = (HANDLE)my_atoui(exitCode);
/* push to stack NOT terminated applications count */
   if(hThread != NULL)
   {
      WaitForSingleObject(hThread, INFINITE);
      GetExitCodeThread(hThread, &rslt);
      CloseHandle(hThread);
      hThread = NULL;
   }
   wsprintf(exitCode,"%d", rslt);
   pushstring(exitCode);
}

/*****************************************************
 * FUNCTION NAME: fct()
 * PURPOSE: 
 *    find_and_close_or_terminate entry point
 * SPECIAL CONSIDERATIONS:
 *    
 *****************************************************/
void __declspec(dllexport) fct(HWND hwndParent,
                                int string_size,
                                char *variables,
                                stack_t **stacktop,
                                extra_parameters *extra)
{
   DWORD dwThreadId;
   HANDLE hThread;
   BOOL fWait = TRUE;
   pthreadParams ptp = GlobalAlloc(GPTR, sizeof(threadParams));
   char *s = GlobalAlloc(GPTR, string_size);

   EXDLL_INIT();
   parwnd = hwndParent;

   ptp->wnd_class = GlobalAlloc(GPTR, string_size);
   ptp->wnd_caption = GlobalAlloc(GPTR, string_size);
   ptp->question = GlobalAlloc(GPTR, string_size);

   while(!popstring(s) && *s == '/' && lstrcmpi(s, "/END") != 0)
   {
      if(lstrcmpi(s, WC) == 0)
      {
         popstring(ptp->wnd_class);
      }
      else if(lstrcmpi(s, WCP) == 0)
      {
         popstring(ptp->wnd_class);
         ptp->fClassPart = TRUE;
      }
      else if(lstrcmpi(s, WT) == 0)
      {
         popstring(ptp->wnd_caption);
      }
      else if(lstrcmpi(s, WTP) == 0)
      {
         popstring(ptp->wnd_caption);
         ptp->fTitlePart = TRUE;
      }
      else if(lstrcmpi(s, T_O) == 0)
      {
         popstring(s);
         ptp->dwTimeout = my_atoui(s);
      }
      else if(lstrcmpi(s, ASYNC) == 0)
      {
         fWait = FALSE;
      }
      else if(lstrcmpi(s, QUESTION) == 0)
      {
         popstring(ptp->question);
         /*if(ptp->question[0] == '\"' && ptp->question[1] == '\"')
            *(ptp->question) = 0;*/
      }
      else if(lstrcmpi(s, UDATA) == 0)
      {
         popstring(s);
         ptp->dwUdata = my_atoui(s);
      }
      else if(lstrcmpi(s, MSGONLY) == 0)
      {
         ptp->fMsgOnly = TRUE;
      }
      else if(lstrcmpi(s, SCCLOSE) == 0)
      {
         ptp->fScClose = TRUE;
      }
      else
      {
         pushstring(s);
         break;
      }
      *s = 0;
   }

   if(ptp->wnd_class[0] == 0 && ptp->wnd_caption[0] == 0)
   {
// for developers only :-)
      popstring("error");
      return;
   }
   if(ptp->dwTimeout == 0)
      ptp->dwTimeout = DEF_TO;

   if(extra->exec_flags->silent != 0)
   {
// no questions if silent
      *(ptp->question) = 0;
   }

   hThread = CreateThread(NULL, 0, closeApp, (void*)ptp, 0, &dwThreadId);
   wsprintf(s, "%u", hThread);
   pushstring(s);
   GlobalFree(s);
   if(fWait) wait(hwndParent, string_size, variables, stacktop, extra);
}

/*****************************************************
 * FUNCTION NAME: DllMain()
 * PURPOSE: 
 *    Dll main entry point
 * SPECIAL CONSIDERATIONS:
 *    
 *****************************************************/
BOOL WINAPI DllMain(HANDLE hInst,
						  ULONG ul_reason_for_call,
						  LPVOID lpReserved)
{
	return TRUE;
}
