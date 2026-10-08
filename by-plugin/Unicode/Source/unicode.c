/*****************************************************************
 *       NSIS plugin for Unicode files conversion v1.0           *
 *                                                               *
 * 2005 Shengalts Aleksander aka Instructor (Shengalts@mail.ru)  *
 *****************************************************************/
 
 /*
  * Modified for NSIS v3.x with unicode support
  *
  * 2022 Jason Ross aka JasonFriday13 on the NSIS forums
  */

#include <windows.h>
// NSIS 2.42 or newer is required for new plugin api
#include "nsis/pluginapi.h"

#define ALLOC(x)   GlobalAlloc(GPTR, (x))
#define FREE       GlobalFree
#define ALLOC_T(x) ALLOC((x) * sizeof(TCHAR))
#define FREE_T     FREE
#define ALLOC_A(x) ALLOC((x) * sizeof(char))
#define FREE_A     FREE
#define ALLOC_W(x) ALLOC((x) * sizeof(WCHAR))
#define FREE_W     FREE

int __UnicodeType(TCHAR *tInFile);

void __declspec(dllexport) UnicodeType(HWND hwndParent, int string_size, 
                                      TCHAR *variables, stack_t **stacktop,
                                      extra_parameters *extra)
{
  EXDLL_INIT();

  {
    TCHAR *tInFile=ALLOC_T(string_size);
    int error=0;

    if (!tInFile)
    {
      error=1;
      pushint(error);
      return;
    }

    popstring(tInFile);
    if (tInFile[0] == 0)
    {
      error=1;
      pushint(error);
      FREE_T(tInFile);
      return;
    }

    __UnicodeType(tInFile);
    FREE_T(tInFile);
  }
}

