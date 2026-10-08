/*****************************************************************
 *                Registry NSIS plugin v4.2                      *
 *                                                               *
 * 2023 Shengalts Aleksander aka Instructor (Shengalts@mail.ru)  *
 *****************************************************************/


//Comment-out and recompile
#define REGISTRY_SEARCH        //Compile with search function
#define REGISTRY_KEY_EXISTS    //Compile with check key function
#define REGISTRY_READ          //Compile with read function
#define REGISTRY_WRITE         //Compile with write function
#define REGISTRY_READ_EXTRA    //Compile with extra read function
#define REGISTRY_WRITE_EXTRA   //Compile with extra write function
#define REGISTRY_CREATE        //Compile with create function
#define REGISTRY_DELETE        //Compile with delete key/value functions
#define REGISTRY_COPY_VALUE    //Compile with copy/move value functions
#define REGISTRY_COPY_KEY      //Compile with copy/move key functions
#define REGISTRY_BACKUP        //Compile with export/import functions
#define REGISTRY_CONVERT       //Compile with convertion functions



#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "StackFunc.h"
#include "StrFunc.h"
#include "WideFunc.h"
#ifdef POCKETPC
  #include <basetyps.h>
  #include "RAPI\rapi.h"
#endif


//Include stack functions
#define StackInsertAfter
#define StackInsertBefore
#define StackInsertIndex
#define StackDelete
#define StackClear
#include "StackFunc.h"

//Include string functions
#define WideCharLower
#define xmemcpy
#define xmemset
#define xatoiW
#define xitoaW
#define xstrcmpiW
#define xstrlenA
#define xstrlenW
#define xstrcpyW
#define xstrcpynA
#define xstrcpynW
#define xuitoaW
#define dec2hexA
#define dec2hexW
#define hex2decA
#define hex2decW
#define hex2binA
#define hex2binW
#define bin2hexA
#define bin2hexW
#define xprintfW
#define xstrstrW
#include "StrFunc.h"

//Include wide functions
#ifndef POCKETPC
  #define RegCreateKeyExWide
  #define RegDeleteKeyWide
  #define RegDeleteValueWide
  #define RegEnumKeyExWide
  #define RegEnumValueWide
  #define RegOpenKeyExWide
  #define RegQueryValueExWide
  #define RegSetValueExWide
  #define RegEnumKeyExWide
#endif
#define SetWindowTextWide
#define CreateFileWide
#define CreateProcessWide
#define SearchPathWide
#define GetFileAttributesWide
#define FileExistsWide
#include "WideFunc.h"

//Defines
#define NSIS_MAX_STRLEN       1024
#define MAX_PATHLEN           MAX_PATH+32
#ifndef POCKETPC
  #define MAX_DATALEN   65536
#else
  #define MAX_DATALEN   4096
#endif
#ifndef REG_QWORD
  #define REG_QWORD     11
#endif
#define IDC_STATIC_DETAILS    1006
#define PPC_TIMEOUT           3000

#define RO_TYPE_ALL          -1

#define RO_NAME_ALL           0
#define RO_NAME_EXACT         1
#define RO_NAME_SUBSTR_S      2
#define RO_NAME_SUBSTR_I      3

#define RO_GOTO_END           0
#define RO_GOTO_BEGIN         1
#define RO_GOTO_OPENKEY       2
#define RO_GOTO_NEXTKEY       3
#define RO_GOTO_NEXTVALUE     4

#define RO_BANNER_IGNORE      0
#define RO_BANNER_DETAILS     1
#define RO_BANNER_CALLBACK    2

#define RO_DELETE_IGNORE      0
#define RO_DELETE_ROOT        1
#define RO_DELETE_ANY         2

#ifndef mod
  #define mod(a)  (((a) < 0) ? (0 - (a)) : (a))
#endif
#ifndef SWAP_ENDIAN
  #define SWAP_ENDIAN(x) (((x<<24)&0xFF000000)|((x<<8)&0xFF0000)|((x>>8)&0xFF00)|((x>>24)&0xFF))
#endif

typedef struct _KEYITEM {
  struct _KEYITEM *next;
  struct _KEYITEM *prev;
  wchar_t wszKeyItem[MAX_PATHLEN];
} KEYITEM;

typedef struct _HREGSTACK {
  HSTACK hKeysStack;
  wchar_t wszPath[MAX_PATHLEN];
  DWORD dwPathLen;
  wchar_t wszName[MAX_PATHLEN];
  DWORD dwNameLen;
  wchar_t wszKey[MAX_PATHLEN];
  wchar_t wszValue[MAX_PATHLEN];
  unsigned char szString[MAX_DATALEN];
  HKEY hKeyRoot;
  HKEY hKeyHandle;
  DWORD dwIndex;
  int nElement;
  int nFindType;
  int nBanner;
  int nGoto;
  int nNames;
  BOOL bFindKeys;
  BOOL bFindValues;
  BOOL bFindStrings;
  BOOL bGotoSubkeys;
} HREGSTACK;

//ExDll
typedef struct _stack_t {
  struct _stack_t *next;
  wchar_t text[1];
} stack_t;

typedef struct
{
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
} exec_flags_t;

typedef struct {
  exec_flags_t *exec_flags;
  int (__stdcall *ExecuteCodeSegment)(int, HWND);
  void (__stdcall *validate_filename)(wchar_t *);
} extra_parameters;

stack_t **g_stacktop;
wchar_t *g_variables;
unsigned int g_stringsize;
extra_parameters *g_pluginParms;
BOOL g_unicode=-1;

//Global variables
wchar_t wszBuf[MAX_DATALEN];
wchar_t wszBuf2[MAX_DATALEN];
wchar_t wszName[MAX_PATHLEN];
wchar_t wszPath[MAX_PATHLEN];
wchar_t wszKey[MAX_PATHLEN];
wchar_t wszValue[MAX_PATHLEN];
unsigned char szString[MAX_DATALEN];
BOOL bMoveValue=FALSE;
BOOL bMoveKey=FALSE;
HWND hSearchDetails=0;
HREGSTACK *hRegStack;

