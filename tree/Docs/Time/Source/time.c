/*****************************************************************
 *                 Time NSIS plugin v2.0                         *
 *                                                               *
 * 2011 Shengalts Aleksander aka Instructor (Shengalts@mail.ru)  *
 *****************************************************************/


//Comment-out and recompile
#define GETLOCALTIME    //Compile with GetLocalTime/GetLocalTimeUTC function
#define SETLOCALTIME    //Compile with SetLocalTime/SetLocalTimeUTC function
#define GETFILETIME     //Compile with GetFileTime/GetFileTimeUTC function
#define SETFILETIME     //Compile with SetFileTime/SetFileTimeUTC function
#define TIMESTRING      //Compile with TimeString function
#define MATHTIME        //Compile with MathTime function



#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "StrFunc.h"
#include "WideFunc.h"

//Include string functions
#define WideCharLower
#define xmemcpy
#define xstrcpynA
#define xstrcpynW
#define xstrcmpiW
#define xstrlenA
#define xstrlenW
#define xatoiW
#define xitoaW
#define xatoi64W
#define xi64toaW
#define xuitoaW
#define dec2hexW
#define xprintfW
#include "StrFunc.h"

//Include wide functions
#define FindFirstFileWide
#define CreateFileWide
#include "WideFunc.h"

//Defines
#define NSIS_MAX_STRLEN  1024
#define UNIT_UNKNOWN     -1
#define UNIT_SECOND      0
#define UNIT_MINUTE      1
#define UNIT_HOUR        2
#define UNIT_DAY         3
#define UNIT_MONTH       4
#define UNIT_YEAR        5
#define DATE_STRUCTURE   6

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
wchar_t wszBuf[NSIS_MAX_STRLEN];
BOOL bUTC=FALSE;

#ifdef MATHTIME
  typedef struct TIME64 {
    __int64 nDay;
    __int64 nMonth;
    __int64 nYear;
    __int64 nHour;
    __int64 nMinute;
    __int64 nSecond;
  } TIME64;

  TIME64 lpTime64a;
  TIME64 lpTime64b;
#endif

//Funtions prototypes and macros 
#if defined SETLOCALTIME || defined SETFILETIME || defined TIMESTRING
  BOOL ParseDateString(const wchar_t *wpString, SYSTEMTIME *lpSystemTime);
#endif
#ifdef GETFILETIME
  wchar_t* FileTimeToString(FILETIME *lpFileTime, wchar_t *wszString, BOOL bUTC);
#endif
#ifdef SETFILETIME
  BOOL ParseFileTimeString(const wchar_t *wpString, FILETIME *lpFileTime, BOOL bUTC);
#endif
#ifdef MATHTIME
  BOOL ParseDateString64(const wchar_t **wpString, TIME64 *lpTime64);
  BOOL NormalizeStructure(TIME64 *lpTime64);
  __int64 StructureToUnit(int nUnit, TIME64 lpTime64);
  void UnitToStructure(int nUnit, TIME64 *lpTime64, __int64 nValue);
  void MathStructures(TIME64 *lpTime64a, TIME64 lpTime64b, wchar_t wchOperator);
  void MathUnits(__int64 *nValueResult, __int64 nValue, wchar_t wchOperator);
  BOOL GetConvertionUnit(const wchar_t **wpString, int *nUnit);
#endif

int popintegerWide();
void pushintegerWide(int integer);
int popstringAnsi(char *str, int len);
int popstringWide(wchar_t *str, int len);
void pushstringAnsi(const char *str);
void pushstringWide(const wchar_t *str);
void Initialize(int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra);

//NSIS functions code 
#ifdef GETLOCALTIME
void __declspec(dllexport) _GetLocalTime(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    SYSTEMTIME lpSystemTime={0};
  
    if (bUTC == FALSE)
      GetLocalTime(&lpSystemTime);
    else
      GetSystemTime(&lpSystemTime);
  
    xprintfW(wszBuf, L"%02d.%02d.%d %02d:%02d:%02d", lpSystemTime.wDay, lpSystemTime.wMonth, lpSystemTime.wYear, lpSystemTime.wHour, lpSystemTime.wMinute, lpSystemTime.wSecond);
    pushstringWide(wszBuf);
  }
}

