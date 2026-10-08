/*****************************************************************
 *                TextReplace NSIS plugin v1.5                   *
 *                                                               *
 * 2006 Shengalts Aleksander aka Instructor (Shengalts@mail.ru)  *
 *****************************************************************/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ConvFunc.h"
#include "StrFunc.h"

/* Defines */
#define NSIS_MAX_STRLEN     1024
#define DETECTBINARY_SIZE   128

#define ERR_POINTER_REPLACEIT    -1
#define ERR_POINTER_REPLACEWITH  -2
#define ERR_EMPTY_REPLACEIT      -3
#define ERR_OPEN_INPUTFILE       -4
#define ERR_GET_FILESIZE         -5
#define ERR_ALLOC_INPUTBUFFER    -6
#define ERR_READ_INPUTFILE       -7
#define ERR_BINARYFILE           -8
#define ERR_OPEN_OUTPUTFILE      -9
#define ERR_ALLOC_OUTPUTBUFFER  -10
#define ERR_WRITE_OUTPUTFILE    -11

/* Include conversion functions */
#define xatoi
#define xitoa
#include "ConvFunc.h"

/* Include string functions */
#define StrReplace
#define GetOptions
#include "StrFunc.h"


/* ExDll */
typedef struct _stack_t {
  struct _stack_t *next;
  char text[1];
} stack_t;

stack_t **g_stacktop;
char *g_variables;
unsigned int g_stringsize;

#define EXDLL_INIT()        \
{                           \
  g_stacktop=stacktop;      \
  g_variables=variables;    \
  g_stringsize=string_size; \
}

/* Global variables */
char szBuf[NSIS_MAX_STRLEN];
char szInputFile[NSIS_MAX_STRLEN];
char szOutputFile[NSIS_MAX_STRLEN];
char szReplaceIt[NSIS_MAX_STRLEN];
char szReplaceWith[NSIS_MAX_STRLEN];
char szOptions[NSIS_MAX_STRLEN];
unsigned char szDetectBuffer[DETECTBINARY_SIZE + 1];

/* Funtions prototypes and macros */
int FillReadBuffer(char *szInputFile, char **lpReadBuffer);
void FreeReadBuffer(char **lpReadBuffer);
int popinteger();
void pushinteger(int integer);
int popstring(char *str, int len);
void pushstring(const char *str, int len);

/* NSIS functions code */
void __declspec(dllexport) _FindInFile(HWND hwndParent, int string_size,
                                      char *variables, stack_t **stacktop)
{
  EXDLL_INIT();
  {
    char *pReadBuf=NULL;
    char *pReplaceIt=&szReplaceIt[0];
    BOOL bSensitive=FALSE;
    int nError;
    int i;

    popstring(szInputFile, NSIS_MAX_STRLEN);
    popstring(szReplaceIt, NSIS_MAX_STRLEN);
    popstring(szOptions, NSIS_MAX_STRLEN);

    if (GetOptions(szOptions, "/S=", FALSE, szBuf, NSIS_MAX_STRLEN) && *szBuf == '1')
      bSensitive=TRUE;
    if (GetOptions(szOptions, "/PI=", FALSE, szBuf, NSIS_MAX_STRLEN) && *szBuf == '1')
    {
      if ((i=xatoi(szReplaceIt)) <= 0 || !(pReplaceIt=(char*)i))
      {
        nError=ERR_POINTER_REPLACEIT;
        goto End;
      }
    }
    if (!*pReplaceIt)
    {
      nError=ERR_EMPTY_REPLACEIT;
      goto End;
    }

    if ((nError=FillReadBuffer(szInputFile, &pReadBuf)) < 0)
      goto End;

    nError=StrReplace(pReadBuf, pReplaceIt, "", bSensitive, NULL, NULL);

    End:
    FreeReadBuffer(&pReadBuf);
    pushinteger(nError);
  }
}

