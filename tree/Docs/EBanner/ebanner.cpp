/***************************************************
* FILE NAME: ebanner.cpp
*
* Copyright 2005 - Present NSIS
*
* PURPOSE:
*    Embedded Banner plug-in
*    Displays image on NSIS page window
*
* CHANGE HISTORY
*
* Author              Date           Modifications
*
* Takhir Bedertdinov  July 13, 2005  Original
* Takhir Bedertdinov  July 14, 2005  IImgCtx version
*                 (all IE can show image formats)
* Takhir Bedertdinov  November 13, 2005  Archive
*     version restore, DllDetach improved
* Takhir Bedertdinov  January   6, 2006  Vert. scaling
* Takhir Bedertdinov  January  15, 2006  /fit and /stretch
* Takhir Bedertdinov  February 18, 2006  /halign, /valign,
*     /fit, /hwnd,  crash on Welcome Cancel, WM_PAINT
* Takhir Bedertdinov  February 22, 2006  WinProc clean
*     on 'show' without 'stop' like in AnimGif plug-in
*         June 21, 2006  Sound break on 'show' removed
**************************************************/



#include <windows.h>
#include <windowsx.h>
#include <olectl.h>
#include <IImgCtx.h>
#include <stdio.h>
#include <vfw.h>
#include "..\exdll\exdll.h"


HWND mciWnd = NULL, childwnd = NULL;
IImgCtx *g_pImage = NULL;
long w, h;
RECT r;
static void *lpWndProcOld = NULL;
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

enum HALIGNS {
   HALIGN_CENTER = 0,
   HALIGN_LEFT,
   HALIGN_RIGHT
};
enum VALIGNS {
   VALIGN_CENTER = 0,
   VALIGN_TOP,
   VALIGN_BOTTOM
};
enum FITS {
   FIT_NONE = 0,
   FIT_WIDTH,
   FIT_HEIGHT,
   FIT_BOTH
};