void __declspec(dllexport) _GetLocalTimeUTC(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  bUTC=TRUE;
  _GetLocalTime(hwndParent, string_size, variables, stacktop, extra);
  bUTC=FALSE;
}
#endif //GETLOCALTIME

#ifdef SETLOCALTIME
void __declspec(dllexport) _SetLocalTime(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    SYSTEMTIME lpSystemTime={0};
  
    popstringWide(wszBuf, NSIS_MAX_STRLEN);
  
    if (ParseDateString(wszBuf, &lpSystemTime))
    {
      if (bUTC == FALSE)
      {
        SetLocalTime(&lpSystemTime);
        SetLocalTime(&lpSystemTime);
      }
      else
      {
        SetSystemTime(&lpSystemTime);
        SetSystemTime(&lpSystemTime);
      }
      SendMessage(HWND_TOPMOST, WM_TIMECHANGE, 0, 0);
      pushstringWide(L"0");
    }
    else
      pushstringWide(L"-1");
  }
}

void __declspec(dllexport) _SetLocalTimeUTC(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  bUTC=TRUE;
  _SetLocalTime(hwndParent, string_size, variables, stacktop, extra);
  bUTC=FALSE;
}
#endif //SETLOCALTIME

#ifdef GETFILETIME
void __declspec(dllexport) _GetFileTime(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    WIN32_FIND_DATAW wfd={0};
    HANDLE hFind=NULL;
    int i;
  
    popstringWide(wszBuf, NSIS_MAX_STRLEN);
  
    if ((hFind=FindFirstFileWide(wszBuf, &wfd)) != INVALID_HANDLE_VALUE && FindClose(hFind))
    {
      pushstringWide(FileTimeToString(&wfd.ftLastAccessTime, wszBuf, bUTC));
      pushstringWide(FileTimeToString(&wfd.ftLastWriteTime, wszBuf, bUTC));
      pushstringWide(FileTimeToString(&wfd.ftCreationTime, wszBuf, bUTC));
      return;
    }
    for(i=0; i < 3; ++i) pushstringWide(L"");
  }
}

void __declspec(dllexport) _GetFileTimeUTC(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  bUTC=TRUE;
  _GetFileTime(hwndParent, string_size, variables, stacktop, extra);
  bUTC=FALSE;
}
#endif //GETFILETIME

#ifdef SETFILETIME
void __declspec(dllexport) _SetFileTime(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    wchar_t wszCreation[128];
    wchar_t wszWrite[128];
    wchar_t wszAccess[128];
    FILETIME ftCreationTime={0};
    FILETIME ftLastWriteTime={0};
    FILETIME ftLastAccessTime={0};
    HANDLE hFind=NULL;
  
    popstringWide(wszBuf, NSIS_MAX_STRLEN);
    popstringWide(wszCreation, NSIS_MAX_STRLEN);
    popstringWide(wszWrite, NSIS_MAX_STRLEN);
    popstringWide(wszAccess, NSIS_MAX_STRLEN);
  
    if ((hFind=CreateFileWide(wszBuf, FILE_WRITE_ATTRIBUTES, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL)) != INVALID_HANDLE_VALUE)
    {
      if (!ParseFileTimeString(wszCreation, &ftCreationTime, bUTC) ||
          !ParseFileTimeString(wszWrite, &ftLastWriteTime, bUTC) ||
          !ParseFileTimeString(wszAccess, &ftLastAccessTime, bUTC) ||
          !SetFileTime(hFind, (*wszCreation)?&ftCreationTime:NULL, (*wszAccess)?&ftLastAccessTime:NULL, (*wszWrite)?&ftLastWriteTime:NULL))
        goto error;
  
      CloseHandle(hFind);
      pushstringWide(L"0");
      return;
    }
  
    error:
    if (hFind != INVALID_HANDLE_VALUE) CloseHandle(hFind);
    pushstringWide(L"-1");
  }
}

void __declspec(dllexport) _SetFileTimeUTC(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  bUTC=TRUE;
  _SetFileTime(hwndParent, string_size, variables, stacktop, extra);
  bUTC=FALSE;
}
#endif //SETFILETIME