//Funtions prototypes
int RegConvertPath(wchar_t *wpFullPath, HKEY *hKeyRoot, wchar_t *wszKeyRoot, wchar_t *wszKey);
int TypeNameToTypeValue(wchar_t *pName);
wchar_t* TypeValueToTypeName(DWORD dwType);
DWORD RegReadType(wchar_t *wszOutput, DWORD dwOutputMax, wchar_t **wpType, DWORD dwType, const unsigned char *szString, DWORD dwStringSize);
void ReplaceEscapeSequence(wchar_t *wpResult, wchar_t *wpText);
LONG RegCopyKey(HKEY hSource, wchar_t *wszPathSource, HKEY hTarget, wchar_t *wszPathTarget);
int RegCopyValue(HKEY hSource, HKEY hTarget, wchar_t *wszPathSource, wchar_t *wpValueSource, wchar_t *wszPathTarget, wchar_t *wpValueTarget, BOOL bMove);
LONG RegDeleteSubKeys(HKEY hKeyRoot, const wchar_t *lpSubKey);
int RegDeleteKeyPath(HKEY hKeyRoot, wchar_t *wszPath);
BOOL WriteRegFile(BOOL bUnicode, HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
int GetOptionsW(wchar_t *wpLine, wchar_t *wpOption, BOOL bSensitive, wchar_t *wszResult, int nMaxResult);

LONG API_RegCreateKeyEx(HKEY hKey, const wchar_t *lpSubKey, DWORD Reserved, wchar_t *lpClass, DWORD dwOptions, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition);
LONG API_RegOpenKeyEx(HKEY hKey, const wchar_t *lpSubKey, DWORD ulOptions, REGSAM c, PHKEY phkResult);
LONG API_RegEnumKeyEx(HKEY hKey, DWORD dwIndex, wchar_t *lpName, LPDWORD lpcName, LPDWORD lpReserved, wchar_t *lpClass, LPDWORD lpcClass, PFILETIME lpftLastWriteTime);
LONG API_RegCloseKey(HKEY hKey);
LONG API_RegDeleteKey(HKEY hKey, const wchar_t *lpSubKey);
LONG API_RegEnumValue(HKEY hKey, DWORD dwIndex, wchar_t *lpValueName, LPDWORD lpcValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
LONG API_RegDeleteValue(HKEY hKey, const wchar_t *lpValueName);
LONG API_RegQueryValueEx(HKEY hKey, const wchar_t *lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
LONG API_RegSetValueEx(HKEY hKey, const wchar_t *lpValueName, DWORD Reserved, DWORD dwType, BYTE* lpData, DWORD cbData);

INT_PTR popintegerWide();
void pushintegerWide(INT_PTR integer);
int popstringAnsi(char *str, int len);
int popstringWide(wchar_t *str, int len);
void pushstringAnsi(const char *str);
void pushstringWide(const wchar_t *str);
void Initialize(int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra);


//Extern functions
#ifdef REGISTRY_SEARCH
void __declspec(dllexport) _Open(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    hRegStack=(HREGSTACK *)GlobalAlloc(GPTR, sizeof(HREGSTACK));
    hRegStack->nFindType=RO_TYPE_ALL;
    hRegStack->nElement=1;
    hRegStack->nNames=RO_NAME_ALL;
    hRegStack->bFindKeys=TRUE;
    hRegStack->bFindValues=TRUE;
    hRegStack->bFindStrings=TRUE;
    hRegStack->bGotoSubkeys=TRUE;

    popstringWide(hRegStack->wszPath, NSIS_MAX_STRLEN);
    popstringWide(wszBuf, NSIS_MAX_STRLEN);

    hRegStack->dwPathLen=RegConvertPath(hRegStack->wszPath, &hRegStack->hKeyRoot, NULL, hRegStack->wszPath);

    if (hRegStack->dwNameLen=GetOptionsW(wszBuf, L"/N=", FALSE, hRegStack->wszName, NSIS_MAX_STRLEN))
      hRegStack->nNames=RO_NAME_EXACT;
    else if (hRegStack->dwNameLen=GetOptionsW(wszBuf, L"/NS=", FALSE, hRegStack->wszName, NSIS_MAX_STRLEN))
      hRegStack->nNames=RO_NAME_SUBSTR_S;
    else if (hRegStack->dwNameLen=GetOptionsW(wszBuf, L"/NI=", FALSE, hRegStack->wszName, NSIS_MAX_STRLEN))
      hRegStack->nNames=RO_NAME_SUBSTR_I;

    if (GetOptionsW(wszBuf, L"/K=", FALSE, wszBuf2, NSIS_MAX_STRLEN) && *wszBuf2 == L'0')
      hRegStack->bFindKeys=FALSE;
    if (GetOptionsW(wszBuf, L"/V=", FALSE, wszBuf2, NSIS_MAX_STRLEN) && *wszBuf2 == L'0')
      hRegStack->bFindValues=FALSE;
    if (GetOptionsW(wszBuf, L"/S=", FALSE, wszBuf2, NSIS_MAX_STRLEN) && *wszBuf2 == L'0')
      hRegStack->bFindStrings=FALSE;
    if (GetOptionsW(wszBuf, L"/G=", FALSE, wszBuf2, NSIS_MAX_STRLEN) && *wszBuf2 == L'0')
      hRegStack->bGotoSubkeys=FALSE;
    if (GetOptionsW(wszBuf, L"/B=", FALSE, wszBuf2, NSIS_MAX_STRLEN))
    {
      if (*wszBuf2 == L'1')
      {
        hRegStack->nBanner=RO_BANNER_DETAILS;
        if (!hSearchDetails)
        {
          if (hSearchDetails=FindWindowExA(hwndParent, NULL, "#32770", NULL))
            hSearchDetails=GetDlgItem(hSearchDetails, IDC_STATIC_DETAILS);
        }
      }
      else if (*wszBuf2 == L'2')
      {
        hRegStack->nBanner=RO_BANNER_CALLBACK;
      }
    }
    if (GetOptionsW(wszBuf, L"/T=", FALSE, wszBuf2, NSIS_MAX_STRLEN))
      hRegStack->nFindType=TypeNameToTypeValue(wszBuf2);

    if (hRegStack->hKeyRoot && (hRegStack->bFindKeys || hRegStack->bFindValues || hRegStack->bFindStrings))
    {
      if (API_RegOpenKeyEx(hRegStack->hKeyRoot, hRegStack->wszPath, 0, KEY_READ, &hRegStack->hKeyHandle) == ERROR_SUCCESS)
      {
        API_RegCloseKey(hRegStack->hKeyHandle);
        hRegStack->nGoto=RO_GOTO_BEGIN;
        pushintegerWide((INT_PTR)hRegStack);
        return;
      }
    }
    GlobalFree((HGLOBAL)hRegStack);
    pushstringWide(L"0");
  }
}

void __declspec(dllexport) _Find(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    KEYITEM *lpElement;
    wchar_t *wpType;
    DWORD dwKeyLen;
    DWORD dwValueLen;
    DWORD dwStringLen;
    DWORD dwStringSize;
    DWORD dwType;
    int i;

    hRegStack=(HREGSTACK *)popintegerWide();

    if (!hRegStack) goto End;
    if (hRegStack->nGoto == RO_GOTO_NEXTVALUE) goto EnumValue;
    else if (hRegStack->nGoto == RO_GOTO_NEXTKEY) goto EnumKey;
    else if (hRegStack->nGoto == RO_GOTO_OPENKEY) goto OpenKey;
    else if (hRegStack->nGoto == RO_GOTO_BEGIN) goto Begin;
    else if (hRegStack->nGoto == RO_GOTO_END) goto End;

    Begin:
    StackInsertIndex((stack **)&hRegStack->hKeysStack.first, (stack **)&hRegStack->hKeysStack.last, (stack **)&lpElement, -1, sizeof(KEYITEM));
    xstrcpynW(lpElement->wszKeyItem, hRegStack->wszPath, MAX_PATHLEN);

    while (hRegStack->nElement != 0)
    {
      --hRegStack->nElement;

      if (lpElement=(KEYITEM *)hRegStack->hKeysStack.first)
      {
        xstrcpynW(hRegStack->wszPath, lpElement->wszKeyItem, MAX_PATHLEN);
        StackDelete((stack **)&hRegStack->hKeysStack.first, (stack **)&hRegStack->hKeysStack.last, (stack *)lpElement);
      }
      else break;

      if (hRegStack->nBanner == RO_BANNER_DETAILS)
      {
        SetWindowTextWide(hSearchDetails, hRegStack->wszPath);
      }
      else if (hRegStack->nBanner == RO_BANNER_CALLBACK)
      {
        pushstringWide(L"BANNER");
        pushstringWide(L"");
        pushstringWide(L"");
        pushstringWide(hRegStack->wszPath);

        hRegStack->nGoto=RO_GOTO_OPENKEY;
        return;
      }

      OpenKey:
      if (API_RegOpenKeyEx(hRegStack->hKeyRoot, hRegStack->wszPath, 0, KEY_READ, &hRegStack->hKeyHandle) != ERROR_SUCCESS)
        continue;

      hRegStack->dwIndex=(DWORD)-1;
      if (!hRegStack->bFindValues && !hRegStack->bFindStrings)
        goto EnumKey;

      EnumValue:
      ++hRegStack->dwIndex;
      hRegStack->szString[0]='\0';
      dwValueLen=MAX_PATHLEN;
      dwStringSize=MAX_DATALEN;

      if (API_RegEnumValue(hRegStack->hKeyHandle, hRegStack->dwIndex, hRegStack->wszValue, &dwValueLen, NULL, &dwType, hRegStack->szString, &dwStringSize) == ERROR_SUCCESS)
      {
        if (dwStringSize)
          dwStringLen=dwStringSize / sizeof(wchar_t) - 1;
        else
          dwStringLen=0;

        if ((hRegStack->bFindValues && (hRegStack->nNames == RO_NAME_ALL ||
                                        (hRegStack->nNames == RO_NAME_EXACT && dwValueLen == hRegStack->dwNameLen && !xstrcmpiW(hRegStack->wszValue, hRegStack->wszName)) ||
                                        (hRegStack->nNames == RO_NAME_SUBSTR_S && xstrstrW(hRegStack->wszValue, dwValueLen, hRegStack->wszName, hRegStack->dwNameLen, TRUE, NULL, NULL)) ||
                                        (hRegStack->nNames == RO_NAME_SUBSTR_I && xstrstrW(hRegStack->wszValue, dwValueLen, hRegStack->wszName, hRegStack->dwNameLen, FALSE, NULL, NULL))) &&
                                       (hRegStack->nFindType == RO_TYPE_ALL ||
                                        hRegStack->nFindType == (int)dwType)) ||
            (hRegStack->bFindStrings && (hRegStack->nNames == RO_NAME_ALL ||
                                        ((dwType == REG_SZ || dwType == REG_EXPAND_SZ) &&
                                         ((hRegStack->nNames == RO_NAME_EXACT && dwStringLen == hRegStack->dwNameLen && !xstrcmpiW((wchar_t *)hRegStack->szString, hRegStack->wszName)) ||
                                          (hRegStack->nNames == RO_NAME_SUBSTR_S && xstrstrW((wchar_t *)hRegStack->szString, dwStringLen, hRegStack->wszName, hRegStack->dwNameLen, TRUE, NULL, NULL)) ||
                                          (hRegStack->nNames == RO_NAME_SUBSTR_I && xstrstrW((wchar_t *)hRegStack->szString, dwStringLen, hRegStack->wszName, hRegStack->dwNameLen, FALSE, NULL, NULL))))) &&
                                        (hRegStack->nFindType == RO_TYPE_ALL || hRegStack->nFindType == (int)dwType)))
        {
          RegReadType(wszBuf, MAX_DATALEN, &wpType, dwType, hRegStack->szString, dwStringSize);
          pushstringWide(wpType);
          pushstringWide(wszBuf);
          pushstringWide(hRegStack->wszValue);
          pushstringWide(hRegStack->wszPath);

          hRegStack->nGoto=RO_GOTO_NEXTVALUE;
          return;
        }
        else goto EnumValue;
      }
      hRegStack->dwIndex=(DWORD)-1;

      EnumKey:
      if (hRegStack->bGotoSubkeys)
      {
        ++hRegStack->dwIndex;
        dwKeyLen=MAX_PATHLEN;

        if (API_RegEnumKeyEx(hRegStack->hKeyHandle, hRegStack->dwIndex, hRegStack->wszKey, &dwKeyLen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
          StackInsertIndex((stack **)&hRegStack->hKeysStack.first, (stack **)&hRegStack->hKeysStack.last, (stack **)&lpElement, -1, sizeof(KEYITEM));
          xprintfW(lpElement->wszKeyItem, L"%s%s%s", hRegStack->wszPath, hRegStack->wszPath[0]?L"\\":L"", hRegStack->wszKey);
          ++hRegStack->nElement;

          if (hRegStack->bFindKeys && (hRegStack->nNames == RO_NAME_ALL ||
                                       (hRegStack->nNames == RO_NAME_EXACT && dwKeyLen == hRegStack->dwNameLen && !xstrcmpiW(hRegStack->wszKey, hRegStack->wszName)) ||
                                       (hRegStack->nNames == RO_NAME_SUBSTR_S && xstrstrW(hRegStack->wszKey, dwKeyLen, hRegStack->wszName, hRegStack->dwNameLen, TRUE, NULL, NULL)) ||
                                       (hRegStack->nNames == RO_NAME_SUBSTR_I && xstrstrW(hRegStack->wszKey, dwKeyLen, hRegStack->wszName, hRegStack->dwNameLen, FALSE, NULL, NULL))))
          {
            pushstringWide(L"REG_KEY");
            pushstringWide(L"");
            pushstringWide(hRegStack->wszKey);
            pushstringWide(hRegStack->wszPath);
            hRegStack->nGoto=RO_GOTO_NEXTKEY;
            return;
          }
          else goto EnumKey;
        }
      }
      API_RegCloseKey(hRegStack->hKeyHandle);
    }
    hRegStack->hKeyHandle=0;
    hRegStack->nGoto=RO_GOTO_END;

    End:
    for(i=0; i < 4; ++i) pushstringWide(L"");
  }
}

void __declspec(dllexport) _Close(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    hRegStack=(HREGSTACK *)popintegerWide();

    if (hRegStack)
    {
      if (hRegStack->hKeyHandle)
        API_RegCloseKey(hRegStack->hKeyHandle);
      if (hRegStack->nBanner == RO_BANNER_DETAILS)
        SetWindowTextWide(hSearchDetails, L"");
      StackClear((stack **)&hRegStack->hKeysStack.first, (stack **)&hRegStack->hKeysStack.last);
      GlobalFree((HGLOBAL)hRegStack);
    }
  }
}
#endif //REGISTRY_SEARCH

#ifdef REGISTRY_KEY_EXISTS
void __declspec(dllexport) _KeyExists(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    HKEY hKeyRoot=0;
    HKEY hKeyHandle=0;
    int nError=-1;

    popstringWide(wszPath, NSIS_MAX_STRLEN);

    RegConvertPath(wszPath, &hKeyRoot, NULL, wszPath);

    if (API_RegOpenKeyEx(hKeyRoot, wszPath, 0, KEY_READ, &hKeyHandle) == ERROR_SUCCESS)
    {
      nError=API_RegCloseKey(hKeyHandle);
    }
    pushstringWide((nError == ERROR_SUCCESS)?L"0":L"-1");
  }
}
#endif //REGISTRY_KEY_EXISTS

