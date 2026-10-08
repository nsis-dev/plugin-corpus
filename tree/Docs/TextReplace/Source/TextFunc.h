/*****************************************************************
 *                Text functions header v1.1                     *
 *                                                               *
 * 2006 Shengalts Aleksander aka Instructor (Shengalts@mail.ru)  *
 *                                                               *
 *                                                               *
 *Functions:                                                     *
 * LineFind, LineRead                                            *
 *                                                               *
 *Usage:                                                         *
 *#define LineRead    //insert LineRead function                 *
 *#include "TextFinc.h"                                          *
 *****************************************************************/

#ifndef _TEXTFUNC_
#define _TEXTFUNC_

int LineFind(char *InputFile, char *OutputFile, char *Lines, int (*Callback)(char *, DWORD, int, HANDLE));
int LineRead(char *InputFile, int nDo, LINEREAD *lpResult);

#endif

/********************************************************************
 *
 *  LineFind
 *
 *Find specified lines in text file, and edit or view these lines in callback function.
 *
 *[in]  char *InputFile    -input text file
 *[out] char *OutputFile   -output file, can be NULL or "/NUL" for read only
 *[in]  char *Lines        -range of lines
 *                           1:-1        all lines to change (default)
 *                           2           second line from start
 *                           -3          third line from end
 *                           5:9         range of lines from 5 to 9
 *                           {2}         only second line from start to output
 *                           {4:5 8:-1}  only range of lines from 4 to 5 and 8 to last line to output
 *[in]  int (*Callback)(char *, DWORD, int, HANDLE)   -callback function
 *                      [out,in] char *               -current line (max length 8192)
 *                      [out]    DWORD                -current line number
 *                      [out]    int                  -current line negative number
 *                      [out]    HANDLE               -output file handle opened to write
 *
 *                      Returns:  0  Write line
 *                                1  Skip write line
 *                                2  Exit from function
 *
 *Returns:  LINEFINDEX_SUCCESS          no errors
 *          LINEFINDEX_INPUTFILE_ERROR  can't open input file
 *          LINEFINDEX_OUTPUTFILE_ERROR can't open output file
 *
 *Example:
 *  #include <windows.h>
 *
 *  #define xatoi
 *  #include "ConvFunc.h"
 *
 *  #define LineFind
 *  #include "TextFunc.h"
 *
 *  int callback(char *szLine, DWORD dwLineNumber, int dwLineNumberMinus, HANDLE hWriteHandle)
 *  {
 *    char szBuf[8192];
 *
 *    lstrcpy(szBuf, szLine);
 *    wsprintf(szLine, "[%d:%d]", dwLineNumber, dwLineNumberMinus);
 *    lstrcat(szLine, szBuf);
 *
 *    return 0;  // Write line
 *  }
 *
 *  void main()
 *  {
 *    LineFind("C:\\SCANDISK.LOG", "C:\\SCANDISK2.LOG", "1:-1", callback);
 *  }
 ********************************************************************/
#if defined LineFind
#define LineFind_INCLUDED
#undef LineFind

#define LINEFIND_SUCCESS           0
#define LINEFIND_INPUTFILE_ERROR   1
#define LINEFIND_OUTPUTFILE_ERROR  2