#ifdef TIMESTRING
void __declspec(dllexport) _TimeString(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    SYSTEMTIME lpSystemTime={0};
    int i;
  
    popstringWide(wszBuf, NSIS_MAX_STRLEN);
  
    if (ParseDateString(wszBuf, &lpSystemTime))
    {
      int lpTime[]={lpSystemTime.wSecond, lpSystemTime.wMinute, lpSystemTime.wHour, lpSystemTime.wYear, lpSystemTime.wMonth, lpSystemTime.wDay};

      for (i=0; i < 6; ++i)
      {
        xitoaW(lpTime[i], wszBuf);
        pushstringWide(wszBuf);
      }
    }
    else
      for(i=0; i < 6; ++i) pushstringWide(L"");
  }
}
#endif //TIMESTRING

#ifdef MATHTIME
void __declspec(dllexport) _MathTime(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    __int64 nResultTmp=0;
    __int64 nResultSum=0;
    const wchar_t *wpBuf=&wszBuf[0];
    wchar_t wchCurrentOperator='\0';
    wchar_t wchPreviousOperator='\0';
    int nUnit=UNIT_UNKNOWN;
    int nUnitTmp=UNIT_UNKNOWN;
    BOOL bNegative=FALSE;
  
    popstringWide(wszBuf, NSIS_MAX_STRLEN);
  
    while (*wpBuf == L' ') ++wpBuf;
  
    for (;;)
    {
      if (GetConvertionUnit(&wpBuf, &nUnitTmp))
      {
        if (nUnit != UNIT_UNKNOWN && nUnitTmp != nUnit) goto error;
        if (!ParseDateString64(&wpBuf, &lpTime64b)) goto error;
        nUnit=nUnitTmp;
        if (nUnit != DATE_STRUCTURE)
        {
          if (!NormalizeStructure(&lpTime64b)) goto error;
          nResultTmp=StructureToUnit(nUnit, lpTime64b);
        }
      }
      else if (nUnit != DATE_STRUCTURE)
      {
        while (*wpBuf == L' ') ++wpBuf;
  
        if ((*wpBuf >= L'0' && *wpBuf <= L'9') || *wpBuf == L'-')
        {
          nResultTmp=xatoi64W(wpBuf, NULL);
  
          while (*++wpBuf &&
            *wpBuf != L'=' &&
            *wpBuf != L'+' &&
            *wpBuf != L'-' &&
            *wpBuf != L'*' &&
            *wpBuf != L'/' &&
            *wpBuf != L'%');
        }
        else goto error;
      }
      else goto error;
  
      wchCurrentOperator=*wpBuf;
      ++wpBuf;
  
      if (nUnit != DATE_STRUCTURE)
        MathUnits(&nResultSum, nResultTmp, wchPreviousOperator);
      else
        MathStructures(&lpTime64a, lpTime64b, wchPreviousOperator);
  
      if (wchCurrentOperator == L'=')
      {
        while (*wpBuf == L' ') ++wpBuf;
  
        if (*wpBuf != L'\0')
        {
          if (nUnit == UNIT_UNKNOWN) goto error;
          if (!GetConvertionUnit(&wpBuf, &nUnitTmp)) goto error;
  
          if (nUnitTmp == nUnit && nUnit == DATE_STRUCTURE)
          {
            if (!NormalizeStructure(&lpTime64a)) goto error;
          }
          else if (nUnitTmp != nUnit && nUnit == DATE_STRUCTURE)
          {
            if (!NormalizeStructure(&lpTime64a)) goto error;
            nResultSum=StructureToUnit(nUnitTmp, lpTime64a);
          }
          else if (nUnitTmp != nUnit && nUnitTmp == DATE_STRUCTURE)
          {
            UnitToStructure(nUnit, &lpTime64a, nResultSum);
            if (!NormalizeStructure(&lpTime64a)) goto error;
          }
          else if (nUnitTmp != nUnit)
          {
            if (nResultSum < 0)
            {
              nResultSum=0 - nResultSum;
              bNegative=TRUE;
            }
            UnitToStructure(nUnit, &lpTime64a, nResultSum);
            if (!NormalizeStructure(&lpTime64a)) goto error;
            nResultSum=StructureToUnit(nUnitTmp, lpTime64a);
            if (bNegative == TRUE) nResultSum=0 - nResultSum;
          }
          nUnit=nUnitTmp;
        }
      }
  
      wchPreviousOperator=wchCurrentOperator;
      if (wchCurrentOperator == L'\0') goto error;
      if (wchCurrentOperator == L'=') break;
    }
  
    if (nUnit != DATE_STRUCTURE)
    {
      xi64toaW(nResultSum, wszBuf);
    }
    else
    {
      xprintfW(wszBuf, L"%02d.%02d.%d %02d:%02d:%02d", (int)lpTime64a.nDay, (int)lpTime64a.nMonth, (int)lpTime64a.nYear, (int)lpTime64a.nHour, (int)lpTime64a.nMinute, (int)lpTime64a.nSecond);
    }
  
    pushstringWide(wszBuf);
    return;
  
    error:
    pushstringWide(L"");
  }
}
#endif //MATHTIME