#ifdef REGISTRY_READ
void __declspec(dllexport) _Read(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    wchar_t *wpType;
    HKEY hKeyRoot=0;
    HKEY hKeyHandle=0;
    DWORD dwStringSize=MAX_DATALEN;
    DWORD dwType;
    int nError=-1;
    szString[0]='\0';

    popstringWide(wszPath, NSIS_MAX_STRLEN);
    popstringWide(wszName, NSIS_MAX_STRLEN);

    RegConvertPath(wszPath, &hKeyRoot, NULL, wszPath);

    if (API_RegOpenKeyEx(hKeyRoot, wszPath, 0, KEY_QUERY_VALUE, &hKeyHandle) == ERROR_SUCCESS)
    {
      nError=API_RegQueryValueEx(hKeyHandle, wszName, NULL, &dwType, szString, &dwStringSize);
      API_RegCloseKey(hKeyHandle);

      if (nError == ERROR_SUCCESS)
        RegReadType(wszBuf, MAX_DATALEN, &wpType, dwType, szString, dwStringSize);
    }
    pushstringWide((nError == ERROR_SUCCESS)?wpType:L"");
    pushstringWide((nError == ERROR_SUCCESS)?wszBuf:L"");
  }
}
#endif //REGISTRY_READ

#ifdef REGISTRY_WRITE
void __declspec(dllexport) _Write(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    wchar_t *wpBuf=&wszBuf[0];
    HKEY hKeyRoot=0;
    HKEY hKeyHandle=0;
    DWORD dwValue;
    DWORD dwStringSize;
    DWORD dwType;
    DWORD dwDisposition;
    int nError=-1;

    popstringWide(wszPath, NSIS_MAX_STRLEN);
    popstringWide(wszName, NSIS_MAX_STRLEN);
    popstringWide(wszBuf, NSIS_MAX_STRLEN);
    popstringWide(wszBuf2, NSIS_MAX_STRLEN);

    RegConvertPath(wszPath, &hKeyRoot, NULL, wszPath);

    if ((dwType=(DWORD)TypeNameToTypeValue(wszBuf2)) != (DWORD)-1 && API_RegCreateKeyEx(hKeyRoot, wszPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyHandle, &dwDisposition) == ERROR_SUCCESS)
    {
      if (dwType == REG_BINARY ||
          dwType == REG_NONE ||
          dwType == REG_LINK ||
          dwType == REG_RESOURCE_LIST ||
          dwType == REG_FULL_RESOURCE_DESCRIPTOR ||
          dwType == REG_RESOURCE_REQUIREMENTS_LIST ||
          dwType == REG_QWORD)
      {
        dwStringSize=(DWORD)hex2binW(wszBuf, szString, MAX_DATALEN);
        nError=API_RegSetValueEx(hKeyHandle, wszName, 0, dwType, szString, dwStringSize);
      }
      else if (dwType == REG_DWORD ||
               dwType == REG_DWORD_BIG_ENDIAN)
      {
        dwValue=(DWORD)xatoiW(wszBuf, NULL);
        if (dwType == REG_DWORD_BIG_ENDIAN)
          dwValue=SWAP_ENDIAN(dwValue);
        nError=API_RegSetValueEx(hKeyHandle, wszName, 0, dwType, (unsigned char *)&dwValue, sizeof(DWORD));
      }
      else if (dwType == REG_MULTI_SZ)
      {
        while (*wpBuf)
        {
          if (*wpBuf == L'\n') *wpBuf=L'\0';
          ++wpBuf;
        }
        *++wpBuf=L'\0';

        dwStringSize=(DWORD)((wpBuf - wszBuf + 1) * sizeof(wchar_t));
        nError=API_RegSetValueEx(hKeyHandle, wszName, 0, REG_MULTI_SZ, (unsigned char *)wszBuf, dwStringSize);
      }
      else if (dwType == REG_EXPAND_SZ ||
               dwType == REG_SZ)
      {
        dwStringSize=(DWORD)((xstrlenW(wszBuf) + 1) * sizeof(wchar_t));
        nError=API_RegSetValueEx(hKeyHandle, wszName, 0, dwType, (unsigned char *)wszBuf, dwStringSize);
      }
      API_RegCloseKey(hKeyHandle);
    }
    pushstringWide((nError == ERROR_SUCCESS)?L"0":L"-1");
  }
}
#endif //REGISTRY_WRITE

#ifdef REGISTRY_READ_EXTRA
void __declspec(dllexport) _ReadExtra(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    wchar_t *wpBuf=&wszBuf[0];
    wchar_t *wpType;
    HKEY hKeyRoot=0;
    HKEY hKeyHandle=0;
    DWORD dwStringSize=MAX_DATALEN;
    DWORD dwStringLen;
    DWORD dwType;
    int nError=-1;
    int nOffset;
    szString[0]='\0';

    popstringWide(wszPath, NSIS_MAX_STRLEN);
    popstringWide(wszName, NSIS_MAX_STRLEN);
    nOffset=(int)popintegerWide();

    RegConvertPath(wszPath, &hKeyRoot, NULL, wszPath);

    if (API_RegOpenKeyEx(hKeyRoot, wszPath, 0, KEY_QUERY_VALUE, &hKeyHandle) == ERROR_SUCCESS)
    {
      nError=API_RegQueryValueEx(hKeyHandle, wszName, NULL, &dwType, szString, &dwStringSize);
      API_RegCloseKey(hKeyHandle);

      if (nError == ERROR_SUCCESS)
      {
        dwStringLen=RegReadType(wszBuf, MAX_DATALEN, &wpType, dwType, szString, dwStringSize);

        if (dwType == REG_BINARY ||
            dwType == REG_NONE ||
            dwType == REG_LINK ||
            dwType == REG_RESOURCE_LIST ||
            dwType == REG_FULL_RESOURCE_DESCRIPTOR ||
            dwType == REG_RESOURCE_REQUIREMENTS_LIST ||
            dwType == REG_QWORD)
        {
          nOffset*=2;
        }
        if (nOffset > 0)
        {
          if ((DWORD)nOffset < dwStringLen)
            wpBuf+=nOffset;
          else
            wpBuf[0]=L'\0';
        }
        else if (nOffset < 0 && (DWORD)mod(nOffset) < dwStringLen)
        {
          wpBuf+=(dwStringLen + nOffset);
        }
      }
    }
    pushstringWide((nError == ERROR_SUCCESS)?wpType:L"");
    pushstringWide((nError == ERROR_SUCCESS)?wpBuf:L"");
  }
}
#endif //REGISTRY_READ_EXTRA