void __declspec(dllexport) FileUnicode2Ansi(HWND hwndParent, int string_size, 
                                      TCHAR *variables, stack_t **stacktop,
                                      extra_parameters *extra)
{
  EXDLL_INIT();

  {
    HANDLE hFileRead=0;
    HANDLE hFileWrite=0;
    TCHAR *tInFile=ALLOC_T(string_size);
    TCHAR *tOutFile=ALLOC_T(string_size);
    TCHAR *tUnicodeType=ALLOC_T(string_size);
    int error=0;
    int nUnicodeType=0;
    int nMultiByteLen=0;
    DWORD dwNumberOfBytesRead=0;
    DWORD dwNumberOfBytesWritten=0;
    DWORD dwBytesToRead=0;
    void *pReadBuffer=NULL;
    char *pBufferMultiByte=NULL;
    WCHAR *pBufferWideCharLEw=NULL;
    char *pBufferWideCharLEa=NULL;

//Check buffer allocations
    if (!tInFile || !tOutFile || !tUnicodeType)
    {
      error=1;
      goto exit;
    }

//Get parameters
    popstring(tInFile);
    if (tInFile[0] == 0)
    {
      error=1;
      goto exit;
    }
    popstring(tOutFile);
    if (tOutFile[0] == 0)
    {
      error=1;
      goto exit;
    }
    popstring(tUnicodeType);
    if (tUnicodeType[0] == 0)
    {
      error=1;
      goto exit;
    }
    if ((lstrcmpi(tUnicodeType, _T("AUTO")) != 0) &&
      (lstrcmpi(tUnicodeType, _T("UTF-8")) != 0) &&
      (lstrcmpi(tUnicodeType, _T("UTF-16LE")) != 0) &&
      (lstrcmpi(tUnicodeType, _T("UTF-16BE")) != 0))
    {
      error=2;
      goto exit;
    }

//Get unicode file type
    if (lstrcmpi(tUnicodeType, _T("AUTO")) == 0)
    {
      nUnicodeType=__UnicodeType(tInFile);
      if (nUnicodeType == -1)
        return;
      else if (nUnicodeType == 0)
      {
        popstring(NULL);
        error=4;
        goto exit;
      }
      else if ((nUnicodeType == 44) || (nUnicodeType == 46))
      {
        popstring(NULL);
        error=5;
        goto exit;
      }
    }

//ReadFile
    hFileRead=CreateFile(tInFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFileRead == INVALID_HANDLE_VALUE)
    {
      error=6;
      goto exit;
    }

    dwBytesToRead=GetFileSize(hFileRead, NULL);
    pReadBuffer=ALLOC(dwBytesToRead+1);
    ReadFile(hFileRead, pReadBuffer, dwBytesToRead, &dwNumberOfBytesRead, NULL);
    CloseHandle(hFileRead);

//ConvertFile
    if ((nUnicodeType == 13) || (lstrcmpi(tUnicodeType, _T("UTF-8")) == 0))
    {
      //UTF-8 -> UTF-16LE
      pBufferWideCharLEw=(WCHAR*)ALLOC_W(dwBytesToRead);
      MultiByteToWideChar(CP_UTF8, 0, pReadBuffer, -1, pBufferWideCharLEw, dwBytesToRead);

      //UTF-16LE -> ANSI
      pBufferMultiByte=(char*)ALLOC_A(dwBytesToRead);
      nMultiByteLen=WideCharToMultiByte(CP_ACP, 0, pBufferWideCharLEw, -1, pBufferMultiByte, dwBytesToRead, 0, 0);
      --nMultiByteLen;
      if (lstrcmpi(tUnicodeType, _T("UTF-8")) != 0)
      {
        ++pBufferMultiByte;
        --nMultiByteLen;
      }
    }
    else if ((nUnicodeType == 22) || (lstrcmpi(tUnicodeType, _T("UTF-16LE")) == 0))
    {
      //UTF-16LE -> ANSI
      pBufferMultiByte=(char*)ALLOC_A(dwBytesToRead/2);
      nMultiByteLen=WideCharToMultiByte(CP_ACP, 0, pReadBuffer, dwBytesToRead/2, pBufferMultiByte, dwBytesToRead, 0, 0);
      if (lstrcmpi(tUnicodeType, _T("UTF-16LE")) != 0)
      {
        ++pBufferMultiByte;
        --nMultiByteLen;
      }
    }
    else if ((nUnicodeType == 23) || (lstrcmpi(tUnicodeType, _T("UTF-16BE")) == 0))
    {
      DWORD dwCount=0;
      DWORD dwCount2=0;
      char *pBufferWideCharLEa2=NULL;

      //UTF-16BE -> UTF-16LE
      pBufferWideCharLEa=(char*)ALLOC_A(dwBytesToRead);
      pBufferWideCharLEa2=(char*)pReadBuffer;

      for (dwCount=0, dwCount2=1; dwCount != dwBytesToRead; dwCount+=2, dwCount2+=2)
      {
        pBufferWideCharLEa[dwCount]=pBufferWideCharLEa2[dwCount2];
        pBufferWideCharLEa[dwCount2]=pBufferWideCharLEa2[dwCount];
      }
      pBufferWideCharLEw=(WCHAR*)pBufferWideCharLEa;

      //UTF-16LE -> ANSI
      pBufferMultiByte=(char*)ALLOC_A(dwBytesToRead/2);
      nMultiByteLen=WideCharToMultiByte(CP_ACP, 0, pBufferWideCharLEw, dwBytesToRead/2, pBufferMultiByte, dwBytesToRead, 0, 0);
      if (lstrcmpi(tUnicodeType, _T("UTF-16BE")) != 0)
      {
        ++pBufferMultiByte;
        --nMultiByteLen;
      }
    }

//WriteFile
    hFileWrite=CreateFile(tOutFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFileWrite == INVALID_HANDLE_VALUE)
    {
      error=7;
      goto exit;
    }

    WriteFile(hFileWrite, pBufferMultiByte, nMultiByteLen, &dwNumberOfBytesWritten, NULL);
    CloseHandle(hFileWrite);
 
//Exit
exit:
    if (tInFile)            FREE_T(tInFile);
    if (tOutFile)           FREE_T(tOutFile);
    if (tUnicodeType)       FREE_T(tUnicodeType);
    if (pReadBuffer)        FREE(pReadBuffer);
    if (pBufferMultiByte)   FREE_A(pBufferMultiByte);
    if (pBufferWideCharLEa) FREE_A(pBufferWideCharLEa);
    if (pBufferWideCharLEw) FREE_W(pBufferWideCharLEw);
    pushint(error);
  }
}