void __declspec(dllexport) _Unload(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
  return TRUE;
}


#if defined SETLOCALTIME || defined SETFILETIME || defined TIMESTRING
//Function: Fills SYSTEMTIME structure with the values from date string "31.12.2005 23:59:59"
BOOL ParseDateString(const wchar_t *wpString, SYSTEMTIME *lpSystemTime)
{
  int a=0;
  wchar_t wchDelimiter=L'\0';

  while (*wpString)
  {
    if (*wpString == L'0')
    {
      while (*++wpString == L'0');
      --wpString;
    }

    if (*wpString >= L'0' && *wpString <= L'9')
    {
      if (++a == 1)
        lpSystemTime->wDay=(WORD)xatoiW(wpString, &wpString);
      else if (a == 2 && wchDelimiter == L'.')
        lpSystemTime->wMonth=(WORD)xatoiW(wpString, &wpString);
      else if (a == 3 && wchDelimiter == L'.')
        lpSystemTime->wYear=(WORD)xatoiW(wpString, &wpString);
      else if (a == 4 && wchDelimiter == L' ')
        lpSystemTime->wHour=(WORD)xatoiW(wpString, &wpString);
      else if (a == 5 && wchDelimiter == L':')
        lpSystemTime->wMinute=(WORD)xatoiW(wpString, &wpString);
      else if (a == 6 && wchDelimiter == L':')
      {
        lpSystemTime->wSecond=(WORD)xatoiW(wpString, &wpString);
        return TRUE;
      }
      else return FALSE;
    }
    else return FALSE;

    wchDelimiter=*wpString++;

    while (a == 3 && *wpString == L' ')
      ++wpString;
  }
  return FALSE;
}
#endif //SETLOCALTIME || SETFILETIME || TIMESTRING

#ifdef GETFILETIME
wchar_t* FileTimeToString(FILETIME *lpFileTime, wchar_t *wszString, BOOL bUTC)
{
  SYSTEMTIME lpSystemTime={0};
  FILETIME lpLocalFileTime={0};

  if (bUTC == FALSE)
  {
    FileTimeToLocalFileTime(lpFileTime, &lpLocalFileTime);
    FileTimeToSystemTime(&lpLocalFileTime, &lpSystemTime);
  }
  else
    FileTimeToSystemTime(lpFileTime, &lpSystemTime);
  
  xprintfW(wszString, L"%02d.%02d.%d %02d:%02d:%02d", lpSystemTime.wDay, lpSystemTime.wMonth, lpSystemTime.wYear, lpSystemTime.wHour, lpSystemTime.wMinute, lpSystemTime.wSecond);
  return wszString;
}
#endif //GETFILETIME

#ifdef SETFILETIME
//Function: Fills FILETIME structure with the values from date string "31.12.2005 23:59:59"
BOOL ParseFileTimeString(const wchar_t *wpString, FILETIME *lpFileTime, BOOL bUTC)
{
  SYSTEMTIME lpSystemTime={0};
  FILETIME lpLocalFileTime={0};

  if (*wpString)
  {
    if (!ParseDateString(wpString, &lpSystemTime))
      return FALSE;
  
    if (bUTC == FALSE)
    {
      if (!SystemTimeToFileTime(&lpSystemTime, &lpLocalFileTime) ||
          !LocalFileTimeToFileTime(&lpLocalFileTime, lpFileTime))
         return FALSE;
    }
    else
    {
      if (!SystemTimeToFileTime(&lpSystemTime, lpFileTime))
         return FALSE;
    }
  }
  return TRUE;
}
#endif //SETFILETIME