#ifdef REGISTRY_WRITE_EXTRA
void __declspec(dllexport) _WriteExtra(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    wchar_t *wpBuf=&wszBuf[0];
    HKEY hKeyRoot=0;
    HKEY hKeyHandle=0;
    DWORD dwStringSize=MAX_DATALEN;
    DWORD dwStringSizeExtra;
    DWORD dwType;
    int nError=-1;
    szString[0]='\0';

    popstringWide(wszPath, NSIS_MAX_STRLEN);
    popstringWide(wszName, NSIS_MAX_STRLEN);
    popstringWide(wszBuf, NSIS_MAX_STRLEN);

    RegConvertPath(wszPath, &hKeyRoot, NULL, wszPath);

    if (API_RegOpenKeyEx(hKeyRoot, wszPath, 0, KEY_QUERY_VALUE|KEY_SET_VALUE, &hKeyHandle) == ERROR_SUCCESS)
    {
      nError=API_RegQueryValueEx(hKeyHandle, wszName, NULL, &dwType, szString, &dwStringSize);

      if (nError == ERROR_SUCCESS && *wszBuf)
      {
        if (dwType == REG_BINARY ||
            dwType == REG_NONE ||
            dwType == REG_LINK ||
            dwType == REG_RESOURCE_LIST ||
            dwType == REG_FULL_RESOURCE_DESCRIPTOR ||
            dwType == REG_RESOURCE_REQUIREMENTS_LIST ||
            dwType == REG_QWORD)
        {
          dwStringSizeExtra=(DWORD)hex2binW(wszBuf, szString + dwStringSize, MAX_DATALEN - dwStringSize);
          nError=API_RegSetValueEx(hKeyHandle, wszName, 0, dwType, szString, dwStringSize + dwStringSizeExtra);
        }
        else if (dwType == REG_MULTI_SZ)
        {
          while (*wpBuf)
          {
            if (*wpBuf == L'\n') *wpBuf=L'\0';
            ++wpBuf;
          }
          *++wpBuf=L'\0';

          dwStringSizeExtra=(DWORD)((wpBuf - wszBuf + 1) * sizeof(wchar_t));
          xmemcpy(szString + dwStringSize - sizeof(wchar_t), wszBuf, dwStringSizeExtra);
          nError=API_RegSetValueEx(hKeyHandle, wszName, 0, REG_MULTI_SZ, szString, dwStringSize + dwStringSizeExtra);
        }
        else if (dwType == REG_EXPAND_SZ ||
                 dwType == REG_SZ)
        {
          dwStringSizeExtra=(DWORD)xprintfW((wchar_t *)(szString + dwStringSize - sizeof(wchar_t)), L"%s", wszBuf) * sizeof(wchar_t);
          nError=API_RegSetValueEx(hKeyHandle, wszName, 0, dwType, szString, dwStringSize + dwStringSizeExtra);
        }
      }
      API_RegCloseKey(hKeyHandle);
    }
    pushstringWide((nError == ERROR_SUCCESS)?L"0":L"-1");
  }
}
#endif //REGISTRY_WRITE_EXTRA

#ifdef REGISTRY_CREATE
void __declspec(dllexport) _CreateKey(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    HKEY hKeyRoot=0;
    HKEY hKeyHandle=0;
    DWORD dwDisposition;

    popstringWide(wszPath, NSIS_MAX_STRLEN);

    RegConvertPath(wszPath, &hKeyRoot, NULL, wszPath);

    if (API_RegCreateKeyEx(hKeyRoot, wszPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyHandle, &dwDisposition) == ERROR_SUCCESS)
    {
      API_RegCloseKey(hKeyHandle);

      pushstringWide((dwDisposition == REG_CREATED_NEW_KEY)?L"0":L"1");
      return;
    }
    pushstringWide(L"-1");
  }
}
#endif //REGISTRY_CREATE

#ifdef REGISTRY_DELETE
void __declspec(dllexport) _DeleteValue(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    HKEY hKeyRoot=0;
    HKEY hKeyHandle=0;
    int nError=-1;

    popstringWide(wszPath, NSIS_MAX_STRLEN);
    popstringWide(wszName, NSIS_MAX_STRLEN);

    RegConvertPath(wszPath, &hKeyRoot, NULL, wszPath);

    if (API_RegOpenKeyEx(hKeyRoot, wszPath, 0, KEY_SET_VALUE, &hKeyHandle) == ERROR_SUCCESS)
    {
      nError=API_RegDeleteValue(hKeyHandle, wszName);
      API_RegCloseKey(hKeyHandle);
    }
    pushstringWide((nError == ERROR_SUCCESS)?L"0":L"-1");
  }
}

void __declspec(dllexport) _DeleteKey(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    HKEY hKeyRoot=0;
    int nError=-1;

    popstringWide(wszPath, NSIS_MAX_STRLEN);

    RegConvertPath(wszPath, &hKeyRoot, NULL, wszPath);

    nError=RegDeleteKeyPath(hKeyRoot, wszPath);

    pushstringWide((nError == ERROR_SUCCESS)?L"0":L"-1");
  }
}

void __declspec(dllexport) _DeleteKeyEmpty(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    HKEY hKeyRoot=0;
    HKEY hKeyHandle=0;
    DWORD dwKeyLen=MAX_PATHLEN;
    DWORD dwValueLen=MAX_PATHLEN;
    DWORD dwType;
    int nError=-1;
    popstringWide(wszPath, NSIS_MAX_STRLEN);

    RegConvertPath(wszPath, &hKeyRoot, NULL, wszPath);

    if (API_RegOpenKeyEx(hKeyRoot, wszPath, 0, KEY_READ, &hKeyHandle) == ERROR_SUCCESS)
    {
      if (API_RegEnumKeyEx(hKeyHandle, 0, wszKey, &dwKeyLen, NULL, NULL, NULL, NULL) != ERROR_SUCCESS &&
          API_RegEnumValue(hKeyHandle, 0, wszValue, &dwValueLen, NULL, &dwType, NULL, NULL) != ERROR_SUCCESS)
      {
        nError=ERROR_SUCCESS;
      }
      API_RegCloseKey(hKeyHandle);

      if (nError == ERROR_SUCCESS)
      {
        nError=RegDeleteKeyPath(hKeyRoot, wszPath);
      }
    }
    pushstringWide((nError == ERROR_SUCCESS)?L"0":L"-1");
  }
}
#endif //REGISTRY_DELETE

#ifdef REGISTRY_COPY_VALUE
void __declspec(dllexport) _CopyValue(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    DWORD dwStringSize=MAX_DATALEN;
    DWORD dwType;
    DWORD dwDisposition;
    HKEY hKeyRootSource=0;
    HKEY hKeyRootTarget=0;
    HKEY hSourceHandle;
    HKEY hTargetHandle;
    int nError=-1;
    szString[0]='\0';

    popstringWide(wszPath, NSIS_MAX_STRLEN);
    popstringWide(wszName, NSIS_MAX_STRLEN);
    popstringWide(wszBuf, NSIS_MAX_STRLEN);
    popstringWide(wszBuf2, NSIS_MAX_STRLEN);

    RegConvertPath(wszPath, &hKeyRootSource, NULL, wszPath);
    RegConvertPath(wszBuf, &hKeyRootTarget, NULL, wszBuf);

    if (API_RegOpenKeyEx(hKeyRootSource, wszPath, 0, KEY_QUERY_VALUE|KEY_SET_VALUE, &hSourceHandle) == ERROR_SUCCESS)
    {
      nError=API_RegQueryValueEx(hSourceHandle, wszName, NULL, &dwType, szString, &dwStringSize);

      if (nError == ERROR_SUCCESS && (nError=API_RegCreateKeyEx(hKeyRootTarget, wszBuf, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hTargetHandle, &dwDisposition)) == ERROR_SUCCESS)
      {
        nError=API_RegSetValueEx(hTargetHandle, wszBuf2, 0, dwType, szString, dwStringSize);

        API_RegCloseKey(hTargetHandle);

        if (bMoveValue == TRUE && nError == ERROR_SUCCESS)
          nError=API_RegDeleteValue(hSourceHandle, wszName);
      }
      API_RegCloseKey(hSourceHandle);
    }
    pushstringWide((nError == ERROR_SUCCESS)?L"0":L"-1");
  }
}

void __declspec(dllexport) _MoveValue(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  bMoveValue=TRUE;
  _CopyValue(hwndParent, string_size, variables, stacktop, extra);
  bMoveValue=FALSE;
}
#endif //REGISTRY_COPY_VALUE

#ifdef REGISTRY_COPY_KEY
void __declspec(dllexport) _CopyKey(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    HKEY hKeyRootSource=0;
    HKEY hKeyRootTarget=0;
    int nError;

    popstringWide(wszPath, NSIS_MAX_STRLEN);
    popstringWide(wszBuf, NSIS_MAX_STRLEN);

    RegConvertPath(wszPath, &hKeyRootSource, NULL, wszPath);
    RegConvertPath(wszBuf, &hKeyRootTarget, NULL, wszBuf);

    if ((nError=RegCopyKey(hKeyRootSource, wszPath, hKeyRootTarget, wszBuf)) == ERROR_SUCCESS)
      if (bMoveKey == TRUE) nError=RegDeleteKeyPath(hKeyRootSource, wszPath);

    pushstringWide((nError == ERROR_SUCCESS)?L"0":L"-1");
  }
}

void __declspec(dllexport) _MoveKey(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  bMoveKey=TRUE;
  _CopyKey(hwndParent, string_size, variables, stacktop, extra);
  bMoveKey=FALSE;
}
#endif //REGISTRY_COPY_KEY