void __declspec(dllexport) FileAnsi2Unicode(HWND hwndParent, int string_size, 
                                      TCHAR *variables, stack_t **stacktop,
                                      extra_parameters *extra)
{
  EXDLL_INIT();

  {
    HANDLE hFileRead=0;
    HANDLE hFileWrite=0;
    TCHAR *tInFile=ALLOC_T(string_size);
    TCHAR *tOutFile=ALLOC_T(string_size);
    TCHAR *tUnicodeType=ALLOC_T(string_size);
    int error=0;
    int nUnicodeType=0;
    int nMultiByteLen=0;
    int nBufferLen=0;
    DWORD dwNumberOfBytesRead=0;
    DWORD dwNumberOfBytesWritten=0;
    DWORD dwBytesToRead=0;
    DWORD dwWideCharByteLen=0;
    void *pReadBuffer=NULL;
    char *pBufferMultiByte=NULL;
    WCHAR *pBufferWideCharLEw=NULL;
    void *pBufferWrite=NULL;
    char *pBufferWideCharBEa=NULL;

//Check buffer allocations
    if (!tInFile || !tOutFile || !tUnicodeType)
    {
      error=1;
      goto exit;
    }

//Get parameters
    popstring(tInFile);
    if (tInFile[0] == 0)
    {
      error=1;
      goto exit;
    }
    popstring(tOutFile);
    if (tOutFile[0] == 0)
    {
      error=1;
      goto exit;
    }
    popstring(tUnicodeType);
    if (tUnicodeType[0] == 0)
    {
      error=1;
      goto exit;
    }

//Get unicode file type
    nUnicodeType=__UnicodeType(tInFile);
    if (nUnicodeType == -1)
      return;
    else if (nUnicodeType != 0)
    {
      popstring(NULL);
      error=3;
      goto exit;
    }

//ReadFile
    hFileRead=CreateFile(tInFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFileRead == INVALID_HANDLE_VALUE)
    {
      error=6;
      goto exit;
    }

    dwBytesToRead=GetFileSize(hFileRead, NULL);
    pReadBuffer=ALLOC(dwBytesToRead+1);
    ReadFile(hFileRead, pReadBuffer, dwBytesToRead, &dwNumberOfBytesRead, NULL);
    CloseHandle(hFileRead);

//ConvertFile
    if (lstrcmpi(tUnicodeType, _T("UTF-8")) == 0)
    {
      //ANSI -> UTF-16LE
      pBufferWideCharLEw=(WCHAR*)ALLOC_W(dwBytesToRead);
      MultiByteToWideChar(CP_ACP, 0, pReadBuffer, dwBytesToRead, pBufferWideCharLEw, dwBytesToRead);

      //Add BOM
      pBufferMultiByte=(char*)ALLOC_W(dwBytesToRead);
      pBufferMultiByte[0]=(char)0xEF;
      pBufferMultiByte[1]=(char)0xBB;
      pBufferMultiByte[2]=(char)0xBF;
      pBufferMultiByte+=3;

      //UTF-16LE -> UTF-8
      nBufferLen=WideCharToMultiByte(CP_UTF8, 0, pBufferWideCharLEw, dwBytesToRead, pBufferMultiByte, dwBytesToRead*2, 0, 0);
      pBufferWrite=pBufferMultiByte - 3;
      nBufferLen+=3;
    }
    else if (lstrcmpi(tUnicodeType, _T("UTF-16LE")) == 0)
    {
      //Add BOM
      dwWideCharByteLen=dwBytesToRead*2 + sizeof(WCHAR); // null terminator
      pBufferWideCharLEw=(WCHAR*)ALLOC_W(dwBytesToRead + sizeof(WCHAR));
      pBufferWideCharLEw[0]=0xFEFF;
      ++pBufferWideCharLEw;

      //ANSI -> UTF-16LE
      MultiByteToWideChar(CP_ACP, 0, pReadBuffer, dwBytesToRead, pBufferWideCharLEw, dwBytesToRead);
      pBufferWrite=--pBufferWideCharLEw;
      nBufferLen=dwWideCharByteLen;
    }
    else if (lstrcmpi(tUnicodeType, _T("UTF-16BE")) == 0)
    {
      DWORD dwCount=0;
      DWORD dwCount2=0;
      char *pBufferWideCharBEa2=NULL;

      //Add BOM
      dwWideCharByteLen=dwBytesToRead*2 + sizeof(WCHAR); // BOM addition
      pBufferWideCharLEw=(WCHAR*)ALLOC_W(dwBytesToRead + sizeof(WCHAR));
      pBufferWideCharLEw[0]=0xFEFF;
      ++pBufferWideCharLEw;

      //ANSI -> UTF-16LE
      MultiByteToWideChar(CP_ACP, 0, pReadBuffer, dwBytesToRead, pBufferWideCharLEw, dwBytesToRead);
      --pBufferWideCharLEw;

      //UTF-16LE -> UTF-16BE
      pBufferWideCharBEa=(char*)ALLOC_A(dwWideCharByteLen + sizeof(WCHAR));
      pBufferWideCharBEa2=(char*)pBufferWideCharLEw;

      for (dwCount=0, dwCount2=1; dwCount != dwWideCharByteLen; dwCount+=2, dwCount2+=2)
      {
        pBufferWideCharBEa[dwCount]=pBufferWideCharBEa2[dwCount2];
        pBufferWideCharBEa[dwCount2]=pBufferWideCharBEa2[dwCount];
      }
      nBufferLen=dwWideCharByteLen;
      pBufferWrite=pBufferWideCharBEa;
    }
    else if ((lstrcmpi(tUnicodeType, _T("UTF-32LE")) == 0) || (lstrcmpi(tUnicodeType, _T("UTF-32BE")) == 0))
    {
      error=5;
      goto exit;
    }
    else
    {
      error=2;
      goto exit;
    }

//WriteFile
    hFileWrite=CreateFile(tOutFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFileWrite == INVALID_HANDLE_VALUE)
    {
      error=7;
      goto exit;
    }
    WriteFile(hFileWrite, pBufferWrite, nBufferLen, &dwNumberOfBytesWritten, NULL);
    CloseHandle(hFileWrite);
 
//Exit
exit:
    if (tInFile)            FREE_T(tInFile);
    if (tOutFile)           FREE_T(tOutFile);
    if (tUnicodeType)       FREE_T(tUnicodeType);
    if (pReadBuffer)        FREE(pReadBuffer);
    if (pBufferMultiByte)   FREE_A(pBufferMultiByte);
    if (pBufferWideCharBEa) FREE_A(pBufferWideCharBEa);
    if (pBufferWideCharLEw) FREE_W(pBufferWideCharLEw);
    pushint(error);
  }
}