#ifdef MATHTIME
//Function: Fills TIME64 structure with the values from the first date in math string
//          and change pointer to the next operator (+,-,*,/,%,=)
//          "(31.12.2005 23:59:59) - date(31.12.2000 23:59:59) = date" -> "- date(31.12.2000 23:59:59) = date"
BOOL ParseDateString64(const wchar_t **wpString, TIME64 *lpTime64)
{
  int a=0;
  wchar_t wchDelimiter=L'\0';
  const wchar_t *wpBufTmp=*wpString;

  if (*wpBufTmp == L'(') ++wpBufTmp;
  else return FALSE;

  while (*wpBufTmp)
  {
    if (*wpBufTmp == L'0')
    {
      while (*++wpBufTmp == L'0');
      --wpBufTmp;
    }

    if (*wpBufTmp >= L'0' && *wpBufTmp <= L'9')
    {
      if (++a == 1)
        lpTime64->nDay=xatoi64W(wpBufTmp, &wpBufTmp);
      else if (a == 2 && wchDelimiter == L'.')
        lpTime64->nMonth=xatoi64W(wpBufTmp, &wpBufTmp);
      else if (a == 3 && wchDelimiter == L'.')
        lpTime64->nYear=xatoi64W(wpBufTmp, &wpBufTmp);
      else if (a == 4 && wchDelimiter == L' ')
        lpTime64->nHour=xatoi64W(wpBufTmp, &wpBufTmp);
      else if (a == 5 && wchDelimiter == L':')
        lpTime64->nMinute=xatoi64W(wpBufTmp, &wpBufTmp);
      else if (a == 6 && wchDelimiter == L':')
      {
        lpTime64->nSecond=xatoi64W(wpBufTmp, &wpBufTmp);

        if (*wpBufTmp == L')')
        {
          while (*++wpBufTmp &&
            *wpBufTmp != L'=' &&
            *wpBufTmp != L'+' &&
            *wpBufTmp != L'-' &&
            *wpBufTmp != L'*' &&
            *wpBufTmp != L'/' &&
            *wpBufTmp != L'%');

          *wpString=wpBufTmp;

          return TRUE;
        }
        return FALSE;
      }
      else return FALSE;
    }
    else return FALSE;

    wchDelimiter=*wpBufTmp++;

    while (a == 3 && *wpBufTmp == L' ')
      ++wpBufTmp;
  }
  return FALSE;
}

//Function: Get date unit of the first date in math string and
//          change pointer to the L'(' or L'\0' char
//          "date(31.12.2005 23:59:59)" -> "(31.12.2005 23:59:59)"
BOOL GetConvertionUnit(const wchar_t **wpString, int *nUnit)
{
  wchar_t szUnit[16];
  const wchar_t *wpStartUnitWord=NULL;
  const wchar_t *wpBuf=*wpString;
  *nUnit=UNIT_UNKNOWN;

  while (*wpBuf == L' ') ++wpBuf;

  wpStartUnitWord=wpBuf;

  while (*wpBuf &&
    *wpBuf != L'(' &&
    *wpBuf != L')' &&
    *wpBuf != L' ' &&
    *wpBuf != L'=' &&
    *wpBuf != L'+' &&
    *wpBuf != L'-' &&
    *wpBuf != L'*' &&
    *wpBuf != L'/' &&
    *wpBuf != L'%')
  {
    ++wpBuf;
  }

  if (!*wpBuf || *wpBuf == L'(')
  {
    xstrcpynW(szUnit, wpStartUnitWord, wpBuf - wpStartUnitWord + 1);

    if (!xstrcmpiW(szUnit, L"second"))
      *nUnit=UNIT_SECOND;
    else if (!xstrcmpiW(szUnit, L"minute"))
      *nUnit=UNIT_MINUTE;
    else if (!xstrcmpiW(szUnit, L"hour"))
      *nUnit=UNIT_HOUR;
    else if (!xstrcmpiW(szUnit, L"day"))
      *nUnit=UNIT_DAY;
    else if (!xstrcmpiW(szUnit, L"month"))
      *nUnit=UNIT_MONTH;
    else if (!xstrcmpiW(szUnit, L"year"))
      *nUnit=UNIT_YEAR;
    else if (!xstrcmpiW(szUnit, L"date"))
      *nUnit=DATE_STRUCTURE;
    else return FALSE;

    *wpString=wpBuf;
    return TRUE;
  }
  return FALSE;
}

