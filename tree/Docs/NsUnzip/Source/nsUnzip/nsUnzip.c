/*****************************************************************
 *                   nsUnzip NSIS plugin v1.0                    *
 *                                                               *
 * 2009 Shengalts Aleksander aka Instructor (Shengalts@mail.ru)  *
 *****************************************************************/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "..\windll\windll.h"
#include "ConvFunc.h"
#include "StrFunc.h"
#include "StackFunc.h"
#include "Resources\Resource.h"


/* Include conversion functions */
#define xatoiA
#define xitoaA
#include "ConvFunc.h"

/* Include string functions */
#define GetOptionsA
#include "StrFunc.h"

//Include stack functions
#define StackInsert
#define StackClear
#include "StackFunc.h"

/* Defines */
#define NSIS_MAX_STRLEN    1024

typedef struct _FILEA {
  struct _FILEA *next;
  struct _FILEA *prev;
  char szFile[MAX_PATH];
} FILEA;

/* ExDll */
typedef struct _stack_t {
  struct _stack_t *next;
  char text[1];
} stack_t;

typedef struct {
  int autoclose;
  int all_user_var;
  int exec_error;
  int abort;
  int exec_reboot;
  int reboot_called;
  int XXX_cur_insttype;
  int XXX_insttype_changed;
  int silent;
  int instdir_error;
  int rtl;
  int errlvl;
  int alter_reg_view;
} exec_flags_type;

typedef struct {
  exec_flags_type *exec_flags;
  int (__stdcall *ExecuteCodeSegment)(int, HWND);
  void (__stdcall *validate_filename)(char *);
} extra_parameters;

stack_t **g_stacktop;
char *g_variables;
unsigned int g_stringsize;
extra_parameters *g_pluginParms;

#define EXDLL_INIT()        \
{                           \
  g_stacktop=stacktop;      \
  g_variables=variables;    \
  g_stringsize=string_size; \
  g_pluginParms=extra;      \
}

/* Global variables */
extern HANDLE hInst;
char szBuf[NSIS_MAX_STRLEN];
char szBuf2[NSIS_MAX_STRLEN];
char szZipFile[MAX_PATH];
char szExtractDir[MAX_PATH];
char szPassword[MAX_PATH];
char szEncryptedFile[MAX_PATH];
char szDialogTitle[MAX_PATH];
char szDialogGroup[MAX_PATH];
char szDialogLabel[MAX_PATH];
char szDialogOK[MAX_PATH];
char szDialogCancel[MAX_PATH];
char szDialogPassword[MAX_PATH];
HSTACK hIncludeStack={0};
HSTACK hExcludeStack={0};
int lpPasswordCallback;
int lpServiceCallback;
int lpVerboseCallback;

/* Funtions prototypes and macros */
int WINAPI printCallback(LPSTR buf, unsigned long size);
int WINAPI replaceCallback(LPSTR filename);
int WINAPI passwordCallback(LPSTR p, int n, LPCSTR m, LPCSTR name);
void WINAPI SendApplicationMessageCallback(unsigned long ucsize, unsigned long csiz,
    unsigned cfactor, unsigned mo, unsigned dy, unsigned yr, unsigned hh, unsigned mm,
    char lowercase, LPSTR filename, LPSTR methbuf, unsigned long crc, char fCrypt);