#ifdef REGISTRY_BACKUP
void __declspec(dllexport) _SaveKey(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    HSTACK hKeysStack={0};
    KEYITEM *lpElement=NULL;
    wchar_t wszRootName[MAX_PATHLEN];
    wchar_t wszNameFilter[MAX_PATHLEN];
    const wchar_t *wpHeader;
    HANDLE hFileWrite=0;
    HKEY hKeyRoot=0;
    HKEY hKeyHandle=0;
    DWORD dwNameLen;
    DWORD dwKeyLen;
    DWORD dwValueLen;
    DWORD dwStringSize;
    DWORD dwType;
    DWORD dwIndex;
    DWORD dwNumberOfBytesWritten;
    int nElement=1;
    int nNames=RO_NAME_ALL;
    int nReplaceSubkeys=RO_DELETE_IGNORE;
    int nBanner=RO_BANNER_IGNORE;
    BOOL bGotoSubkeys=TRUE;
    BOOL bAppendFile=FALSE;
    BOOL bTypeUnicode=g_unicode;
    int nLenght;

    popstringWide(wszPath, NSIS_MAX_STRLEN);
    popstringWide(wszName, NSIS_MAX_STRLEN);
    popstringWide(wszBuf, NSIS_MAX_STRLEN);

    RegConvertPath(wszPath, &hKeyRoot, wszRootName, wszPath);

    if (dwNameLen=GetOptionsW(wszBuf, L"/N=", FALSE, wszNameFilter, NSIS_MAX_STRLEN))
      nNames=RO_NAME_EXACT;
    if (GetOptionsW(wszBuf, L"/G=", FALSE, wszBuf2, NSIS_MAX_STRLEN) && *wszBuf2 == L'0')
      bGotoSubkeys=FALSE;
    if (GetOptionsW(wszBuf, L"/A=", FALSE, wszBuf2, NSIS_MAX_STRLEN) && *wszBuf2 == L'1')
      bAppendFile=TRUE;
    if (GetOptionsW(wszBuf, L"/U=", FALSE, wszBuf2, NSIS_MAX_STRLEN) && *wszBuf2 == L'1')
      bTypeUnicode=TRUE;
    if (GetOptionsW(wszBuf, L"/B=", FALSE, wszBuf2, NSIS_MAX_STRLEN) && *wszBuf2 == L'1')
    {
      nBanner=RO_BANNER_DETAILS;
      if (!hSearchDetails)
      {
        if (hSearchDetails=FindWindowExA(hwndParent, NULL, "#32770", NULL))
          hSearchDetails=GetDlgItem(hSearchDetails, IDC_STATIC_DETAILS);
      }
    }
    if (GetOptionsW(wszBuf, L"/D=", FALSE, wszBuf2, NSIS_MAX_STRLEN))
    {
      if (*wszBuf2 == L'1')
        nReplaceSubkeys=RO_DELETE_ROOT;
      else if (*wszBuf2 == L'2')
        nReplaceSubkeys=RO_DELETE_ANY;
    }

    if (API_RegOpenKeyEx(hKeyRoot, wszPath, 0, KEY_READ, &hKeyHandle) != ERROR_SUCCESS)
    {
      pushstringWide(L"-1");
      return;
    }
    API_RegCloseKey(hKeyHandle);

    if (bAppendFile)
    {
      hFileWrite=CreateFileWide(wszName, FILE_APPEND_DATA, 0, NULL, OPEN_EXISTING, 0, 0);
      if (hFileWrite == INVALID_HANDLE_VALUE) bAppendFile=FALSE;
    }
    if (!bAppendFile) hFileWrite=CreateFileWide(wszName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    if (hFileWrite == INVALID_HANDLE_VALUE)
    {
      pushstringWide(L"-1");
      return;
    }

    if (!bAppendFile)
    {
      if (bTypeUnicode)
      {
        //Write BOM
        WriteFile(hFileWrite, "\xFF\xFE", 2, &dwNumberOfBytesWritten, NULL);
        wpHeader=L"Windows Registry Editor Version 5.00\r\n";
      }
      else wpHeader=L"REGEDIT4\r\n";

      //Write header
      WriteRegFile(bTypeUnicode, hFileWrite, wpHeader, (int)xstrlenW(wpHeader) * sizeof(wchar_t), &dwNumberOfBytesWritten, NULL);
    }

    StackInsertIndex((stack **)&hKeysStack.first, (stack **)&hKeysStack.last, (stack **)&lpElement, 1, sizeof(KEYITEM));
    xstrcpynW(lpElement->wszKeyItem, wszPath, MAX_PATHLEN);

    while (nElement != 0)
    {
      --nElement;

      if (lpElement=(KEYITEM *)hKeysStack.first)
      {
        xstrcpynW(wszPath, lpElement->wszKeyItem, MAX_PATHLEN);
        StackDelete((stack **)&hKeysStack.first, (stack **)&hKeysStack.last, (stack *)lpElement);
      }
      else break;

      if (nBanner == RO_BANNER_DETAILS)
        SetWindowTextWide(hSearchDetails, wszPath);

      if (API_RegOpenKeyEx(hKeyRoot, wszPath, 0, KEY_READ, &hKeyHandle) != ERROR_SUCCESS)
        continue;

      if (!wszPath[0])
      {
        nLenght=(int)xprintfW(wszBuf, L"\r\n[%s]\r\n", wszRootName);
      }
      else
      {
        if (nReplaceSubkeys != RO_DELETE_IGNORE)
        {
          if (nReplaceSubkeys == RO_DELETE_ROOT) nReplaceSubkeys=RO_DELETE_IGNORE;
          nLenght=(int)xprintfW(wszBuf, L"\r\n[-%s\\%s]", wszRootName, wszPath);
          WriteRegFile(bTypeUnicode, hFileWrite, wszBuf, nLenght * sizeof(wchar_t), &dwNumberOfBytesWritten, NULL);
        }
        nLenght=(int)xprintfW(wszBuf, L"\r\n[%s\\%s]\r\n", wszRootName, wszPath);
      }
      WriteRegFile(bTypeUnicode, hFileWrite, wszBuf, nLenght * sizeof(wchar_t), &dwNumberOfBytesWritten, NULL);

      dwIndex=0;
      for (;;)
      {
        dwValueLen=MAX_PATHLEN;
        dwStringSize=MAX_DATALEN;
        szString[0]='\0';
        if (API_RegEnumValue(hKeyHandle, dwIndex++, wszValue, &dwValueLen, NULL, &dwType, szString, &dwStringSize) != ERROR_SUCCESS)
          break;

        if (nNames == RO_NAME_ALL || (nNames == RO_NAME_EXACT && dwValueLen == dwNameLen && !xstrcmpiW(wszValue, wszNameFilter)))
        {
          //Write value
          unsigned int dwStringCount=0;
          int nCurWidth=0;
          int nColumnWidth;
          DWORD dwNumberOfBytesWritten;

          if (!*wszValue)
          {
            nLenght=(int)xstrcpyW(wszBuf, L"@");
          }
          else
          {
            ReplaceEscapeSequence(wszBuf2, wszValue);
            nLenght=(int)xprintfW(wszBuf, L"\"%s\"", wszBuf2);
          }

          if (dwType == REG_BINARY ||
              dwType == REG_EXPAND_SZ ||
              dwType == REG_MULTI_SZ ||
              dwType == REG_DWORD_BIG_ENDIAN ||
              dwType == REG_NONE ||
              dwType == REG_LINK ||
              dwType == REG_RESOURCE_LIST ||
              dwType == REG_FULL_RESOURCE_DESCRIPTOR ||
              dwType == REG_RESOURCE_REQUIREMENTS_LIST ||
              dwType == REG_QWORD)
          {
            if (!bTypeUnicode)
            {
              if (dwType == REG_EXPAND_SZ ||
                  dwType == REG_MULTI_SZ)
              {
                xstrcpynW(wszBuf2, (const wchar_t *)szString, dwStringSize / sizeof(wchar_t));
                dwStringSize=WideCharToMultiByte(CP_ACP, 0, wszBuf2, dwStringSize / sizeof(wchar_t), (char *)szString, MAX_DATALEN, NULL, NULL);
              }
            }
            if (dwType == REG_BINARY)
              nLenght+=(int)xprintfW(wszBuf + nLenght, L"=hex:");
            else
              nLenght+=(int)xprintfW(wszBuf + nLenght, L"=hex(%x):", dwType);

            if (dwStringSize == 0)
            {
              nLenght+=(int)xprintfW(wszBuf + nLenght, L"\r\n");
              WriteRegFile(bTypeUnicode, hFileWrite, wszBuf, nLenght * sizeof(wchar_t), &dwNumberOfBytesWritten, NULL);
            }
            else
            {
              nColumnWidth=(79 - nLenght) / 3;

              if (nColumnWidth <= 0)
                nColumnWidth=1;

              while (dwStringCount < dwStringSize)
              {
                for (nCurWidth=0; dwStringCount < dwStringSize && nCurWidth < nColumnWidth; ++dwStringCount, ++nCurWidth)
                {
                  nLenght+=(int)xprintfW(wszBuf + nLenght, L"%02x,", (unsigned int)szString[dwStringCount]);
                }
                if (dwStringCount == dwStringSize)
                  nLenght+=(int)xprintfW(wszBuf + (--nLenght), L"\r\n");
                else
                  nLenght+=(int)xprintfW(wszBuf + nLenght, L"\\\r\n");

                WriteRegFile(bTypeUnicode, hFileWrite, wszBuf, nLenght * sizeof(wchar_t), &dwNumberOfBytesWritten, NULL);
                nLenght=(int)xstrcpyW(wszBuf, L"  ");
                nColumnWidth=25;
              }
            }
          }
          else if (dwType == REG_SZ)
          {
            ReplaceEscapeSequence(wszBuf2, (wchar_t *)szString);
            nLenght+=(int)xprintfW(wszBuf + nLenght, L"=\"%s\"\r\n", wszBuf2);
            WriteRegFile(bTypeUnicode, hFileWrite, wszBuf, nLenght * sizeof(wchar_t), &dwNumberOfBytesWritten, NULL);
          }
          else if (dwType == REG_DWORD)
          {
            nLenght+=(int)xprintfW(wszBuf + nLenght, L"=dword:%08x\r\n", *((unsigned int *)szString));
            WriteRegFile(bTypeUnicode, hFileWrite, wszBuf, nLenght * sizeof(wchar_t), &dwNumberOfBytesWritten, NULL);
          }
        }
      }

      if (bGotoSubkeys)
      {
        dwIndex=0;
        for (;;)
        {
          dwKeyLen=MAX_PATHLEN;
          if (API_RegEnumKeyEx(hKeyHandle, dwIndex++, wszKey, &dwKeyLen, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
            break;

          StackInsertIndex((stack **)&hKeysStack.first, (stack **)&hKeysStack.last, (stack **)&lpElement, dwIndex, sizeof(KEYITEM));
          xprintfW(lpElement->wszKeyItem, L"%s%s%s", wszPath, wszPath[0]?L"\\":L"", wszKey);

          ++nElement;
        }
      }
      API_RegCloseKey(hKeyHandle);
    }
    CloseHandle(hFileWrite);
    StackClear((stack **)&hKeysStack.first, (stack **)&hKeysStack.last);
    if (nBanner == RO_BANNER_DETAILS)
      SetWindowTextWide(hSearchDetails, L"");
    pushstringWide(L"0");
  }
}

#ifndef POCKETPC
void __declspec(dllexport) _RestoreKey(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;

    popstringWide(wszName, NSIS_MAX_STRLEN);

    if (FileExistsWide(wszName))
    {
      if (SearchPathWide(NULL, L"regedit.exe", NULL, MAX_DATALEN, wszBuf2, 0))
      {
        xprintfW(wszBuf, L"%s /s \"%s\"", wszBuf2, wszName);
        xmemset(&si, 0, sizeof(STARTUPINFOW));
        si.cb=sizeof(STARTUPINFOW);

        if (CreateProcessWide(NULL, wszBuf, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        {
          CloseHandle(pi.hProcess);
          CloseHandle(pi.hThread);
          pushstringWide(L"0");
          return;
        }
      }
    }
    pushstringWide(L"-1");
  }
}
#endif //POCKETPC
#endif //REGISTRY_BACKUP

#ifdef REGISTRY_CONVERT
void __declspec(dllexport) _StrToHexA(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    char *szBuf=(char *)wszBuf;
    char *szBuf2=(char *)wszBuf2;
    int nLen;

    nLen=popstringAnsi(szBuf, NSIS_MAX_STRLEN);
    bin2hexA((unsigned char *)szBuf, nLen, szBuf2, NSIS_MAX_STRLEN, TRUE);
    pushstringAnsi(szBuf2);
  }
}

void __declspec(dllexport) _StrToHexW(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    int nLen;

    nLen=popstringWide(wszBuf, NSIS_MAX_STRLEN);
    bin2hexW((unsigned char *)wszBuf, nLen * sizeof(wchar_t), wszBuf2, NSIS_MAX_STRLEN, TRUE);
    pushstringWide(wszBuf2);
  }
}

void __declspec(dllexport) _HexToStrA(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    char *szBuf=(char *)wszBuf;
    char *szBuf2=(char *)wszBuf2;
    int nLen;

    popstringAnsi(szBuf, NSIS_MAX_STRLEN);
    nLen=(int)hex2binA(szBuf, (unsigned char *)szBuf2, NSIS_MAX_STRLEN);
    szBuf2[nLen]='\0';
    pushstringAnsi(szBuf2);
  }
}