//Function: Normalizes any irregular date structure
BOOL NormalizeStructure(TIME64 *lpTime64)
{
  __int64 nTmp=0;
  int a=0;
  int nYearCount=0;
  int days[12]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  //Converts negative values to positive
  year:
  for (;;)
  {
    if (lpTime64->nYear >= 0 && lpTime64->nMonth <= 0)
    {
      lpTime64->nMonth+=12;
      --lpTime64->nYear;
      continue;
    }
    else if (lpTime64->nYear < 0) return FALSE;
    else break;
  }

  month:
  for (;;)
  {
    if (lpTime64->nMonth > 0 && lpTime64->nDay <= 0)
    {
      if (lpTime64->nMonth == 1)
        lpTime64->nDay+=31;
      else
        lpTime64->nDay+=days[lpTime64->nMonth - 2];

      if (lpTime64->nMonth == 3 && (lpTime64->nYear == 0 || (lpTime64->nYear >= 4 && lpTime64->nYear % 4 == 0)))
        ++lpTime64->nDay;
      --lpTime64->nMonth;
      continue;
    }
    else if (lpTime64->nMonth <= 0) goto year;
    else break;
  }

  day:
  for (;;)
  {
    if (lpTime64->nDay > 0 && lpTime64->nHour < 0)
    {
      lpTime64->nHour+=24;
      --lpTime64->nDay;
      continue;
    }
    else if (lpTime64->nDay <= 0) goto month;
    else break;
  }

  hour:
  for (;;)
  {
    if (lpTime64->nHour >= 0 && lpTime64->nMinute < 0)
    {
      lpTime64->nMinute+=60;
      --lpTime64->nHour;
      continue;
    }
    else if (lpTime64->nHour < 0) goto day;
    else break;
  }

  for (;;)
  {
    if (lpTime64->nMinute >= 0 && lpTime64->nSecond < 0)
    {
      lpTime64->nSecond+=60;
      --lpTime64->nMinute;
      continue;
    }
    else if (lpTime64->nMinute < 0) goto hour;
    else break;
  }

  //Normalize positive values
  //Seconds
  nTmp=lpTime64->nSecond / 60;
  lpTime64->nSecond=lpTime64->nSecond % 60;

  //Minutes
  lpTime64->nMinute+=nTmp;
  nTmp=lpTime64->nMinute / 60;
  lpTime64->nMinute=lpTime64->nMinute % 60;

  //Hours
  lpTime64->nHour+=nTmp;
  nTmp=lpTime64->nHour / 24;
  lpTime64->nHour=lpTime64->nHour % 24;

  //Days
  lpTime64->nDay+=nTmp;

  //Normalize months before days processing
  if (lpTime64->nMonth > 12)
  {
    if (lpTime64->nMonth % 12 == 0)
    {
      nTmp=lpTime64->nMonth / 12 - 1;
      lpTime64->nMonth=12;
    }
    else
    {
      nTmp=lpTime64->nMonth / 12;
      lpTime64->nMonth=lpTime64->nMonth % 12;
    }
    lpTime64->nYear=lpTime64->nYear + nTmp;
  }

  if (lpTime64->nYear >= 4) nYearCount=(int)(lpTime64->nYear % 4);
  else nYearCount=(int)lpTime64->nYear;

  for (a=(int)(lpTime64->nMonth - 1);; a=0)
  {
    for (; a < 12; ++a)
    {
      nTmp=lpTime64->nDay - days[a];
      if (a == 1 && (nYearCount == 0 || (nYearCount >= 4 && nYearCount % 4 == 0)))
        --nTmp;

      if (nTmp > 0)
      {

        lpTime64->nDay=nTmp;
        ++lpTime64->nMonth;
      }
      else break;
    }
    if (nTmp > 0) ++nYearCount;
    else break;
  }

  //Months after days processing
  if (lpTime64->nMonth > 12)
  {
    if (lpTime64->nMonth % 12 == 0)
    {
      nTmp=lpTime64->nMonth / 12 - 1;
      lpTime64->nMonth=12;
    }
    else
    {
      nTmp=lpTime64->nMonth / 12;
      lpTime64->nMonth=lpTime64->nMonth % 12;
    }
  }
  else nTmp=0;

  //Years
  lpTime64->nYear=lpTime64->nYear + nTmp;

  return TRUE;
}

