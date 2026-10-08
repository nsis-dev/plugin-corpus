/*********************************************************************************************************
 *
 *  Module Name:	nsODBC.cpp
 *
 *  Abstract:		NSIS ODBC managment plugin
 *
 *  Author:		Vyacheslav I. Levtchenko (mail-to: sl@r-tt.com, sl@eltrast.ru)
 * 
 *  Modified:   Florian Otti
 *
 *  Revision History:	20.10.2003	started
 *                      06.12.2023  changed to Unicode 
 *                      11.12.2023  changed parameter format to semicolon separated list and retry SQLConfigDataSource call with multistring
 *
 *  Classes, methods and structures:
 *
 *  TODO:
 *
 *********************************************************************************************************/

#include <windows.h>
#include <odbcinst.h>

#include "debug.h"
#include "exdll.h"
#include <cstdio>

#if 0
 // Constants ---------------------------------------------------------------
 // SQLConfigDataSource request flags
#define  ODBC_ADD_DSN     1               // Add data source
#define  ODBC_CONFIG_DSN  2               // Configure (edit) data source
#define  ODBC_REMOVE_DSN  3               // Remove data source

#if (ODBCVER >= 0x0250)
#define  ODBC_ADD_SYS_DSN 4		  // add a system DSN
#define  ODBC_CONFIG_SYS_DSN	5	  // Configure a system DSN
#define  ODBC_REMOVE_SYS_DSN	6	  // remove a system DSN

#if (ODBCVER >= 0x0300)
#define	 ODBC_REMOVE_DEFAULT_DSN	7 // remove the default DSN
#endif  /* ODBCVER >= 0x0300 */

#endif

#endif

#define TEMP_SIZE	0x10000

#define TEMPNEW(temp)	\
  wchar_t* temp = new wchar_t [TEMP_SIZE]; \
  if (!temp) RET_ERROR ();

#define TEMPDEL(temp)	\
  if (temp) delete temp, temp = NULL;

#define NS_SQL_CONFIG(Request) \
  nsSQLConfig (hwndParent, string_size, variables, stacktop, Request);

static void nsSQLConfig(HWND hwndParent, int string_size, wchar_t* variables, stack_t** stacktop, WORD Request)
{
    EXDLL_INIT();
    brk();

    wchar_t* DriverName = STRNEW();
    wchar_t* Attributes = STRNEW();

    if (popstring(DriverName) || popstring(Attributes))
    {
        STRDEL(DriverName);
        STRDEL(Attributes);
        RET_ERROR();
    }

    BOOL rc = SQLConfigDataSource(NULL, Request, DriverName, Attributes);
    
    if (!rc)
    {
        // Try to do it again, but use the multi-string format (required for SQL Server)
        // multi-string format is separated with null character and ends in two null characters.
        // Rest of string is always null so don't need to take care of that
        for (int i = 0; i < string_size; ++i)
        {
            if (Attributes[i] == L';')
            {
                Attributes[i] = L'\0';
            }
        }
        rc = SQLConfigDataSource(NULL, Request, DriverName, Attributes);
    }

    /* For debugging when trying to figure out what the actual error is
    if (!rc)
    {
        WORD  iError = 1, cbErrorMsg = 0;
        DWORD ErrorCode;
        wchar_t ErrorMsg[SQL_MAX_MESSAGE_LENGTH + 1];
        RETCODE ret;

        brk();

        do
        {
            ret = SQLInstallerError(iError++, &ErrorCode, ErrorMsg, sizeof(ErrorMsg), &cbErrorMsg);
        } while (ret != SQL_NO_DATA && ret != SQL_ERROR);
    }

    // Or when trying to figure out what is being done in the installer
    FILE* logFile;
    errno_t err = fopen_s(&logFile, "debug_log.txt", "w");
    if (err == 0 && logFile != NULL) {

        fwprintf(logFile, L"string_size: %d\n", string_size);

        for (int i = 0; i < string_size; ++i)
        {
            fputwc(Attributes[i] == L'\0' ? L'X' : Attributes[i], logFile);
        }
        fclose(logFile);
    }

    */

    STRDEL(DriverName);
    STRDEL(Attributes);

    RET(rc);
}

NSISFunction(AddDSN) /* [DriverName] Parameters pairs follows: [DSN=XXX,UID=0x777] ... */
{
    NS_SQL_CONFIG(ODBC_ADD_DSN);
}

NSISFunction(AddSysDSN) /* [DriverName] Parameters pairs follows: [DSN=XXX,UID=0x777] ... */
{
    NS_SQL_CONFIG(ODBC_ADD_SYS_DSN);
}

NSISFunction(ConfDSN) /* [DriverName] Parameters pairs follows: [DSN=XXX,UID=0x777] ... */
{
    NS_SQL_CONFIG(ODBC_CONFIG_DSN);
}

NSISFunction(ConfSysDSN) /* [DriverName] Parameters pairs follows: [DSN=XXX,UID=0x777] ... */
{
    NS_SQL_CONFIG(ODBC_CONFIG_SYS_DSN);
}

NSISFunction(RemoveDSN) /* [DriverName] Parameters pairs follows: [DSN=XXX,UID=0x777] ... */
{
    NS_SQL_CONFIG(ODBC_REMOVE_DSN);
}

NSISFunction(RemoveSysDSN) /* [DriverName] Parameters pairs follows: [DSN=XXX,UID=0x777] ... */
{
    NS_SQL_CONFIG(ODBC_REMOVE_SYS_DSN);
}

NSISFunction(RemoveDefDSN) /* [DriverName] Parameters pairs follows: [DSN=XXX,UID=0x777] ... */
{
    NS_SQL_CONFIG(ODBC_REMOVE_DEFAULT_DSN);
}

extern "C" BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}