void __declspec(dllexport) _ReplaceInFile(HWND hwndParent, int string_size,
                                      char *variables, stack_t **stacktop)
{
  EXDLL_INIT();
  {
    char *pReadBuf=NULL;
    char *pWriteBuf=NULL;
    char *pReplaceIt=&szReplaceIt[0];
    char *pReplaceWith=&szReplaceWith[0];
    HANDLE hReadHandle=0;
    HANDLE hWriteHandle=0;
    DWORD nBytesDone=0;
    DWORD nAttributes=0;
    BOOL bSensitive=FALSE;
    BOOL bCopyAttrib=TRUE;
    BOOL bNormalAttrib=FALSE;
    BOOL bCopyFile=TRUE;
    int nReadBufSize=0;
    int nWriteBufSize=0;
    int nError=0;
    int i;

    popstring(szInputFile, NSIS_MAX_STRLEN);
    popstring(szOutputFile, NSIS_MAX_STRLEN);
    popstring(szReplaceIt, NSIS_MAX_STRLEN);
    popstring(szReplaceWith, NSIS_MAX_STRLEN);
    popstring(szOptions, NSIS_MAX_STRLEN);

    if (GetOptions(szOptions, "/S=", FALSE, szBuf, NSIS_MAX_STRLEN) && *szBuf == '1')
      bSensitive=TRUE;
    if (GetOptions(szOptions, "/C=", FALSE, szBuf, NSIS_MAX_STRLEN) && *szBuf == '0')
      bCopyFile=FALSE;
    if (GetOptions(szOptions, "/AI=", FALSE, szBuf, NSIS_MAX_STRLEN) && *szBuf == '0')
      bCopyAttrib=FALSE;
    if (GetOptions(szOptions, "/AO=", FALSE, szBuf, NSIS_MAX_STRLEN) && *szBuf == '1')
      bNormalAttrib=TRUE;
    if (GetOptions(szOptions, "/PI=", FALSE, szBuf, NSIS_MAX_STRLEN) && *szBuf == '1')
    {
      if ((i=xatoi(szReplaceIt)) <= 0 || !(pReplaceIt=(char*)i))
      {
        nError=ERR_POINTER_REPLACEIT;
        goto End;
      }
    }
    if (GetOptions(szOptions, "/PO=", FALSE, szBuf, NSIS_MAX_STRLEN) && *szBuf == '1')
    {
      if ((i=xatoi(pReplaceWith)) <= 0 || !(pReplaceWith=(char*)i))
      {
        nError=ERR_POINTER_REPLACEWITH;
        goto End;
      }
    }
    if (!*pReplaceIt)
    {
      nError=ERR_EMPTY_REPLACEIT;
      goto End;
    }

    if ((nError=FillReadBuffer(szInputFile, &pReadBuf)) < 0)
      goto End;

    nReadBufSize=nError;
    nError=StrReplace(pReadBuf, pReplaceIt, pReplaceWith, bSensitive, NULL, &nWriteBufSize);
    --nWriteBufSize;

    if (!nError)
    {
      if (lstrcmpi(szInputFile, szOutputFile) && bCopyFile == TRUE)
        CopyFile(szInputFile, szOutputFile, FALSE);
      goto End;
    }

    if (bCopyAttrib == TRUE) nAttributes=GetFileAttributes(szInputFile);
    if (bNormalAttrib == TRUE) SetFileAttributes(szOutputFile, FILE_ATTRIBUTE_NORMAL);

    if ((hWriteHandle=CreateFile(szOutputFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0)) == INVALID_HANDLE_VALUE)
    {
      nError=ERR_OPEN_OUTPUTFILE;
      goto End;
    }

    if (!(pWriteBuf=(char*)GlobalAlloc(GPTR, nWriteBufSize + 2)))
    {
      nError=ERR_ALLOC_OUTPUTBUFFER;
      goto End;
    }

    StrReplace(pReadBuf, pReplaceIt, pReplaceWith, bSensitive, pWriteBuf, NULL);

    if (!WriteFile(hWriteHandle, pWriteBuf, nWriteBufSize, &nBytesDone, NULL))
    {
      nError=ERR_WRITE_OUTPUTFILE;
      goto End;
    }
    CloseHandle(hWriteHandle);
    hWriteHandle=0;

    if (bCopyAttrib == TRUE) SetFileAttributes(szOutputFile, nAttributes);

    End:
    if (hWriteHandle) CloseHandle(hWriteHandle);
    if (pWriteBuf)
    {
      GlobalFree(pWriteBuf);
      pWriteBuf=NULL;
    }
    FreeReadBuffer(&pReadBuf);
    pushinteger(nError);
  }
}