//Function: Converts normal date structure to the specified date unit
__int64 StructureToUnit(int nUnit, TIME64 lpTime64)
{
  __int64 nDaysBeforeYear=0;
  int nDaysBeforeMonth=0;

  if (nUnit == UNIT_SECOND || nUnit == UNIT_MINUTE || nUnit == UNIT_HOUR || nUnit == UNIT_DAY)
  {
    if (lpTime64.nMonth == 1) nDaysBeforeMonth=0;
    else if (lpTime64.nMonth == 2) nDaysBeforeMonth=31;
    else if (lpTime64.nMonth == 3) nDaysBeforeMonth=59;
    else if (lpTime64.nMonth == 4) nDaysBeforeMonth=90;
    else if (lpTime64.nMonth == 5) nDaysBeforeMonth=120;
    else if (lpTime64.nMonth == 6) nDaysBeforeMonth=151;
    else if (lpTime64.nMonth == 7) nDaysBeforeMonth=181;
    else if (lpTime64.nMonth == 8) nDaysBeforeMonth=212;
    else if (lpTime64.nMonth == 9) nDaysBeforeMonth=243;
    else if (lpTime64.nMonth == 10) nDaysBeforeMonth=273;
    else if (lpTime64.nMonth == 11) nDaysBeforeMonth=304;
    else if (lpTime64.nMonth == 12) nDaysBeforeMonth=334;

    if (lpTime64.nMonth > 2 && (lpTime64.nYear == 0 || (lpTime64.nYear >= 4 && lpTime64.nYear % 4 == 0)))
      ++nDaysBeforeMonth;

    if (lpTime64.nYear == 0)
      nDaysBeforeYear=0;
    else
      nDaysBeforeYear=365 * (lpTime64.nYear - ((lpTime64.nYear - 1) / 4 + 1)) + 366 * ((lpTime64.nYear - 1) / 4 + 1);
  }

  if (nUnit == UNIT_SECOND) return (lpTime64.nSecond + 60 * lpTime64.nMinute + 3600 * lpTime64.nHour + 86400 * (lpTime64.nDay + nDaysBeforeMonth - 1) + 86400 * nDaysBeforeYear);
  else if (nUnit == UNIT_MINUTE) return (lpTime64.nMinute + 60 * lpTime64.nHour + 1440 * (lpTime64.nDay + nDaysBeforeMonth - 1) + 1440 * nDaysBeforeYear);
  else if (nUnit == UNIT_HOUR) return (lpTime64.nHour + 24 * (lpTime64.nDay + nDaysBeforeMonth - 1) + 24*nDaysBeforeYear);
  else if (nUnit == UNIT_DAY) return ((lpTime64.nDay + nDaysBeforeMonth - 1) + nDaysBeforeYear);
  else if (nUnit == UNIT_MONTH) return (12 * lpTime64.nYear + lpTime64.nMonth - 1);
  else if (nUnit == UNIT_YEAR) return (lpTime64.nYear);

  return 0;
}

//Function: Converts specified date unit to the date structure
void UnitToStructure(int nUnit, TIME64 *lpTime64, __int64 nValue)
{
  lpTime64->nYear=0;
  lpTime64->nMonth=1;
  lpTime64->nDay=1;
  lpTime64->nHour=0;
  lpTime64->nMinute=0;
  lpTime64->nSecond=0;

  if (nUnit == UNIT_SECOND)
    lpTime64->nSecond=nValue;
  else if (nUnit == UNIT_MINUTE)
    lpTime64->nMinute=nValue;
  else if (nUnit == UNIT_HOUR)
    lpTime64->nHour=nValue;
  else if (nUnit == UNIT_DAY)
    lpTime64->nDay=nValue + 1;
  else if (nUnit == UNIT_MONTH)
    lpTime64->nMonth=nValue + 1;
  else if (nUnit == UNIT_YEAR)
    lpTime64->nYear=nValue;
}