void __declspec(dllexport) _HexToStrW(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    int nLen;

    popstringWide(wszBuf, NSIS_MAX_STRLEN);
    nLen=(int)hex2binW(wszBuf, (unsigned char *)wszBuf2, NSIS_MAX_STRLEN) / sizeof(wchar_t);
    wszBuf2[nLen]=L'\0';
    pushstringWide(wszBuf2);
  }
}
#endif //REGISTRY_CONVERT

#ifdef POCKETPC
void __declspec(dllexport) _CERAPIINIT(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    if (CeRapiInit() == S_OK)
    {
      pushstringWide(L"0");
      return;
    }
    //Unable to initialize connection to device
    pushstringWide(L"-1");
  }
}

void __declspec(dllexport) _CERAPIINITEX(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    RAPIINIT rapiinit;

    rapiinit.cbSize=sizeof(RAPIINIT);
    rapiinit.heRapiInit=NULL;
    rapiinit.hrRapiInit=-1;

    if (CeRapiInitEx(&rapiinit) != S_OK)
    {
      //Unable to initialize connection to device
      pushstringWide(L"-1");
      return;
    }
    if (MsgWaitForMultipleObjects(1, &rapiinit.heRapiInit, FALSE, PPC_TIMEOUT, 0) == WAIT_TIMEOUT)
    {
      //Timed out waiting for device
      CeRapiUninit();
      pushstringWide(L"-2");
      return;
    }
    if (rapiinit.hrRapiInit != S_OK)
    {
      //Failed to connect to device
      CeRapiUninit();
      pushstringWide(L"-3");
      return;
    }
    pushstringWide(L"0");
  }
}

void __declspec(dllexport) _CERAPIUNINIT(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  CeRapiUninit();
}
#endif //POCKETPC

void __declspec(dllexport) _Unload(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
  if (fdwReason == DLL_PROCESS_ATTACH)
  {
  }
  else if (fdwReason == DLL_THREAD_ATTACH)
  {
  }
  else if (fdwReason == DLL_THREAD_DETACH)
  {
  }
  else if (fdwReason == DLL_PROCESS_DETACH)
  {
  }
  return TRUE;
}


// Function: L"HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft" -> HKEY_LOCAL_MACHINE L"SOFTWARE\Microsoft"
int RegConvertPath(wchar_t *wpFullPath, HKEY *hKeyRoot, wchar_t *wszKeyRoot, wchar_t *wszKey)
{
  wchar_t wszRoot[MAX_PATHLEN];
  wchar_t *wpKeyCount;
  int nFullPathLen;
  int nRootLen;
  int nKeyLen;

  nFullPathLen=(int)xstrlenW(wpFullPath);
  for (wpKeyCount=wpFullPath + nFullPathLen - 1; wpKeyCount >= wpFullPath && *wpKeyCount == L'\\'; --wpKeyCount)
    *wpKeyCount=L'\0';

  for (wpKeyCount=wpFullPath; *wpKeyCount && *wpKeyCount != L'\\'; ++wpKeyCount);
  nRootLen=(int)min(wpKeyCount - wpFullPath + 1, MAX_PATHLEN);
  xstrcpynW(wszRoot, wpFullPath, nRootLen);

  if (*wpKeyCount == L'\\') ++wpKeyCount;
  nKeyLen=(int)min(wpFullPath + nFullPathLen - wpKeyCount + 1, MAX_PATHLEN);
  nKeyLen=(int)xstrcpynW(wszKey, wpKeyCount, nKeyLen);

  if (!xstrcmpiW(wszRoot, L"HKEY_CLASSES_ROOT") || !xstrcmpiW(wszRoot, L"HKCR"))
  {
    *hKeyRoot=HKEY_CLASSES_ROOT;
    if (wszKeyRoot) xstrcpyW(wszKeyRoot, L"HKEY_CLASSES_ROOT");
  }
  else if (!xstrcmpiW(wszRoot, L"HKEY_CURRENT_USER") || !xstrcmpiW(wszRoot, L"HKCU"))
  {
    *hKeyRoot=HKEY_CURRENT_USER;
    if (wszKeyRoot) xstrcpyW(wszKeyRoot, L"HKEY_CURRENT_USER");
  }
  else if (!xstrcmpiW(wszRoot, L"HKEY_LOCAL_MACHINE") || !xstrcmpiW(wszRoot, L"HKLM"))
  {
    *hKeyRoot=HKEY_LOCAL_MACHINE;
    if (wszKeyRoot) xstrcpyW(wszKeyRoot, L"HKEY_LOCAL_MACHINE");
  }
  else if (!xstrcmpiW(wszRoot, L"HKEY_USERS") || !xstrcmpiW(wszRoot, L"HKU"))
  {
    *hKeyRoot=HKEY_USERS;
    if (wszKeyRoot) xstrcpyW(wszKeyRoot, L"HKEY_USERS");
  }
  else if (!xstrcmpiW(wszRoot, L"HKEY_PERFORMANCE_DATA") || !xstrcmpiW(wszRoot, L"HKPD"))
  {
    *hKeyRoot=HKEY_PERFORMANCE_DATA;
    if (wszKeyRoot) xstrcpyW(wszKeyRoot, L"HKEY_PERFORMANCE_DATA");
  }
  else if (!xstrcmpiW(wszRoot, L"HKEY_CURRENT_CONFIG") || !xstrcmpiW(wszRoot, L"HKCC"))
  {
    *hKeyRoot=HKEY_CURRENT_CONFIG;
    if (wszKeyRoot) xstrcpyW(wszKeyRoot, L"HKEY_CURRENT_CONFIG");
  }
  else if (!xstrcmpiW(wszRoot, L"HKEY_DYN_DATA") || !xstrcmpiW(wszRoot, L"HKDD"))
  {
    *hKeyRoot=HKEY_DYN_DATA;
    if (wszKeyRoot) xstrcpyW(wszKeyRoot, L"HKEY_DYN_DATA");
  }
  else
  {
    *hKeyRoot=0;
    return 0;
  }
  return nKeyLen;
}

int TypeNameToTypeValue(wchar_t *pName)
{
  if (!xstrcmpiW(pName, L"REG_BINARY")) return REG_BINARY;
  else if (!xstrcmpiW(pName, L"REG_DWORD")) return REG_DWORD;
  else if (!xstrcmpiW(pName, L"REG_DWORD_BIG_ENDIAN")) return REG_DWORD_BIG_ENDIAN;
  else if (!xstrcmpiW(pName, L"REG_EXPAND_SZ")) return REG_EXPAND_SZ;
  else if (!xstrcmpiW(pName, L"REG_MULTI_SZ")) return REG_MULTI_SZ;
  else if (!xstrcmpiW(pName, L"REG_NONE")) return REG_NONE;
  else if (!xstrcmpiW(pName, L"REG_SZ")) return REG_SZ;
  else if (!xstrcmpiW(pName, L"REG_LINK")) return REG_LINK;
  else if (!xstrcmpiW(pName, L"REG_RESOURCE_LIST")) return REG_RESOURCE_LIST;
  else if (!xstrcmpiW(pName, L"REG_FULL_RESOURCE_DESCRIPTOR")) return REG_FULL_RESOURCE_DESCRIPTOR;
  else if (!xstrcmpiW(pName, L"REG_RESOURCE_REQUIREMENTS_LIST")) return REG_RESOURCE_REQUIREMENTS_LIST;
  else if (!xstrcmpiW(pName, L"REG_QWORD")) return REG_QWORD;
  else return -1;
}