void __declspec(dllexport) _FillReadBuffer(HWND hwndParent, int string_size,
                                      char *variables, stack_t **stacktop)
{
  EXDLL_INIT();
  {
    char *pReadBuf=NULL;
    int nError;

    popstring(szInputFile, NSIS_MAX_STRLEN);

    if ((nError=FillReadBuffer(szInputFile, &pReadBuf)) < 0)
      pushinteger(nError);
    else
      pushinteger((int)pReadBuf);
  }
}

void __declspec(dllexport) _FreeReadBuffer(HWND hwndParent, int string_size,
                                      char *variables, stack_t **stacktop)
{
  EXDLL_INIT();
  {
    char *pReadBuf;

    pReadBuf=(char *)popinteger();

    FreeReadBuffer(&pReadBuf);
  }
}

void __declspec(dllexport) _Unload(HWND hwndParent, int string_size,
                                      char *variables, stack_t **stacktop)
{
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
  return TRUE;
}


/* Functions */
int FillReadBuffer(char *szInputFile, char **pReadBuffer)
{
  char *pReadBuf=*pReadBuffer=NULL;
  HANDLE hReadHandle=0;
  DWORD nBytesDone=0;
  DWORD nBytesToRead=0;
  int nError=0;
  DWORD i;

  if ((hReadHandle=CreateFile(szInputFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)) == INVALID_HANDLE_VALUE)
  {
    nError=ERR_OPEN_INPUTFILE;
    goto End;
  }

  if (!ReadFile(hReadHandle, szDetectBuffer, DETECTBINARY_SIZE, &nBytesDone, NULL))
  {
    nError=ERR_READ_INPUTFILE;
    goto End;
  }
  szDetectBuffer[nBytesDone]='\0';
  for (i=0; i < DETECTBINARY_SIZE && i < nBytesDone; ++i)
  {
    if (szDetectBuffer[i] == 0x00)
    {
      nError=ERR_BINARYFILE;
      goto End;
    }
  }
  SetFilePointer(hReadHandle, 0, NULL, FILE_BEGIN);

  if ((nBytesToRead=GetFileSize(hReadHandle, NULL)) == INVALID_FILE_SIZE)
  {
    nError=ERR_GET_FILESIZE;
    goto End;
  }
  if (!(pReadBuf=(char*)GlobalAlloc(GPTR, nBytesToRead + 1)))
  {
    nError=ERR_ALLOC_INPUTBUFFER;
    goto End;
  }
  if (!ReadFile(hReadHandle, pReadBuf, nBytesToRead, &nBytesDone, NULL))
  {
    nError=ERR_READ_INPUTFILE;
    goto End;
  }

  End:
  if (hReadHandle) CloseHandle(hReadHandle);

  if (nError == 0)
  {
    *pReadBuffer=pReadBuf;
    return nBytesDone;
  }
  else
  {
    if (pReadBuf) GlobalFree(pReadBuf);
    *pReadBuffer=NULL;
    return nError;
  }
}

void FreeReadBuffer(char **pReadBuffer)
{
  if (*pReadBuffer)
  {
    GlobalFree(*pReadBuffer);
    *pReadBuffer=NULL;
  }
}

int popinteger()
{
  char szInt[32];

  popstring(szInt, 32);
  return xatoi(szInt);
}

void pushinteger(int integer)
{
  char szInt[32];

  xitoa(integer, szInt, 0);
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