//Function: Arithmetic operation with two structures
void MathStructures(TIME64 *lpTime64a, TIME64 lpTime64b, wchar_t wchOperator)
{
  if (wchOperator == L'+')
  {
    lpTime64a->nYear+=lpTime64b.nYear;
    lpTime64a->nMonth+=lpTime64b.nMonth;
    lpTime64a->nDay+=lpTime64b.nDay;
    lpTime64a->nHour+=lpTime64b.nHour;
    lpTime64a->nMinute+=lpTime64b.nMinute;
    lpTime64a->nSecond+=lpTime64b.nSecond;
  }
  else if (wchOperator == L'-')
  {
    lpTime64a->nYear-=lpTime64b.nYear;
    lpTime64a->nMonth-=lpTime64b.nMonth;
    lpTime64a->nDay-=lpTime64b.nDay;
    lpTime64a->nHour-=lpTime64b.nHour;
    lpTime64a->nMinute-=lpTime64b.nMinute;
    lpTime64a->nSecond-=lpTime64b.nSecond;
  }
  else if (wchOperator == L'*')
  {
    lpTime64a->nYear*=lpTime64b.nYear;
    lpTime64a->nMonth*=lpTime64b.nMonth;
    lpTime64a->nDay*=lpTime64b.nDay;
    lpTime64a->nHour*=lpTime64b.nHour;
    lpTime64a->nMinute*=lpTime64b.nMinute;
    lpTime64a->nSecond*=lpTime64b.nSecond;
  }
  else if (wchOperator == L'/')
  {
    lpTime64a->nYear/=lpTime64b.nYear;
    lpTime64a->nMonth/=lpTime64b.nMonth;
    lpTime64a->nDay/=lpTime64b.nDay;
    lpTime64a->nHour/=lpTime64b.nHour;
    lpTime64a->nMinute/=lpTime64b.nMinute;
    lpTime64a->nSecond/=lpTime64b.nSecond;
  }
  else if (wchOperator == L'%')
  {
    lpTime64a->nYear%=lpTime64b.nYear;
    lpTime64a->nMonth%=lpTime64b.nMonth;
    lpTime64a->nDay%=lpTime64b.nDay;
    lpTime64a->nHour%=lpTime64b.nHour;
    lpTime64a->nMinute%=lpTime64b.nMinute;
    lpTime64a->nSecond%=lpTime64b.nSecond;
  }
  else if (wchOperator == L'\0')
  {
    lpTime64a->nYear=lpTime64b.nYear;
    lpTime64a->nMonth=lpTime64b.nMonth;
    lpTime64a->nDay=lpTime64b.nDay;
    lpTime64a->nHour=lpTime64b.nHour;
    lpTime64a->nMinute=lpTime64b.nMinute;
    lpTime64a->nSecond=lpTime64b.nSecond;
  }
}

//Function: Arithmetic operation with two units
void MathUnits(__int64 *nValueResult, __int64 nValue, wchar_t wchOperator)
{
  if (wchOperator == L'+')
    *nValueResult+=nValue;
  else if (wchOperator == L'-')
    *nValueResult-=nValue;
  else if (wchOperator == L'*')
    *nValueResult*=nValue;
  else if (wchOperator == L'/')
    *nValueResult/=nValue;
  else if (wchOperator == L'%')
    *nValueResult%=nValue;
  else if (wchOperator == L'\0')
    *nValueResult=nValue;
}
#endif //MATHTIME

int popintegerWide()
{
  wchar_t wszInt[32];

  wszInt[0]=L'\0';
  popstringWide(wszInt, 31);
  return xatoiW(wszInt, NULL);
}

void pushintegerWide(int integer)
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
      len=xstrcpynA(str, (const char *)th->text, len);
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
      len=xstrcpynW(str, th->text, len);
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
    wchar_t wszCurPath[]=L"C:\\";

    g_pluginParms->validate_filename(wszCurPath);
    g_unicode=(wszCurPath[2] == L'\\')?FALSE:TRUE;

    //Initialize WideFunc.h header
    WideInitialize();
    WideGlobal_bOldWindows=!g_unicode;
  }
}