void __declspec(dllexport) FileUnicode2UTF8(HWND hwndParent, int string_size, 
                                      TCHAR *variables, stack_t **stacktop,
                                      extra_parameters *extra)
{
  EXDLL_INIT();

  {
    HANDLE hFileRead=0;
    HANDLE hFileWrite=0;
    TCHAR *tInFile=ALLOC_T(string_size);
    TCHAR *tOutFile=ALLOC_T(string_size);
    TCHAR *tUnicodeType=ALLOC_T(string_size);
    int error=0;
    int nUnicodeType=0;
    int nMultiByteLen=0;
    int nBufferLen=0;
    DWORD dwNumberOfBytesRead=0;
    DWORD dwNumberOfBytesWritten=0;
    DWORD dwBytesToRead=0;
    void *pReadBuffer=NULL;
    char *pBufferMultiByte=NULL;
    WCHAR *pBufferWideCharLEw=NULL;
    char *pBufferWideCharLEa=NULL;

//Check buffer allocations
    if (!tInFile || !tOutFile || !tUnicodeType)
    {
      error=1;
      goto exit;
    }

//Get parameters
    popstring(tInFile);
    if (tInFile[0] == 0)
    {
      error=1;
      goto exit;
    }
    popstring(tOutFile);
    if (tOutFile[0] == 0)
    {
      error=1;
      goto exit;
    }
    popstring(tUnicodeType);
    if (tUnicodeType[0] == 0)
    {
      error=1;
      goto exit;
    }

    if ((lstrcmpi(tUnicodeType, _T("AUTO")) != 0) &&
      (lstrcmpi(tUnicodeType, _T("UTF-8")) != 0) &&
      (lstrcmpi(tUnicodeType, _T("UTF-16LE")) != 0) &&
      (lstrcmpi(tUnicodeType, _T("UTF-16BE")) != 0))
    {
      error=2;
      goto exit;
    }

//Get unicode file type
    if (lstrcmpi(tUnicodeType, _T("AUTO")) == 0)
    {
      nUnicodeType=__UnicodeType(tInFile);
      if (nUnicodeType == -1)
        return;
      else if ((nUnicodeType == 44) || (nUnicodeType == 46))
      {
        popstring(NULL);
        error=5;
        goto exit;
      }
      else if (nUnicodeType == 0) 
      {
        popstring(NULL);
        error=4;
        goto exit;
      }
    }

//ReadFile
    hFileRead=CreateFile(tInFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFileRead == INVALID_HANDLE_VALUE)
    {
      error=6;
      goto exit;
    }
    dwBytesToRead=GetFileSize(hFileRead, NULL);
    pReadBuffer=ALLOC(dwBytesToRead+1);
    ReadFile(hFileRead, pReadBuffer, dwBytesToRead, &dwNumberOfBytesRead, NULL);
    CloseHandle(hFileRead);

//ConvertFile
    if ((nUnicodeType == 13) || (lstrcmpi(tUnicodeType, _T("UTF-8")) == 0))
    {
      DWORD dwCount;
      nMultiByteLen= dwBytesToRead;

      pBufferMultiByte=(char*)ALLOC_A(dwBytesToRead);

      for (dwCount=0; dwCount < dwBytesToRead; dwCount++) {
        pBufferMultiByte[dwCount] = ((char *)pReadBuffer)[dwCount];
      }
    }
    else if ((nUnicodeType == 22) || (lstrcmpi(tUnicodeType, _T("UTF-16LE")) == 0))
    {
      nBufferLen=dwBytesToRead;
      nMultiByteLen = 0;
      while (nMultiByteLen == 0)
      {
        pBufferMultiByte=(char*)ALLOC_A(nBufferLen);

        //UTF-16LE -> UTF-8
        nMultiByteLen=WideCharToMultiByte(CP_UTF8, 0, pReadBuffer, dwBytesToRead/2, pBufferMultiByte, dwBytesToRead, NULL, NULL);
        if ((nMultiByteLen == 0) && (GetLastError () == ERROR_INSUFFICIENT_BUFFER))
        {
          if (pBufferMultiByte)
          {
            FREE_A(pBufferMultiByte);
            pBufferMultiByte = NULL;
          }
          nBufferLen = 2 * nBufferLen;
        }
      }
    }
    else if ((nUnicodeType == 23) || (lstrcmpi(tUnicodeType, _T("UTF-16BE")) == 0))
    {
      DWORD dwCount=0;
      DWORD dwCount2=0;
      char *pBufferWideCharLEa2=NULL;

      //UTF-16BE -> UTF-16LE
      pBufferWideCharLEa=(char*)ALLOC_A(dwBytesToRead);
      pBufferWideCharLEa2=(char*)pReadBuffer;

      for (dwCount=0, dwCount2=1; dwCount != dwBytesToRead; dwCount+=2, dwCount2+=2)
      {
        pBufferWideCharLEa[dwCount]=pBufferWideCharLEa2[dwCount2];
        pBufferWideCharLEa[dwCount2]=pBufferWideCharLEa2[dwCount];
      }
      pBufferWideCharLEw=(WCHAR*)pBufferWideCharLEa;
      nBufferLen=dwBytesToRead;
      pBufferMultiByte=(char*)ALLOC_A(nBufferLen);

      //UTF-16LE -> UTF-8
      nMultiByteLen=WideCharToMultiByte(CP_UTF8, 0, pBufferWideCharLEw, dwBytesToRead/2, pBufferMultiByte, dwBytesToRead, 0, 0);
    }

//WriteFile
    hFileWrite=CreateFile(tOutFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFileWrite == INVALID_HANDLE_VALUE)
    {
      error=7;
      goto exit;
    }

    WriteFile(hFileWrite, pBufferMultiByte, nMultiByteLen, &dwNumberOfBytesWritten, NULL);
    CloseHandle(hFileWrite);
 
//Exit
exit:
    if (tInFile)            FREE_T(tInFile);
    if (tOutFile)           FREE_T(tOutFile);
    if (tUnicodeType)       FREE_T(tUnicodeType);
    if (pReadBuffer)        FREE(pReadBuffer);
    if (pBufferMultiByte)   FREE_A(pBufferMultiByte);
    if (pBufferWideCharLEa) FREE_A(pBufferWideCharLEa);
    if (pBufferWideCharLEw) FREE_W(pBufferWideCharLEw);
    pushint(error);
  }
}