int LineFind(char *InputFile, char *OutputFile, char *Lines, int (*Callback)(char *, DWORD, int, HANDLE))
{
	char *pLines=&Lines[0];
	char *pLines2=&Lines[0];
	char *pReadBuf=NULL;
	char *pReadBuf2=NULL;
	char *pResultLine=NULL;
	char *pPreviosLine=NULL;
	int nCurrentLine=0;
	int nLineStart=0;
	int nLineEnd=0;
	int nError=0;
	DWORD dwLineSum=0;
	DWORD nBytesToRead=0;
	DWORD nBytesDone=0;
	BOOL bNUL=FALSE;
	BOOL bCut=FALSE;
	HANDLE hReadHandle=0;
	HANDLE hWriteHandle=0;

	if (OutputFile == NULL) bNUL=TRUE;
	else if (!lstrcmpi(OutputFile, "/NUL")) bNUL=TRUE;
	else if (!*OutputFile) return LINEFIND_OUTPUTFILE_ERROR;

	if ((hReadHandle=CreateFile(InputFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)) == INVALID_HANDLE_VALUE)
		return LINEFIND_INPUTFILE_ERROR;
	if (bNUL == FALSE && (hWriteHandle=CreateFile(OutputFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0)) == INVALID_HANDLE_VALUE)
	{
		CloseHandle(hReadHandle);
		return LINEFIND_OUTPUTFILE_ERROR;
	}

	nBytesToRead=GetFileSize(hReadHandle, NULL);
	pReadBuf=(char*)GlobalAlloc(GPTR, nBytesToRead+1);
	pReadBuf[nBytesToRead]='\0';
	pReadBuf2=pReadBuf;
	pPreviosLine=pReadBuf;
	ReadFile(hReadHandle, pReadBuf, nBytesToRead, &nBytesDone, NULL);
	CloseHandle(hReadHandle);
	pResultLine=(char*)GlobalAlloc(GPTR, 8192+1);

	while (*pReadBuf2)
	{
		while (*pReadBuf2 != '\n' && *pReadBuf2 != '\r' && *pReadBuf2)
			++pReadBuf2;

		if (*pReadBuf2 == '\n')
		{
			if (*++pReadBuf2 == '\r') ++pReadBuf2;
		}
		else if (*pReadBuf2 == '\r')
		{
			if (*++pReadBuf2 == '\n') ++pReadBuf2;
		}
		++dwLineSum;
	}
	pReadBuf2=pReadBuf;

	if (*pLines2 == '{')
	{
		pLines2=++pLines;
		bCut=TRUE;
	}

	Range:
	while (*pLines2 != ':' && *pLines2 != ' ' && *pLines2 && (bCut == FALSE || *pLines2 != '}'))
		++pLines2;
	if (*pLines2 == ':')
	{
		nLineStart=xatoi(pLines);
		nLineEnd=xatoi(++pLines2);
		while (*pLines2 != ' ' && *pLines2)
			++pLines2;
		if (*pLines2 == ' ')
			pLines=++pLines2;
	}
	else if (*pLines2 == ' ')
	{
		nLineStart=nLineEnd=xatoi(pLines);
		pLines=++pLines2;
	}
	else if (!*pLines2)
		nLineStart=nLineEnd=xatoi(pLines);

	if (nLineStart < 0)
		nLineStart=dwLineSum + nLineStart + 1;
	if (nLineEnd < 0)
		nLineEnd=dwLineSum + nLineEnd + 1;
	if (nLineStart > nLineEnd)
		nLineEnd=nLineStart;
	if ((!nLineStart || !nLineEnd) || nCurrentLine > nLineEnd)
		if (*pLines2) goto Range;
		else if (bCut == TRUE) goto Close;

	while (*pReadBuf2)
	{
		pPreviosLine=pReadBuf2;

		while (*pReadBuf2 != '\n' && *pReadBuf2 != '\r' && *pReadBuf2)
			++pReadBuf2;

		if (*pReadBuf2 == '\n')
		{
			if (*++pReadBuf2 == '\r') ++pReadBuf2;
		}
		else if (*pReadBuf2 == '\r')
		{
			if (*++pReadBuf2 == '\n') ++pReadBuf2;
		}

		++nCurrentLine;

		if (nLineStart <= nCurrentLine && nCurrentLine <= nLineEnd)
		{
			lstrcpyn(pResultLine, pPreviosLine, pReadBuf2 - pPreviosLine + 1);
			nError=Callback(pResultLine, nCurrentLine, nCurrentLine - dwLineSum - 1, hWriteHandle);

			if (nError == 2)
				goto Close;
			if (nError != 1 && bNUL == FALSE)
				WriteFile(hWriteHandle, pResultLine, lstrlen(pResultLine), &nBytesDone, NULL);
		}
		else if (bCut == FALSE && bNUL == FALSE)
		{
			lstrcpyn(pResultLine, pPreviosLine, pReadBuf2 - pPreviosLine + 1);
			WriteFile(hWriteHandle, pResultLine, pReadBuf2 - pPreviosLine, &nBytesDone, NULL);
		}

		if (nCurrentLine >= nLineEnd && !*pLines2 && bCut == TRUE) goto Close;
		else if (nCurrentLine >= nLineEnd && *pLines2) goto Range;
	}

	Close:
	if (bNUL == FALSE) CloseHandle(hWriteHandle);
	if (pReadBuf) GlobalFree(pReadBuf);
	if (pResultLine) GlobalFree(pResultLine);
	return LINEFIND_SUCCESS;
}
#endif

