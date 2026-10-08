#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <ctype.h>

#include "../exdll.h"
#include "../zlib/unzip.h"
#include "resource.h"

#define DLG_PERCENT 0
#define DLG_FILES   1
#define DLG_PROGBAR 2

BOOL CALLBACK zipDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

char tempzip_path[1024];
char tempzip_file[MAX_PATH];

HINSTANCE g_hInstance;
HWND g_hwndParent;
int dlg = DLG_PERCENT;

static void doMKDir(char *directory)
{
  char *p, *p2;
  char buf[MAX_PATH];
  if (!*directory) return;
  lstrcpy(buf,directory);
  p=buf; while (*p) p++;
  while (p >= buf && *p != '\\') p--;
  p2 = buf;
  if (p2[1] == ':') p2+=4;
  else if (p2[0] == '\\' && p2[1] == '\\')
  {
    p2+=2;
    while (*p2 && *p2 != '\\') p2++;
    if (*p2) p2++;
    while (*p2 && *p2 != '\\') p2++;
    if (*p2) p2++;
  }
  if (p >= p2)
  {
    *p=0;
    doMKDir(buf);
  }
  CreateDirectory(directory,NULL);
}


BOOL tempzip_make(HWND hwndDlg, char *fn, char *pn) {

  char buf[MAX_PATH];
  unz_global_info gi;
  int tf=0;
  int nf=0;
  int quit=0;

  unzFile f;
  f = unzOpen(fn);

  if (!f || unzGoToFirstFile(f) != UNZ_OK) {
    if (f) unzClose(f);
    // Error opening ZIP file
    pushstring("2");
    return 1;
  }
  
  if (unzGetGlobalInfo(f,&gi) != UNZ_OK) {
    pushstring("2");
    return 1;
  }

  if (dlg==DLG_PERCENT) {
    tf = gi.number_entry;
    SetDlgItemText(hwndDlg,IDC_ZIPINFO,"Extracting: 0 %");
  }  
  else if (dlg==DLG_PROGBAR) SendMessage(GetDlgItem(hwndDlg, IDC_ZIPPROG),PBM_SETRANGE,0,MAKELPARAM(0, gi.number_entry));
  else if (dlg==DLG_FILES) SetDlgItemText(hwndDlg,IDC_ZIPINFO,"Extracting: 0 files");

  do {
    char filename[MAX_PATH];
    unz_file_info info;

    unzGetCurrentFileInfo(f,&info,filename,sizeof(filename),NULL,0,NULL,0);

    // was zip created on MS-DOS/Windows?
    if ((info.version & 0xFF00) == 0) {
      OemToCharBuff(filename, filename, strlen(filename));
    }

    if (filename[0] &&
        filename[strlen(filename)-1] != '\\' &&
        filename[strlen(filename)-1] != '/')
    {
      char *pfn=filename;
      char out_filename[1024];
      while (*pfn)
      {
        if (*pfn == '/') *pfn='\\';
        pfn++;
      }
      pfn=filename;
      if (pfn[1] == ':' && pfn[2] == '\\') pfn+=3;
      while (*pfn == '\\') pfn++;


      lstrcpy(out_filename,pn);
      lstrcat(out_filename,"\\");
      lstrcat(out_filename,pfn);
      if (strstr(pfn,"\\"))
      {
        char buf[1024];
        char *p;
        lstrcpy(buf,out_filename);
        p=buf+strlen(buf);
        while (p > buf && *p != '\\') p--;
        *p=0;
        if (buf[0]) doMKDir(buf);
      }

      if (unzOpenCurrentFile(f) == UNZ_OK)
      {
        MSG msg;
        FILE *fp;
        int l;
        //wsprintf(buf,"Extracting: %s file",pfn);
        //SetWindowText(hwndDlg, buf);
        // SendDlgItemMessage(hwndDlg,IDC_ZIPINFO_FILES,LB_ADDSTRING,0,(LPARAM)pfn);
        fp = fopen(out_filename,"wb");
        if (fp)
        {
          do
          {
            char buf[1024];
            l=unzReadCurrentFile(f,buf,sizeof(buf));
            if (l > 0)
            {
              if (fwrite(buf,1,l,fp) != (unsigned int)l)
              {
                unzClose(f);
                fclose(fp);
                // Error writing output file(s)
                pushstring("3");
                return 1;
              }
            }
          } while (l > 0);

          fclose(fp);

          {
            // set file time
            HANDLE hf = CreateFile(out_filename, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);
            if (hf != INVALID_HANDLE_VALUE)
            {
              FILETIME ft, lft;
              DosDateTimeToFileTime(HIWORD(info.dosDate), LOWORD(info.dosDate), &ft);
              LocalFileTimeToFileTime(&ft, &lft);
              SetFileTime(hf, 0, 0, &lft);
              CloseHandle(hf);
            }
          }
        }
        else
        {
          unzClose(f);
          // Error opening output file(s)
          pushstring("3");
          return 1;
        }
        //MSG msg;

        if (dlg==DLG_FILES) {
          nf++;
          wsprintf(buf,"Extracting: %d files",nf);
          SetDlgItemText(hwndDlg,IDC_ZIPINFO,buf);
        }

        //int quit=0;
        while (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
        {
          if (msg.message == WM_DESTROY && msg.hwnd == hwndDlg) // || msg.message == WM_RBUTTONDOWN
          {
            quit++;
            break;
          }
          if ((msg.message == WM_COMMAND && LOWORD(msg.wParam) == IDC_CANCELZIP) && msg.hwnd == hwndDlg)
          {
            quit++;
            break;
          }

          TranslateMessage(&msg);
          DispatchMessage(&msg);
        }
        
        unzCloseCurrentFile(f);
        if (quit) break;
      }
      else
      {
        unzClose(f);
        // Error extracting from ZIP file
        pushstring("4");
        return 1;
      }
    }
		
    if (dlg==DLG_PROGBAR) {
      nf++;
      SendMessage(GetDlgItem(hwndDlg, IDC_ZIPPROG),PBM_SETPOS,nf,0);
    } else if (dlg==DLG_PERCENT) {
      nf++;
      wsprintf(buf,"Extracting: %d %%",(int)(((double)100/(double)tf)*(double)nf));
      SetDlgItemText(hwndDlg,IDC_ZIPINFO,buf);
    }

  } while (unzGoToNextFile(f) == UNZ_OK);

  if (quit)
  {
    unzClose(f);
    // cancel
    pushstring("1");
    return 1;
  }

  if (dlg!=DLG_PROGBAR) {
    if (dlg==DLG_PERCENT) {
      wsprintf(buf,"Extracted: %d %%",(int)(((double)100/(double)tf)*(double)nf));
	} else if (dlg==DLG_FILES) {
      wsprintf(buf,"Extracted: %d files",nf);
	}
    SetDlgItemText(hwndDlg,IDC_ZIPINFO,buf);
  }

  unzClose(f);
  Sleep(1000);
  pushstring("0");
  return 1;
}

BOOL CALLBACK zipDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  static HMENU hMenu;
  static HMENU hPopup;

  if (uMsg == WM_DESTROY)
  {
    if (hPopup) DestroyMenu(hPopup);
    hPopup=0;
    if (hMenu) DestroyMenu(hMenu);
    hMenu=0;
  }

  switch (uMsg) {
    case WM_INITDIALOG:
    {
      SetTimer(hwndDlg,001,100,(TIMERPROC)NULL);
      hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDD_POPUP));
      hPopup = GetSubMenu(hMenu, 0);
    }
    break;

    case WM_LBUTTONDOWN:
    {
      POINT pt;
      GetCursorPos(&pt);
      PostMessage(hwndDlg, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(pt.x, pt.y));
    }
    break;

    case WM_RBUTTONDOWN:
    {
      POINT pt;
      GetCursorPos(&pt);
      TrackPopupMenu(hPopup, 0, pt.x, pt.y, 0, hwndDlg, NULL);
    }
    break;

    case WM_TIMER:
    {
      if (KillTimer(hwndDlg, 001)) {
        doMKDir(tempzip_path);
        if (tempzip_make(hwndDlg, tempzip_file, tempzip_path))
        {
          EndDialog(hwndDlg, 0);
        }
      }
    }
    break;
  }
  return 0;
}