wchar_t* TypeValueToTypeName(DWORD dwType)
{
  if (dwType == REG_BINARY) return L"REG_BINARY";
  else if (dwType == REG_DWORD) return L"REG_DWORD";
  else if (dwType == REG_DWORD_BIG_ENDIAN) return L"REG_DWORD_BIG_ENDIAN";
  else if (dwType == REG_EXPAND_SZ) return L"REG_EXPAND_SZ";
  else if (dwType == REG_MULTI_SZ) return L"REG_MULTI_SZ";
  else if (dwType == REG_NONE) return L"REG_NONE";
  else if (dwType == REG_SZ) return L"REG_SZ";
  else if (dwType == REG_LINK) return L"REG_LINK";
  else if (dwType == REG_RESOURCE_LIST) return L"REG_RESOURCE_LIST";
  else if (dwType == REG_FULL_RESOURCE_DESCRIPTOR) return L"REG_FULL_RESOURCE_DESCRIPTOR";
  else if (dwType == REG_RESOURCE_REQUIREMENTS_LIST) return L"REG_RESOURCE_REQUIREMENTS_LIST";
  else if (dwType == REG_QWORD) return L"REG_QWORD";
  else return L"INVALID";
}

// Function: Converts registry data to string
DWORD RegReadType(wchar_t *wszOutput, DWORD dwOutputMax, wchar_t **wpType, DWORD dwType, const unsigned char *szString, DWORD dwStringSize)
{
  DWORD dwOutputLen=0;

  wszOutput[0]=L'\0';

  if (dwType == REG_BINARY ||
      dwType == REG_NONE ||
      dwType == REG_LINK ||
      dwType == REG_RESOURCE_REQUIREMENTS_LIST ||
      dwType == REG_RESOURCE_LIST ||
      dwType == REG_FULL_RESOURCE_DESCRIPTOR ||
      dwType == REG_QWORD)
  {
    dwOutputLen=(DWORD)bin2hexW(szString, dwStringSize, wszOutput, dwOutputMax, TRUE);
  }
  else if (dwType == REG_SZ ||
           dwType == REG_EXPAND_SZ)
  {
    dwOutputLen=(DWORD)xstrcpynW(wszOutput, (wchar_t *)szString, dwOutputMax);
  }
  else if (dwType == REG_MULTI_SZ)
  {
    const wchar_t *wpInputCount;
    const wchar_t *wpInputMax=(const wchar_t *)(szString + dwStringSize);
    wchar_t *wpOutputCount=wszOutput;

    if (*(wpInputMax - 1) == L'\0')
      --wpInputMax;
    if (*(wpInputMax - 1) == L'\0')
      --wpInputMax;

    for (wpInputCount=(const wchar_t *)szString; wpInputCount < wpInputMax; ++wpInputCount)
    {
      if (*wpInputCount == L'\0')
        *wpOutputCount++=L'\n';
      else
        *wpOutputCount++=*wpInputCount;
    }
    *wpOutputCount=L'\0';
    dwOutputLen=(DWORD)(wpOutputCount - wszOutput);
  }
  else if (dwType == REG_DWORD)
  {
    dwOutputLen=(DWORD)xprintfW(wszOutput, L"%d", *((unsigned int *)szString));
  }
  else if (dwType == REG_DWORD_BIG_ENDIAN)
  {
    dwOutputLen=(DWORD)xprintfW(wszOutput, L"%d", SWAP_ENDIAN(*((unsigned int *)szString)));
  }

  *wpType=TypeValueToTypeName(dwType);
  return dwOutputLen;
}