/********************************************************************
 *
 *  LineRead
 *
 *Read lines from text file.
 *
 *[in]  char *InputFile       -input text file
 *[in]  int nDo               -action
 *                              LINEREAD_READFIRST          first call of the function
 *                              LINEREAD_READNEXT           find next line
 *                              LINEREAD_READCLOSE          close search
 *[out] LINEREAD *lpResult    -pointer to the output structure
 *                              [out]    char *szLine          -current line
 *                              [out]    int nLineNumber       -current line number
 *                              [out]    int nLineNumberMinus  -current line negative number
 *                              [out]    HANDLE hReadHandle    -input file handle opened to read
 *                              [out]    int nBufferSize       -read buffer size
 *                              [out]    char *pBuffer         -pointer to the first character of the read buffer
 *                              [out]    char *pCurrentLine    -pointer to the first character of the current line
 *                              [out]    char *pNextLine       -pointer to the first character of the next line
 *
 *Returns:  LINEREAD_SUCCESS          no errors
 *          LINEREAD_FINISHED         last line reached
 *          LINEREAD_INPUTFILE_ERROR  can't open input file
 *          LINEREAD_PARAMETER_ERROR  error in parameter
 *
 *Example:
 *  #include <windows.h>
 *
 *  #define LineRead
 *  #include "TextFunc.h"
 *
 *  void main()
 *  {
 *    char szBuf[8192]="";
 *    LINEREAD lpResult;
 *    DWORD nBytesDone=0;
 *    HANDLE hWriteHandle=0;
 *
 *    if (!LineRead("C:\\SCANDISK.LOG", LINEREAD_READFIRST, &lpResult))
 *    {
 *      if ((hWriteHandle=CreateFile("C:\\SCANDISK2.LOG", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0)) == INVALID_HANDLE_VALUE)
 *        return;
 *
 *      do
 *      {
 *        wsprintf(szBuf, "[%d:%d]", lpResult.nLineNumber, lpResult.nLineNumberMinus);
 *        lstrcat(szBuf, lpResult.szLine);
 *
 *        if (!WriteFile(hWriteHandle, szBuf, lstrlen(szBuf), &nBytesDone, NULL))
 *        {
 *           LineRead(NULL, LINEREAD_READCLOSE, &lpResult);
 *           break;
 *        }
 *      }
 *      while (!LineRead(NULL, LINEREAD_READNEXT, &lpResult));
 *
 *      CloseHandle(hWriteHandle);
 *    }
 *  }
 ********************************************************************/
#ifdef LineRead
#define LineRead_INCLUDED
#undef LineRead

#define LINEREAD_READFIRST  1
#define LINEREAD_READNEXT   2
#define LINEREAD_READCLOSE  3

#define LINEREAD_SUCCESS           0
#define LINEREAD_FINISHED          1
#define LINEREAD_INPUTFILE_ERROR   2
#define LINEREAD_PARAMETER_ERROR   3

typedef struct _LINEREAD {
	char *szLine;
	int nLineNumber;
	int nLineNumberMinus;
	HANDLE hReadHandle;
	int nBufferSize;
	char *pBuffer;
	char *pCurrentLine;
	char *pNextLine;
} LINEREAD;