char *my_strchr(char *s, char c)
{
   while(*s != 0)
   {
      if(*s == c)
         return s;
      s++;
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
 * FUNCTION NAME: paintBanner()
 * PURPOSE: 
 *    paints banner on the dialog window
 * SPECIAL CONSIDERATIONS:
 *    redrawwindow not works in 'show'
 *****************************************************/
void paintBanner(void)
{
	if(g_pImage != NULL)
   {
      HDC hdc = GetDC(childwnd);
      g_pImage->Draw(hdc, &r);
      ReleaseDC(childwnd, hdc);
      ValidateRect(childwnd, &r);
   }
}
void clearImage(void)
{
   if(lpWndProcOld != NULL &&
      IsWindow(childwnd) &&
      (void *)GetWindowLong(childwnd, GWL_WNDPROC) == WndProc)
      SetWindowLong(childwnd, GWL_WNDPROC,(long)lpWndProcOld);
   lpWndProcOld = NULL;
   if(g_pImage != NULL)
   {
      g_pImage->Release();
      g_pImage = NULL;
      if(IsWindow(childwnd))
      {
         RedrawWindow(childwnd, NULL, NULL, RDW_INVALIDATE|RDW_ERASE);
         UpdateWindow(childwnd);
      }
   }
   childwnd = NULL;
}

void stopSound(void)
{
   if(mciWnd != NULL && IsWindow(mciWnd))
      MCIWndDestroy(mciWnd);
   mciWnd = NULL;
}


/*****************************************************
 * FUNCTION NAME: stop()
 * PURPOSE: 
 *    image banner "stop" dll entry point
 * SPECIAL CONSIDERATIONS:
 *    releases image and stops sound (if any)
 *****************************************************/
extern "C"
void __declspec(dllexport) stop(HWND hwndParent,
                                int string_size,
                                char *variables,
                                stack_t **stacktop)
{
   clearImage();
   stopSound();
}


/*****************************************************
 * FUNCTION NAME: play()
 * PURPOSE: 
 *    PlaySound dll entry point
 * SPECIAL CONSIDERATIONS:
 *    makes PlaySound working on Win98
 *    works with /NOUNLOAD parameter only
 *    wav files are huge :-(
 *    but PlaySound not flushes on the screen like this 
 *    mci window does
 *****************************************************/
extern "C"
void __declspec(dllexport) play(HWND hwndParent,
                                int string_size,
                                char *variables,
                                stack_t **stacktop)
{
   char fn[MAX_PATH];
   char cmd[32] = "play";
   EXDLL_INIT();

   if(popstring(fn) == 0)
   {
      if(lstrcmpi(fn, "/loop") == 0)
      {
         lstrcat(cmd, " repeat");
         *fn = 0;
         popstring(fn);
      }
      if(*fn == 0) stopSound();
      else
      {
         if((mciWnd = MCIWndCreate(hwndParent, 0, MCIWNDF_NOPLAYBAR, fn)) != NULL)
         {
            ShowWindow(mciWnd, SW_HIDE);
            if(MCIWndCanPlay(mciWnd))
               MCIWndSendString(mciWnd, cmd);
         }
      }
   }
}


/*****************************************************
 * FUNCTION NAME: show()
 * PURPOSE: 
 *    image banner "show" dll entry point
 * SPECIAL CONSIDERATIONS:
 *    
 *****************************************************/
extern "C"
void __declspec(dllexport) show(HWND hwndParent,
                                int string_size,
                                char *variables,
                                stack_t **stacktop)
{
   char fn[MAX_PATH] = "";
   unsigned short wcPort[MAX_PATH];
   int iHAlign = HALIGN_CENTER,
      iVAlign = VALIGN_BOTTOM,
      iFit = FIT_NONE;
   int w, h;
   SIZE imSize = {0, 0};
   DWORD dwState;
   char *pvalue;

   EXDLL_INIT();

   clearImage();
//   stop(NULL, 0, NULL, NULL);
   while(!popstring(fn) && *fn == '/' && (pvalue = my_strchr(fn, '=')) != NULL)
   {
      *pvalue++ = 0;
      if(lstrcmpi(fn, "/halign") == 0)
      {
         if(lstrcmpi(pvalue, "left") == 0) iHAlign = HALIGN_LEFT;
         else if(lstrcmpi(pvalue, "right") == 0) iHAlign = HALIGN_RIGHT;
      }
      if(lstrcmpi(fn, "/valign") == 0)
      {
         if(lstrcmpi(pvalue, "top") == 0) iVAlign = VALIGN_TOP;
         else if(lstrcmpi(pvalue, "center") == 0) iVAlign = VALIGN_CENTER;
      }
      else if(lstrcmpi(fn, "/hwnd") == 0)
         childwnd = (HWND) my_atoui(pvalue);
      else if(lstrcmpi(fn, "/fit") == 0)
      {
         if(lstrcmpi(pvalue, "height") == 0) iFit = FIT_HEIGHT;
         else if(lstrcmpi(pvalue, "width") == 0) iFit = FIT_WIDTH;
         else if(lstrcmpi(pvalue, "both") == 0) iFit = FIT_BOTH;
      }
   }

// nothing to do if file not defined
   if(*fn == 0) return;
// if target window not defined we'll search for default
   if(childwnd == NULL)
   {
      if((childwnd = FindWindowEx(hwndParent, NULL, "#32770", NULL)) == NULL)
         return;
      if((hwndParent = FindWindowEx(hwndParent, childwnd, "#32770", NULL)) != NULL &&
         !IsWindowVisible(hwndParent))
         childwnd = hwndParent;
      if(childwnd == NULL)
         return;
   }

// load new image
   MultiByteToWideChar(CP_ACP,0,fn,-1, wcPort,sizeof(wcPort)/2);
   if(SUCCEEDED(CoCreateInstance(CLSID_IImgCtx, NULL, CLSCTX_ALL, IID_IImgCtx, (void**)&g_pImage)))
   {
      if(SUCCEEDED(g_pImage->Load(wcPort, 0)))
      {
         while(SUCCEEDED(g_pImage->GetStateInfo(&dwState, NULL, true)) &&
            (IMGLOAD_COMPLETE & dwState) == 0 &&
            (IMGLOAD_ERROR & dwState) == 0)
         {
            Sleep(20);
         }
         g_pImage->GetStateInfo(&dwState, &imSize, true);
      }
      if(imSize.cx == 0 || imSize.cy == 0) // error path or format (IMGLOAD_ERROR)
      {
         g_pImage->Release();
         g_pImage = NULL;
      }
   }
   if(g_pImage == NULL) return;

// fit image
   GetClientRect(childwnd, &r);
   if(iFit == FIT_HEIGHT)
   {
      h = r.bottom - r.top;
      w = (imSize.cx * (r.bottom - r.top)) / imSize.cy;
   }
   else if(iFit == FIT_WIDTH)
   {
      w = r.right - r.left;
      h = (imSize.cy * (r.right - r.left)) / imSize.cx;
   }
   else if(iFit == FIT_BOTH)
   {
      w = r.right - r.left;
      h = r.bottom - r.top;
   }
   else
   {
      w = imSize.cx;
      h = imSize.cy;
   }

// align image
   if(iHAlign == HALIGN_CENTER) r.left = (r.left + r.right - w) / 2;
   else if(iHAlign == HALIGN_RIGHT) r.left = r.right - w;
   if(iVAlign == VALIGN_CENTER) r.bottom = (r.bottom + r.top + h) / 2;
   else if(iVAlign == VALIGN_TOP) r.bottom = r.top + h;
   r.right = r.left + w;
   r.top = r.bottom - h;

// attach to parent window message queue for paint and close events
   lpWndProcOld = (void *)SetWindowLong(childwnd, GWL_WNDPROC, (long)WndProc);
   paintBanner();
}




/*****************************************************
 * FUNCTION NAME: DllMain()
 * PURPOSE: 
 *    Dll main (initialization) entry point
 * SPECIAL CONSIDERATIONS:
 *    
 *****************************************************/
BOOL WINAPI DllMain(HANDLE hInst,
                    ULONG ul_reason_for_call,
                    LPVOID lpReserved)
{
   if(ul_reason_for_call == DLL_PROCESS_DETACH)
      stop(NULL, 0, NULL, NULL);
   return TRUE;
}

/*****************************************************
 * FUNCTION NAME: WndProc()
 * PURPOSE: 
 *    window proc. 
 * SPECIAL CONSIDERATIONS:
 *    for paint and close purposes
 *****************************************************/
static LRESULT CALLBACK WndProc(HWND hwnd,
                                UINT uMsg,
                                WPARAM wParam,
                                LPARAM lParam)
{
   switch (uMsg)
   {
   case WM_PAINT:
      paintBanner();
      break;

   case WM_CLOSE:
         stop(hwnd, 0, NULL, NULL);
      break;
   case WM_COMMAND:
      if(LOWORD(wParam) == IDCANCEL ||
         LOWORD(wParam) == IDOK ||
         LOWORD(wParam) == IDABORT)
         stop(hwnd, 0, NULL, NULL);
      break;

   default:
      break;
   }
   return CallWindowProc((WNDPROC)lpWndProcOld,
         hwnd, uMsg, wParam, lParam);
}