int WINAPI ServiceCallback(LPCSTR zipname, unsigned long zipsize, LPCSTR name, unsigned long usize, unsigned long csize, unsigned long crc);
BOOL CALLBACK DecryptDlgProcA(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
FILEA* StackInsertFileA(HSTACK *hStack, char *pFile);
int StackGetFilesA(HSTACK *hStack, char **pFiles, int nCountMax);
int SlashToBackSlashA(const char *pInPath, char *szOutPath);
int BackSlashToSlashA(const char *pInPath, char *szOutPath);
int popinteger();
void pushinteger(int integer);
int popstring(char *str, int len);
void pushstring(const char *str, int len);


/* NSIS functions code */
void __declspec(dllexport) Extract(HWND hwndParent, int string_size, char *variables, stack_t **stacktop, extra_parameters *extra)
{
  EXDLL_INIT();
  {
    USERFUNCTIONS uf={0};
    DCL dcl={0};
    char **infv=NULL;
    char **exfv=NULL;
    int infc=0;
    int exfc=0;
    int nResult;

    szPassword[0]='\0';
    szEncryptedFile[0]='\0';
    lpPasswordCallback=-1;
    lpServiceCallback=-1;
    lpVerboseCallback=-1;

    //USERFUNCTIONS
    uf.print                  = printCallback;
    uf.sound                  = NULL;
    uf.replace                = replaceCallback;
    uf.password               = passwordCallback;
    uf.SendApplicationMessage = SendApplicationMessageCallback;
    uf.ServCallBk             = ServiceCallback;

    //DCL
    dcl.ExtractOnlyNewer    = FALSE;
                              //true for "update" without interaction
                              //(extract only newer/new files, without queries)
    dcl.SpaceToUnderscore   = FALSE;
                              //true if convert space to underscore
    dcl.PromptToOverwrite   = FALSE;
                              // true if prompt to overwrite is wanted
    dcl.fQuiet              = 2;
                              //quiet flag: 0 = all messages, 1 = few messages, 2 = no messages
    dcl.ncflag              = FALSE;
                              //write to stdout if true
    dcl.ntflag              = FALSE;
                              //test zip file
    dcl.nvflag              = FALSE;
                              //verbose listing
    dcl.neflag              = FALSE;
                              //replace only existing files
    dcl.nfflag              = FALSE;
                              //"freshen" (replace existing files by newer versions)
    dcl.nzflag              = FALSE;
                              //display zip file comment
    dcl.ndflag              = 1;
                              //controls (sub)directory recreation during extraction
                              //0 = junk paths from filenames
                              //1 = "safe" usage of paths in filenames (skip "../")
                              //2 = allow also unsafe path components (dir traversal)
    dcl.noflag              = TRUE;
                              //true if you are to always overwrite existing files
    dcl.naflag              = FALSE;
                              //do end-of-line translation
    dcl.nZIflag             = FALSE;
                              //get ZipInfo if TRUE
    dcl.C_flag              = FALSE;
                              //be case insensitive if TRUE
    dcl.fPrivilege          = 0;
                              //1 => restore ACLs in user mode,
                              //2 => try to use privileges for restoring ACLs
    dcl.lpszZipFN           = NULL;
                              //zip file name
    dcl.lpszExtractDir      = NULL;
                              //directory to extract to. This should be NULL if you
                              //are extracting to the current directory.

    //Get parameters from NSIS stack
    while (!popstring(szBuf, NSIS_MAX_STRLEN))
    {
      if (lstrcmpiA(szBuf, "/END"))
      {
        if (szBuf[0] == '/')
        {
          if (!lstrcmpA(szBuf, "/t"))
            dcl.ntflag=TRUE;
          else if (!lstrcmpA(szBuf, "/e"))
            dcl.neflag=TRUE;
          else if (!lstrcmpA(szBuf, "/f"))
            dcl.nfflag=TRUE;
          else if (!lstrcmpA(szBuf, "/u"))
            dcl.ExtractOnlyNewer=TRUE;
          else if (!lstrcmpA(szBuf, "/n"))
            dcl.noflag=FALSE;
          else if (!lstrcmpA(szBuf, "/j"))
            dcl.ndflag=0;
          else if (!lstrcmpA(szBuf, "/C"))
            dcl.C_flag=TRUE;
          else if (!lstrcmpA(szBuf, "/s"))
            dcl.SpaceToUnderscore=TRUE;
          else if (!lstrcmpA(szBuf, "/XX"))
            dcl.fPrivilege=2;
          else if (!lstrcmpA(szBuf, "/X"))
            dcl.fPrivilege=1;
          else if (GetOptionsA(szBuf, "/x=", TRUE, szBuf2, MAX_PATH))
          {
            BackSlashToSlashA(szBuf2, szBuf2);
            StackInsertFileA(&hExcludeStack, szBuf2);
            ++exfc;
          }
          else if (GetOptionsA(szBuf, "/d=", TRUE, szExtractDir, MAX_PATH))
          {
            dcl.lpszExtractDir=szExtractDir;
          }
          else if (GetOptionsA(szBuf, "/P=", TRUE, szPassword, MAX_PATH))
          {
          }
          else if (GetOptionsA(szBuf, "/callP=", FALSE, szBuf2, MAX_PATH))
          {
            lpPasswordCallback=xatoiA(szBuf2);
          }
          else if (GetOptionsA(szBuf, "/callS=", FALSE, szBuf2, MAX_PATH))
          {
            lpServiceCallback=xatoiA(szBuf2);
          }
          else if (GetOptionsA(szBuf, "/callV=", FALSE, szBuf2, MAX_PATH))
          {
            lpVerboseCallback=xatoiA(szBuf2);
            dcl.nvflag=TRUE;
          }
        }
        else
        {
          if (!dcl.lpszZipFN)
          {
            BackSlashToSlashA(szBuf, szBuf);
            lstrcpynA(szZipFile, szBuf, MAX_PATH);
            dcl.lpszZipFN=szZipFile;
          }
          else
          {
            BackSlashToSlashA(szBuf, szBuf);
            StackInsertFileA(&hIncludeStack, szBuf);
            ++infc;
          }
        }
      }
      else break;
    }

    //Set file names to be unarchived
    if (infc)
    {
      if (infv=(char **)GlobalAlloc(GPTR, infc * sizeof(char*)))
      {
        StackGetFilesA(&hIncludeStack, infv, infc);
      }
    }

    //Set file names to be excluded from the unarchiving process
    if (exfc)
    {
      if (exfv=(char **)GlobalAlloc(GPTR, exfc * sizeof(char*)))
      {
        StackGetFilesA(&hExcludeStack, exfv, exfc);
      }
    }

    //Decompress
    nResult=Wiz_SingleEntryUnzip(infc, infv, exfc, exfv, &dcl, &uf);

    //Error codes:
    // 0   Normal; no errors or warnings detected. (There may still be errors in the archive, but if so, they weren't particularly relevant to UnZip's processing and are presumably quite minor.)
    // 1   One or more warning errors were encountered, but processing completed successfully anyway. This includes zipfiles where one or more files was skipped due to unsupported compression method or encryption with an unknown password.
    // 2   A generic error in the zipfile format was detected. Processing may have completed successfully anyway; some broken zipfiles created by other archivers have simple work-arounds.
    // 3   A severe error in the zipfile format was detected. Processing probably failed immediately.
    // 4   UnZip was unable to allocate memory for one or more buffers during program initialization.
    // 5   UnZip was unable to allocate memory or unable to obtain a tty (terminal) to read the decryption password(s).
    // 6   UnZip was unable to allocate memory during decompression to disk.
    // 7   UnZip was unable to allocate memory during in-memory decompression.
    // 9   The specified zipfile(s) was not found.
    //10   Invalid options were specified on the command line.
    //11   No matching files were found.
    //50   The disk is (or was) full during extraction.
    //51   The end of the ZIP archive was encountered prematurely.
    //80   The user aborted UnZip prematurely with control-C (or similar)
    //81   Testing or extraction of one or more files failed due to unsupported compression methods or unsupported decryption.
    //82   No files were found due to bad decryption password(s). (If even one file is successfully processed, however, the exit status is 1.)

    //Free memory
    StackClear((stack **)&hIncludeStack.first, (stack **)&hIncludeStack.last);
    StackClear((stack **)&hExcludeStack.first, (stack **)&hExcludeStack.last);
    if (infv) GlobalFree((HGLOBAL)infv);
    if (exfv) GlobalFree((HGLOBAL)exfv);

    //Return error code
    pushinteger(nResult);
  }
}

void __declspec(dllexport) GetPassword(HWND hwndParent, int string_size, char *variables, stack_t **stacktop, extra_parameters *extra)
{
  EXDLL_INIT();
  {
    popstring(szDialogTitle, MAX_PATH);
    popstring(szDialogGroup, MAX_PATH);
    popstring(szDialogLabel, MAX_PATH);
    popstring(szDialogOK, MAX_PATH);
    popstring(szDialogCancel, MAX_PATH);

    szDialogPassword[0]='\0';
    DialogBoxA(hInst, MAKEINTRESOURCEA(IDD_DECRYPT), hwndParent, (DLGPROC)DecryptDlgProcA);
    pushstring(szDialogPassword, MAX_PATH);
  }
}

int WINAPI printCallback(LPSTR buf, unsigned long size)
{
  return (int)(unsigned int) size;
}

int WINAPI replaceCallback(LPSTR filename)
{
  return 1;
}

int WINAPI passwordCallback(LPSTR p, int n, LPCSTR zfn/*m*/, LPCSTR name)
{
  if (!*szPassword)
  {
    if (lstrcmpiA(szEncryptedFile, zfn))
    {
      if (lpPasswordCallback >= 0)
      {
        SlashToBackSlashA(zfn, szBuf);
        pushstring(szBuf, NSIS_MAX_STRLEN);

        g_pluginParms->ExecuteCodeSegment(lpPasswordCallback - 1, 0);

        popstring(p, n);
      }
      lstrcpynA(szEncryptedFile, zfn, MAX_PATH);
    }
  }
  else
  {
    lstrcpynA(p, szPassword, n);
  }
  return 0;
}

void WINAPI SendApplicationMessageCallback(unsigned long ucsize, unsigned long csiz,
    unsigned cfactor, unsigned mo, unsigned dy, unsigned yr, unsigned hh, unsigned mm,
    char lowercase, LPSTR filename, LPSTR methbuf, unsigned long crc, char fCrypt)
{
  if (lpVerboseCallback >= 0)
  {
    wsprintf(szBuf, "%c", fCrypt);
    pushstring(szBuf, NSIS_MAX_STRLEN);          //Encrypted: "E" or not " "
    pushstring(methbuf, NSIS_MAX_STRLEN);        //Method: "Stored", "Shrunk", "Reduce1", "Reduce2", "Reduce3", "Reduce4", "Implode", "Token", "Defl:#", "Def64#", "ImplDCL", "PKres11", "BZip2", "Unk:###"
    wsprintf(szBuf, "%02u:%02u", hh, mm);
    pushstring(szBuf, NSIS_MAX_STRLEN);          //File time: "hh:mm"
    wsprintf(szBuf, "%02u.%02u.%02u", mo, dy, yr);
    pushstring(szBuf, NSIS_MAX_STRLEN);          //File date: "dd.MM.yy"
    wsprintf(szBuf, "%s%d%%", (csiz > ucsize?"-":""), cfactor);
    pushstring(szBuf, NSIS_MAX_STRLEN);          //Compress ratio
    wsprintf(szBuf, "%08x", crc);
    pushstring(szBuf, NSIS_MAX_STRLEN);          //CRC: ff00ff00
    wsprintf(szBuf, "%d", csiz);
    pushstring(szBuf, NSIS_MAX_STRLEN);          //Compressed size
    wsprintf(szBuf, "%d", ucsize);
    pushstring(szBuf, NSIS_MAX_STRLEN);          //Uncompressed size
    pushstring(filename, NSIS_MAX_STRLEN);       //File name

    g_pluginParms->ExecuteCodeSegment(lpVerboseCallback - 1, 0);
  }
}

int WINAPI ServiceCallback(LPCSTR zipname, unsigned long zipsize, LPCSTR name, unsigned long usize, unsigned long csize, unsigned long crc)
{
  if (lpServiceCallback >= 0)
  {
    wsprintf(szBuf, "%08x", crc);
    pushstring(szBuf, NSIS_MAX_STRLEN);
    xitoaA(csize, szBuf, 0);
    pushstring(szBuf, NSIS_MAX_STRLEN);
    xitoaA(usize, szBuf, 0);
    pushstring(szBuf, NSIS_MAX_STRLEN);
    SlashToBackSlashA(name, szBuf);
    pushstring(szBuf, NSIS_MAX_STRLEN);
    xitoaA(zipsize, szBuf, 0);
    pushstring(szBuf, NSIS_MAX_STRLEN);
    SlashToBackSlashA(zipname, szBuf);
    pushstring(szBuf, NSIS_MAX_STRLEN);

    g_pluginParms->ExecuteCodeSegment(lpServiceCallback - 1, 0);

    if (g_pluginParms->exec_flags->exec_error)
      return 1;
  }
  return 0;
}

BOOL CALLBACK DecryptDlgProcA(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  static HWND hWndPass1;
  static HWND hWndOK;

  if (uMsg == WM_INITDIALOG)
  {
    hWndPass1=GetDlgItem(hDlg, IDC_PASS1);
    hWndOK=GetDlgItem(hDlg, IDOK);

    SetWindowText(hDlg, szDialogTitle);
    SetDlgItemTextA(hDlg, IDC_GROUP_LABEL, szDialogGroup);
    SetDlgItemTextA(hDlg, IDC_PASS1_LABEL, szDialogLabel);
    SetDlgItemTextA(hDlg, IDOK, szDialogOK);
    SetDlgItemTextA(hDlg, IDCANCEL, szDialogCancel);

    SendMessage(hWndPass1, EM_LIMITTEXT, (WPARAM)MAX_PATH, 0);
    EnableWindow(hWndOK, FALSE);
  }
  else if (uMsg == WM_COMMAND)
  {
    if (LOWORD(wParam) == IDC_PASS1)
    {
      if (GetWindowTextLengthA(hWndPass1))
        EnableWindow(hWndOK, TRUE);
      else
        EnableWindow(hWndOK, FALSE);
    }
    else if (LOWORD(wParam) == IDOK)
    {
      GetWindowTextA(hWndPass1, szDialogPassword, MAX_PATH);
      EndDialog(hDlg, 0);
      return TRUE;
    }
    else if (LOWORD(wParam) == IDCANCEL)
    {
      EndDialog(hDlg, 0);
      return TRUE;
    }
  }
  return FALSE;
}

FILEA* StackInsertFileA(HSTACK *hStack, char *pFile)
{
  FILEA *lpElement=NULL;

  if (!StackInsert((stack **)&hStack->first, (stack **)&hStack->last, (stack **)&lpElement, -1, sizeof(FILEA)))
  {
    lstrcpyn(lpElement->szFile, pFile, MAX_PATH);
  }
  return lpElement;
}

int StackGetFilesA(HSTACK *hStack, char **pFiles, int nCountMax)
{
  FILEA *lpElement=(FILEA *)hStack->last;
  int nCount=0;

  if (nCountMax > 0)
  {
    while (lpElement)
    {
      pFiles[nCount++]=lpElement->szFile;
      if (nCount == nCountMax) break;

      lpElement=lpElement->prev;
    }
  }
  return nCount;
}

int SlashToBackSlashA(const char *pInPath, char *szOutPath)
{
  int i;

  for (i=0; pInPath[i]; ++i)
  {
    if (pInPath[i] == '/')
      szOutPath[i]='\\';
    else
      szOutPath[i]=pInPath[i];
  }
  szOutPath[i]=pInPath[i];

  return i;
}

int BackSlashToSlashA(const char *pInPath, char *szOutPath)
{
  int i;

  for (i=0; pInPath[i]; ++i)
  {
    if (pInPath[i] == '\\')
      szOutPath[i]='/';
    else
      szOutPath[i]=pInPath[i];
  }
  szOutPath[i]=pInPath[i];

  return i;
}

int popinteger()
{
  char szInt[32];

  popstring(szInt, 31);
  return xatoiA(szInt);
}

void pushinteger(int integer)
{
  char szInt[32];

  xitoaA(integer, szInt, 0);
  pushstring(szInt, 32);
}

//Function: Removes the element from the top of the NSIS stack and puts it in the buffer
int popstring(char *str, int len)
{
  stack_t *th;

  if (!g_stacktop || !*g_stacktop) return 1;
  th=(*g_stacktop);
  lstrcpyn(str, th->text, len);
  *g_stacktop=th->next;
  GlobalFree((HGLOBAL)th);
  return 0;
}

//Function: Adds an element to the top of the NSIS stack
void pushstring(const char *str, int len)
{
  stack_t *th;

  if (!g_stacktop) return;
  th=(stack_t*)GlobalAlloc(GPTR, sizeof(stack_t) + len);
  lstrcpyn(th->text, str, len);
  th->next=*g_stacktop;
  *g_stacktop=th;
}