void __declspec(dllexport) nzip(HWND hwndParent, int string_size, char *variables, stack_t **stacktop)
{
  g_hwndParent=hwndParent;
  InitCommonControls();

  EXDLL_INIT();

  {
    char szParam[1024];	
    char* dialogs[] = {
      MAKEINTRESOURCE(IDD_DLGZIP1),
      MAKEINTRESOURCE(IDD_DLGZIP1),
      MAKEINTRESOURCE(IDD_DLGZIP2)
    };
	
    if (popstring(szParam)) return;
    if (popstring(tempzip_file)) return;
    if (popstring(tempzip_path)) return;
	
    if (strlen(tempzip_file)>0 && strlen(tempzip_path)>0 && strlen(szParam)>0)
    {
      if (lstrcmpi(szParam, "/percent") == 0) dlg = DLG_PERCENT;
      else if (lstrcmpi(szParam, "/files") == 0) dlg = DLG_FILES;
      else if (lstrcmpi(szParam, "/progbar") == 0) dlg = DLG_PROGBAR;
      else {
        pushstring("error");
        return;
      }
      DialogBox(g_hInstance, dialogs[dlg], g_hwndParent, zipDlgProc);
    }
    else
    {
      pushstring("error");
    }
  }
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
  g_hInstance=hInst;
  return TRUE;
}