LONG RegDeleteSubKeys(HKEY hKey, const wchar_t *lpSubKey)
{
  wchar_t wszKeyTmp[MAX_PATHLEN];
  HKEY hSubKey;
  DWORD dwKeyLen=MAX_PATHLEN;
  DWORD dwIndex=0;
  int nError;

  if ((nError=API_RegOpenKeyEx(hKey, lpSubKey, 0, KEY_ENUMERATE_SUB_KEYS, &hSubKey)) == ERROR_SUCCESS)
  {
    while (API_RegEnumKeyEx(hSubKey, dwIndex, wszKeyTmp, &dwKeyLen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
    {
      if (RegDeleteSubKeys(hSubKey, wszKeyTmp) != ERROR_SUCCESS)
        ++dwIndex;
      dwKeyLen=MAX_PATHLEN;
    }
    API_RegCloseKey(hSubKey);
    nError=API_RegDeleteKey(hKey, lpSubKey);
  }
  return nError;
}

// Function: Tries to delete registry key in one step (Win9x/Me),
// if error then calls RegDeleteSubKeys
int RegDeleteKeyPath(HKEY hKeyRoot, wchar_t *wszPath)
{
  wchar_t *wpKeyName=wszPath + xstrlenW(wszPath) - 1;
  HKEY hKeyHandle;
  int nError=-1;

  while (wpKeyName >= wszPath && *wpKeyName != L'\\')
    --wpKeyName;

  if (wpKeyName < wszPath) return -1;
  *wpKeyName++=L'\0';

  if (API_RegOpenKeyEx(hKeyRoot, wszPath, 0, KEY_ENUMERATE_SUB_KEYS, &hKeyHandle) == ERROR_SUCCESS)
  {
    if ((nError=API_RegDeleteKey(hKeyHandle, wpKeyName)) != ERROR_SUCCESS)
    {
      nError=RegDeleteSubKeys(hKeyHandle, wpKeyName);
    }
    API_RegCloseKey(hKeyHandle);
  }
  return (nError == ERROR_SUCCESS)?0:-1;
}

// Function: Recursively copies registry key
LONG RegCopyKey(HKEY hSource, wchar_t *wszPathSource, HKEY hTarget, wchar_t *wszPathTarget)
{
  //Global variables: wszKey, wszValue, szString

  DWORD dwKeyLen;
  DWORD dwValueLen;
  DWORD dwStringSize;
  DWORD dwType;
  DWORD dwIndex;
  DWORD dwDisposition;
  HKEY hSourceHandle;
  HKEY hTargetHandle;
  int nError;

  if ((nError=API_RegOpenKeyEx(hSource, wszPathSource, 0, KEY_READ, &hSourceHandle)) != ERROR_SUCCESS ||
      (nError=API_RegCreateKeyEx(hTarget, wszPathTarget, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hTargetHandle, &dwDisposition)) != ERROR_SUCCESS)
  {
    return nError;
  }

  dwIndex=0;
  for (;;)
  {
    dwValueLen=MAX_PATHLEN;
    dwStringSize=MAX_DATALEN;
    szString[0]='\0';
    if (API_RegEnumValue(hSourceHandle, dwIndex++, wszValue, &dwValueLen, NULL, &dwType, szString, &dwStringSize) != ERROR_SUCCESS)
      break;

    API_RegSetValueEx(hTargetHandle, wszValue, 0, dwType, szString, dwStringSize);
  }

  dwIndex=0;
  for (;;)
  {
    dwKeyLen=MAX_PATHLEN;
    if (API_RegEnumKeyEx(hSourceHandle, dwIndex++, wszKey, &dwKeyLen, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
    {
      API_RegCloseKey(hSourceHandle);
      API_RegCloseKey(hTargetHandle);
      return ERROR_SUCCESS;
    }

    RegCopyKey(hSourceHandle, wszKey, hTargetHandle, wszKey);
  }
}

// Function: Converts escape sequences (\\ -> \\\\), (\" -> \\\"), (\r -> \\r), (\n -> \\n)
void ReplaceEscapeSequence(wchar_t *wpResult, wchar_t *wpText)
{
  int a=0;
  int b=0;

  for (; wpText[a]; ++a, ++b)
  {
    if (wpText[a] == L'\\')
      wpResult[b]=L'\\', wpResult[++b]=L'\\';
    else if (wpText[a] == L'\"')
      wpResult[b]=L'\\', wpResult[++b]=L'\"';
    else if (wpText[a] == L'\r')
      wpResult[b]=L'\\', wpResult[++b]=L'r';
    else if (wpText[a] == L'\n')
      wpResult[b]=L'\\', wpResult[++b]=L'n';
    else wpResult[b]=wpText[a];
  }
  wpResult[b]=L'\0';
}

BOOL WriteRegFile(BOOL bUnicode, HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
  if (bUnicode)
    return WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);

  //Ansi
  nNumberOfBytesToWrite=WideCharToMultiByte(CP_ACP, 0, (const wchar_t *)lpBuffer, nNumberOfBytesToWrite / sizeof(wchar_t), (char *)wszBuf2, MAX_DATALEN * sizeof(wchar_t), NULL, NULL);
  return WriteFile(hFile, (char *)wszBuf2, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}

int GetOptionsW(wchar_t *wpLine, wchar_t *wpOption, BOOL bSensitive, wchar_t *wszResult, int nMaxResult)
{
  wchar_t *wpLineStart;
  wchar_t *wpLineEnd;
  wchar_t *wpOptionCount;
  wchar_t *wpOptionString=NULL;
  wchar_t wchQuote='\0';
  wchar_t wchDelimiter=*wpOption++;
  int nResult=0;

  for (wpLineStart=wpLineEnd=wpLine; *wpLineStart && *wpLineEnd; ++wpLineStart)
  {
    if (wchQuote == L'\0' && *wpLineStart != L'\"' && *wpLineStart != L'\'' && *wpLineStart != L'`')
    {
      if (*wpLineStart == wchDelimiter)
      {
        if (wpOptionString) break;

        for (wpLineEnd=wpLineStart + 1, wpOptionCount=wpOption;
             *wpLineEnd && (*wpLineEnd == *wpOptionCount ||
                            #if defined WideCharLower_INCLUDED
                              (!bSensitive && WideCharLower(*wpLineEnd) == WideCharLower(*wpOptionCount)));
                            #elif defined WideCharUpper_INCLUDED
                              (!bSensitive && WideCharUpper(*wpLineEnd) == WideCharUpper(*wpOptionCount)));
                            #else
                              #pragma message ("NOTE: WideCharLower and WideCharUpper undefined - xstrrepW will not work on Win95/98/Me.")
                              (!bSensitive && (UINT_PTR)CharUpperW((wchar_t *)(UINT_PTR)(WORD)*wpLineEnd) == (UINT_PTR)CharUpperW((wchar_t *)(UINT_PTR)(WORD)*wpOptionCount)));
                            #endif
             ++wpLineEnd)
        {
          if (!*++wpOptionCount)
          {
            wpLineStart=wpLineEnd;
            wpOptionString=wpLineEnd + 1;
            break;
          }
        }
      }
    }
    else if (wchQuote == L'\0')
      wchQuote=*wpLineStart;
    else if (*wpLineStart == wchQuote)
      wchQuote=L'\0';
  }
  if (wpOptionString)
  {
    while (*--wpLineStart == L' ' || *wpLineStart == L'\t');

    if (*wpOptionString == *wpLineStart && (*wpOptionString == L'\"' || *wpOptionString == L'\'' || *wpOptionString == L'`'))
    {
      ++wpOptionString, --wpLineStart;
    }
    nResult=(int)(wpLineStart - wpOptionString + 2);
    if (wszResult)
    {
      if (nMaxResult < nResult) nResult=nMaxResult;
      nResult=(int)xstrcpynW(wszResult, wpOptionString, nResult);
    }
  }
  return nResult;
}

//Standard registry API calls
LONG API_RegCreateKeyEx(HKEY hKey, const wchar_t *lpSubKey, DWORD Reserved, wchar_t *lpClass, DWORD dwOptions, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition)
{
  LONG nError;

  #ifndef POCKETPC
    nError=RegCreateKeyExWide(hKey, lpSubKey, Reserved, NULL, dwOptions, samDesired|g_pluginParms->exec_flags->alter_reg_view, lpSecurityAttributes, phkResult, lpdwDisposition);
  #else
    nError=CeRegCreateKeyEx(hKey, lpSubKey, Reserved, L"", dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);
  #endif
  return nError;
}

LONG API_RegOpenKeyEx(HKEY hKey, const wchar_t *lpSubKey, DWORD ulOptions, REGSAM c, PHKEY phkResult)
{
  LONG nError;

  #ifndef POCKETPC
    nError=RegOpenKeyExWide(hKey, lpSubKey, ulOptions, c|g_pluginParms->exec_flags->alter_reg_view, phkResult);
  #else
    nError=CeRegOpenKeyEx(hKey, lpSubKey, ulOptions, c, phkResult);
  #endif
  return nError;
}

LONG API_RegEnumKeyEx(HKEY hKey, DWORD dwIndex, wchar_t *lpName, LPDWORD lpcName, LPDWORD lpReserved, wchar_t *lpClass, LPDWORD lpcClass, PFILETIME lpftLastWriteTime)
{
  LONG nError;

  #ifndef POCKETPC
    nError=RegEnumKeyExWide(hKey, dwIndex, lpName, lpcName, lpReserved, lpClass, lpcClass, lpftLastWriteTime);
  #else
    nError=CeRegEnumKeyEx(hKey, dwIndex, lpName, lpcName, lpReserved, lpClass, lpcClass, lpftLastWriteTime);
  #endif
  return nError;
}

LONG API_RegCloseKey(HKEY hKey)
{
  LONG nError;

  #ifndef POCKETPC
    nError=RegCloseKey(hKey);
  #else
    nError=CeRegCloseKey(hKey);
  #endif
  return nError;
}

LONG API_RegDeleteKey(HKEY hKey, const wchar_t *lpSubKey)
{
  LONG nError;

  #ifndef POCKETPC
    nError=RegDeleteKeyWide(hKey, lpSubKey);
  #else
    nError=CeRegDeleteKey(hKey, lpSubKey);
  #endif
  return nError;
}

LONG API_RegEnumValue(HKEY hKey, DWORD dwIndex, wchar_t *lpValueName, LPDWORD lpcValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
  LONG nError;

  #ifndef POCKETPC
    nError=RegEnumValueWide(hKey, dwIndex, lpValueName, lpcValueName, lpReserved, lpType, lpData, lpcbData);
  #else
    nError=CeRegEnumValue(hKey, dwIndex, lpValueName, lpcValueName, lpReserved, lpType, lpData, lpcbData);
  #endif
  return nError;
}

LONG API_RegDeleteValue(HKEY hKey, const wchar_t *lpValueName)
{
  LONG nError;

  #ifndef POCKETPC
    nError=RegDeleteValueWide(hKey, lpValueName);
  #else
    nError=CeRegDeleteValue(hKey, lpValueName);
  #endif
  return nError;
}

LONG API_RegQueryValueEx(HKEY hKey, const wchar_t *lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
  LONG nError;

  #ifndef POCKETPC
    nError=RegQueryValueExWide(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
  #else
    nError=CeRegQueryValueEx(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
  #endif
  return nError;
}

LONG API_RegSetValueEx(HKEY hKey, const wchar_t *lpValueName, DWORD Reserved, DWORD dwType, BYTE* lpData, DWORD cbData)
{
  LONG nError;

  #ifndef POCKETPC
    nError=RegSetValueExWide(hKey, lpValueName, Reserved, dwType, lpData, cbData);
  #else
    nError=CeRegSetValueEx(hKey, lpValueName, Reserved, dwType, lpData, cbData);
  #endif
  return nError;
}

INT_PTR popintegerWide()
{
  wchar_t wszInt[32];

  wszInt[0]=L'\0';
  popstringWide(wszInt, 31);
  return xatoiW(wszInt, NULL);
}

void pushintegerWide(INT_PTR integer)
{
  wchar_t wszInt[32];

  xitoaW(integer, wszInt);
  pushstringWide(wszInt);
}

int popstringAnsi(char *str, int len)
{
  stack_t *th;

  if (g_stacktop && *g_stacktop)
  {
    th=(*g_stacktop);
    if (g_unicode)
    {
      if (len=WideCharToMultiByte(CP_ACP, 0, (const wchar_t *)th->text, -1, str, len, NULL, NULL))
        str[--len]='\0';
    }
    else
    {
      len=(int)xstrcpynA(str, (const char *)th->text, len);
    }
    *g_stacktop=th->next;
    GlobalFree((HGLOBAL)th);
    return len;
  }
  return -1;
}

int popstringWide(wchar_t *str, int len)
{
  stack_t *th;

  if (g_stacktop && *g_stacktop)
  {
    th=(*g_stacktop);
    if (g_unicode)
    {
      len=(int)xstrcpynW(str, th->text, len);
    }
    else
    {
      if (len=MultiByteToWideChar(CP_ACP, 0, (const char *)th->text, -1, str, len))
        str[--len]=L'\0';
    }
    *g_stacktop=th->next;
    GlobalFree((HGLOBAL)th);
    return len;
  }
  return -1;
}

void pushstringAnsi(const char *str)
{
  stack_t *th;

  if (g_stacktop)
  {
    if (g_unicode)
    {
      th=(stack_t *)GlobalAlloc(GPTR, sizeof(stack_t) + NSIS_MAX_STRLEN * sizeof(wchar_t));
      MultiByteToWideChar(CP_ACP, 0, str, -1, (wchar_t *)th->text, NSIS_MAX_STRLEN);
    }
    else
    {
      th=(stack_t *)GlobalAlloc(GPTR, sizeof(stack_t) + NSIS_MAX_STRLEN);
      xstrcpynA((char *)th->text, str, NSIS_MAX_STRLEN);
    }
    th->next=*g_stacktop;
    *g_stacktop=th;
  }
}

void pushstringWide(const wchar_t *str)
{
  stack_t *th;

  if (g_stacktop)
  {
    if (g_unicode)
    {
      th=(stack_t *)GlobalAlloc(GPTR, sizeof(stack_t) + NSIS_MAX_STRLEN * sizeof(wchar_t));
      xstrcpynW(th->text, str, NSIS_MAX_STRLEN);
    }
    else
    {
      th=(stack_t *)GlobalAlloc(GPTR, sizeof(stack_t) + NSIS_MAX_STRLEN);
      WideCharToMultiByte(CP_ACP, 0, str, -1, (char *)th->text, NSIS_MAX_STRLEN, NULL, NULL);
    }
    th->next=*g_stacktop;
    *g_stacktop=th;
  }
}

void Initialize(int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  g_stacktop=stacktop;
  g_variables=variables;
  g_stringsize=string_size;
  g_pluginParms=extra;

  if (g_unicode == -1)
  {
    wchar_t wszPath[]=L"C:\\";

    g_pluginParms->validate_filename(wszPath);
    g_unicode=(wszPath[2] == L'\\')?FALSE:TRUE;

    //Initialize WideFunc.h header
    WideInitialize();
    WideGlobal_bOldWindows=!g_unicode;
  }
}