int __UnicodeType(TCHAR *tInFile)
{
  HANDLE hFileRead=0;
  BYTE bom[4];
  DWORD dwNumberOfBytesRead=0;

  hFileRead=CreateFile(tInFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  if (hFileRead == INVALID_HANDLE_VALUE)
  {
    pushint(6);
    return -1;
  }
  ReadFile(hFileRead, &bom, 4, &dwNumberOfBytesRead, NULL);
  CloseHandle(hFileRead);

  if (bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF)
  {
    pushstring(_T("UTF-8"));                    //  Variable Width (Web)
    return 13;
  }
    else if (bom[0] == 0xFF && bom[1] == 0xFE && bom[2] == 0 && bom[3] == 0)
    {
      pushstring(_T("UTF-32LE|UCS-4LE"));   // 32-bit Little Endian
      return 44;
    }
    else if (bom[0] == 0 && bom[1] == 0 && bom[2] == 0xFE&& bom[3] == 0xFF)
    {
      pushstring(_T("UTF-32BE|UCS-4BE"));   // 32-bit Big Endian
      return 46;
    }
    else if (bom[0] == 0xFF && bom[1] == 0xFE)
    {
      pushstring(_T("UTF-16LE|UCS-2LE"));   // Little Endian (Default for Windows)
      return 22;
    }
    else if (bom[0] == 0xFE && bom[1] == 0xFF)
    {
      pushstring(_T("UTF-16BE|UCS-2BE"));   // Big Endian (Default for Linux)
      return 23;
    }
    else
    {
      pushstring(_T("NONE"));              // None Unicode
      return 0;
    }
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
  return TRUE;
}
