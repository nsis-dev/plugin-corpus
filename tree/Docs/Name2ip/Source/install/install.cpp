/*****************************************************************
 *                name2ip NSIS plugin setup                      *
 *                                                               *
 * 2006 Shengalts Aleksander aka Instructor (Shengalts@mail.ru)  *
 *****************************************************************/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include "res\resource.h"

#define PROGRAM_NAME        "name2ip"
#define PROGRAM_CAPTION     PROGRAM_NAME " Plugin Setup"
#define PROGRAM_REGROOT     HKEY_LOCAL_MACHINE
#define PROGRAM_REGKEY      "SOFTWARE\\NSIS"
#define PROGRAM_FILE        "\\makensis.exe"
#define PROGRAM_BROWSETEXT  "Choose NSIS directory:"


#ifndef BIF_NONEWFOLDERBUTTON
	#define BIF_NONEWFOLDERBUTTON 0x200
#endif
#ifndef BIF_NEWDIALOGSTYLE
	#define BIF_NEWDIALOGSTYLE 0x0040
#endif

HINSTANCE hInstance;
LPSTR lpCmdLine;
int nCmdShow;
char szExeDir[MAX_PATH];

BOOL CALLBACK SelectPath(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
BOOL CopyFileWithMessages(HWND hwnd, char *pSourcePath, char *pSourceFile, char *pTargetPath, char *pTargetFile);
BOOL FileExists(char *fname);

void _WinMain()
{
  STARTUPINFO lpStartupInfo;
  HWND hDlg;
  MSG msg;
  int i;

  //Get standart WinMain info, because of own entry point.
  hInstance=GetModuleHandle(NULL);

  lpCmdLine=GetCommandLine();
  if (*lpCmdLine++ == '\"')
    while (*lpCmdLine != '\"' && *lpCmdLine != '\0') ++lpCmdLine;
  else
    while (*lpCmdLine != ' ' && *lpCmdLine != '\0') ++lpCmdLine;
  if (*lpCmdLine != '\0')
    while (*++lpCmdLine == ' ');

  lpStartupInfo.cb=sizeof(STARTUPINFO);
  GetStartupInfo(&lpStartupInfo);
  nCmdShow=lpStartupInfo.wShowWindow;

  //Get program directory
  i=GetModuleFileName(NULL, szExeDir, MAX_PATH);
  while (i > 0 && szExeDir[i] != '\\') --i;
  szExeDir[i]='\0';


  if (hDlg=CreateDialog(hInstance, MAKEINTRESOURCE(IDD_PATH), 0, (DLGPROC)SelectPath))
    ShowWindow(hDlg, SW_SHOW);

  while (GetMessage(&msg, 0, 0, 0)) 
  {
    if (!IsDialogMessage(hDlg, &msg))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
  ExitProcess(0);
}

BOOL CALLBACK SelectPath(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  char szBuf[MAX_PATH];
  HKEY hKey;
  DWORD dwType;
  DWORD dwSize;
  static HWND hOK;
  static HWND hPath;

  switch(uMsg)
  {
    case WM_INITDIALOG:
      szBuf[0]='\0';
      hOK=GetDlgItem(hwnd,IDOK);
      hPath=GetDlgItem(hwnd,IDC_PATH);

      if (RegOpenKeyEx(PROGRAM_REGROOT, PROGRAM_REGKEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
      {
        dwSize=MAX_PATH;
        RegQueryValueEx(hKey, "", NULL, &dwType, (unsigned char *)&szBuf, &dwSize);
        RegCloseKey(hKey);
      }

      SetWindowText(hwnd, PROGRAM_CAPTION);
      SendMessage(hwnd,WM_SETICON,(WPARAM)ICON_BIG,(LPARAM)LoadIcon(hInstance,MAKEINTRESOURCE(IDI_ICON)));
      SendMessage(hPath,EM_LIMITTEXT, MAX_PATH, 0);
      SetWindowText(hPath, szBuf);
      return TRUE;

    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
        case IDC_PATH:
        {
          GetWindowText(hPath, szBuf, MAX_PATH);
          lstrcat(szBuf, PROGRAM_FILE);
          EnableWindow(hOK, FileExists(szBuf));
          return TRUE;
        }
        case IDC_BROWSE:
        {
          BROWSEINFO bi={0};
          LPITEMIDLIST pIdList;
          LPMALLOC pMalloc;

          GetWindowText(hPath, szBuf, MAX_PATH);
          bi.hwndOwner=hwnd;
          bi.pszDisplayName=szBuf;
          bi.lpszTitle=PROGRAM_BROWSETEXT;
          bi.ulFlags=BIF_RETURNONLYFSDIRS|BIF_NONEWFOLDERBUTTON|BIF_NEWDIALOGSTYLE;
          bi.lpfn=BrowseCallbackProc;
          bi.lParam=(LPARAM)szBuf;

          if (pIdList=SHBrowseForFolder(&bi))
          {
            SHGetPathFromIDList(pIdList, szBuf);

            if (SHGetMalloc(&pMalloc))
            {
              pMalloc->Free(pIdList);
              pMalloc->Release();
            }
            SetWindowText(hPath, szBuf);
          }
          return TRUE;
        }

        case IDOK:
        {
          char szSourcePath[MAX_PATH];
          char szTargetPath[MAX_PATH];

          GetWindowText(hPath, szBuf, MAX_PATH);

          //Plugin installation
          wsprintf(szTargetPath, "%s\\Docs\\%s", szBuf, PROGRAM_NAME);
          if (!CopyFileWithMessages(hwnd, szExeDir, "Readme.txt", szTargetPath, NULL))
            goto Quit;
          wsprintf(szSourcePath, "%s\\Example", szExeDir);
          wsprintf(szTargetPath, "%s\\Examples\\%s", szBuf, PROGRAM_NAME);
          if (!CopyFileWithMessages(hwnd, szSourcePath, PROGRAM_NAME "Test.nsi", szTargetPath, NULL))
            goto Quit;
          wsprintf(szSourcePath, "%s\\Include", szExeDir);
          wsprintf(szTargetPath, "%s\\Include", szBuf);
          if (!CopyFileWithMessages(hwnd, szSourcePath, PROGRAM_NAME ".nsh", szTargetPath, NULL))
            goto Quit;
          wsprintf(szSourcePath, "%s\\Plugin", szExeDir);
          wsprintf(szTargetPath, "%s\\Plugins", szBuf);
          if (!CopyFileWithMessages(hwnd, szSourcePath, PROGRAM_NAME ".dll", szTargetPath, NULL))
            goto Quit;

          DestroyWindow(hwnd);
          MessageBox(NULL, "Installation complite", PROGRAM_CAPTION, MB_OK|MB_ICONINFORMATION);
          PostQuitMessage(0);
          return TRUE;
        }

        case IDCANCEL:
        {
          Quit:
          DestroyWindow(hwnd);
          PostQuitMessage(0);
          return TRUE;
        }
      }
  }
  return FALSE;
}

int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
  char szPath[MAX_PATH];
  BOOL bEnable=FALSE;

  if (uMsg == BFFM_INITIALIZED || uMsg == BFFM_SELCHANGED)
  {
    if (uMsg == BFFM_INITIALIZED)
    {
      SendMessage(hwnd, BFFM_SETSELECTION, 1, lpData);
      lstrcpy(szPath, (char *)lpData);
    }
    else if (uMsg == BFFM_SELCHANGED)
    {
      SHGetPathFromIDList((LPITEMIDLIST)lParam,szPath);
    }

    if (*szPath)
    {
      lstrcat(szPath, PROGRAM_FILE);
      bEnable=FileExists(szPath);
    }
    SendMessage(hwnd, BFFM_ENABLEOK, 0, bEnable);
  }
  return 0;
}

BOOL CopyFileWithMessages(HWND hwnd, char *pSourcePath, char *pSourceFile, char *pTargetPath, char *pTargetFile)
{
  char szSource[MAX_PATH];
  char szTarget[MAX_PATH];
  char szTmp[MAX_PATH+32];
  int nChoice;

  wsprintf(szSource, "%s\\%s", pSourcePath, pSourceFile);

  if (!FileExists(szSource))
  {
    wsprintf(szTmp, "%s\n\nFile does not exists. Continue?", szSource);
    if (MessageBox(hwnd, szTmp, PROGRAM_CAPTION, MB_YESNO|MB_ICONEXCLAMATION|MB_DEFBUTTON2) == IDNO)
      return FALSE;
    return TRUE;
  }

  wsprintf(szTmp, "%s\\*.*", pTargetPath);

  if (!FileExists(szTmp))
  {
    CreateDirectory(pTargetPath, NULL);
  }

  wsprintf(szTarget, "%s\\%s", pTargetPath, pTargetFile?pTargetFile:pSourceFile);

  if (FileExists(szTarget))
  {
    wsprintf(szTmp, "%s\n\nFile already exists. Replace it?", szTarget);

    nChoice=MessageBox(hwnd, szTmp, PROGRAM_CAPTION, MB_YESNOCANCEL|MB_ICONQUESTION);

    if (nChoice == IDNO)
    {
      return TRUE;
    }
    else if (nChoice == IDCANCEL)
    {
      MessageBox(hwnd, "Installation aborted", PROGRAM_CAPTION, MB_OK|MB_ICONEXCLAMATION);
      return FALSE;
    }
  }
  CopyFile(szSource, szTarget, FALSE);

  return TRUE;
}

BOOL FileExists(char *fname)
{
  WIN32_FIND_DATA wfd;
  HANDLE hFind=FindFirstFile(fname, &wfd);

  if (INVALID_HANDLE_VALUE != hFind)
  {
    FindClose(hFind);
    return TRUE;
  }
  return FALSE;
}