int LineRead(char *InputFile, int nDo, LINEREAD *lpResult)
{
	static char *pReadBuf;
	static char *pReadBuf2;
	static char *pResultLine;
	static char *pPreviosLine;
	static int nCurrentLine;
	static int nLineStart;
	static int nLineEnd;
	static int nError;
	static int nLineSum;
	static DWORD nBytesToRead;
	static DWORD nBytesDone;
	static HANDLE hReadHandle;
	static BOOL bFinished;

	if (nDo == LINEREAD_READFIRST)
		goto Begin;
	if (bFinished == TRUE)
		goto Close;
	if (nDo == LINEREAD_READNEXT)
		goto FindNext;
	if (nDo == LINEREAD_READCLOSE)
		goto Close;
	return LINEREAD_PARAMETER_ERROR;

	Begin:
	pReadBuf=NULL;
	pReadBuf2=NULL;
	pResultLine=NULL;
	pPreviosLine=NULL;
	nCurrentLine=0;
	nLineSum=0;
	nError=0;
	nBytesToRead=0;
	nBytesDone=0;
	hReadHandle=0;
	bFinished=FALSE;

	if ((hReadHandle=CreateFile(InputFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)) == INVALID_HANDLE_VALUE)
		return LINEREAD_INPUTFILE_ERROR;

	nBytesToRead=GetFileSize(hReadHandle, NULL);
	pReadBuf=(char*)GlobalAlloc(GPTR, nBytesToRead+1);
	pReadBuf[nBytesToRead]='\0';
	pReadBuf2=pReadBuf;
	pPreviosLine=pReadBuf;
	ReadFile(hReadHandle, pReadBuf, nBytesToRead, &nBytesDone, NULL);
	CloseHandle(hReadHandle);

	while (*pReadBuf2)
	{
		while (*pReadBuf2 != '\n' && *pReadBuf2 != '\r' && *pReadBuf2)
			++pReadBuf2;

		if (*pReadBuf2 == '\n')
		{
			if (*++pReadBuf2 == '\r') ++pReadBuf2;
		}
		else if (*pReadBuf2 == '\r')
		{
			if (*++pReadBuf2 == '\n') ++pReadBuf2;
		}
		++nLineSum;
	}
	pReadBuf2=pReadBuf;

	while (*pReadBuf2)
	{
		pPreviosLine=pReadBuf2;

		while (*pReadBuf2 != '\n' && *pReadBuf2 != '\r' && *pReadBuf2)
			++pReadBuf2;

		if (*pReadBuf2 == '\n')
		{
			if (*++pReadBuf2 == '\r') ++pReadBuf2;
		}
		else if (*pReadBuf2 == '\r')
		{
			if (*++pReadBuf2 == '\n') ++pReadBuf2;
		}

		++nCurrentLine;

		if (pResultLine)
		{
			GlobalFree(pResultLine);
			pResultLine=NULL;
		}
		pResultLine=(char*)GlobalAlloc(GPTR, (pReadBuf2 - pPreviosLine + 1) + 1);
		lstrcpyn(pResultLine, pPreviosLine, pReadBuf2 - pPreviosLine + 1);

		lpResult->szLine=pResultLine;
		lpResult->nLineNumber=nCurrentLine;
		lpResult->nLineNumberMinus=nCurrentLine - nLineSum - 1;
		lpResult->hReadHandle=hReadHandle;
		lpResult->nBufferSize=nBytesToRead;
		lpResult->pBuffer=pReadBuf;
		lpResult->pCurrentLine=pPreviosLine;
		lpResult->pNextLine=pReadBuf2;
		return LINEREAD_SUCCESS;

		FindNext:;
	}

	Close:
	if (pReadBuf)
	{
		GlobalFree(pReadBuf);
		pReadBuf=NULL;
	}
	if (pResultLine)
	{
		GlobalFree(pResultLine);
		pResultLine=NULL;
	}
	lpResult->szLine=NULL;
	lpResult->nLineNumber=0;
	lpResult->nLineNumberMinus=0;
	lpResult->hReadHandle=0;
	lpResult->nBufferSize=0;
	lpResult->pBuffer=NULL;
	lpResult->pCurrentLine=NULL;
	lpResult->pNextLine=NULL;
	bFinished=TRUE;
	return LINEREAD_FINISHED;
}
#endif
