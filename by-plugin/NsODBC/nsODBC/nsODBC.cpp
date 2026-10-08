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

    TEMPNEW(Attributes);

    wchar_t* DriverName = STRNEW();

    if (popstring(DriverName))
    {
        STRDEL(DriverName);
        TEMPDEL(Attributes);
        RET_ERROR();
    }

    DWORD i = 0;

    while (i < (TEMP_SIZE - (g_stringsize + 1))
        && !popstring(&Attributes[i])) i += wcslen(&Attributes[i]) + 1;

    Attributes[i] = 0; /* EOMS 0x0000 */

    BOOL rc = SQLConfigDataSource(NULL, Request, DriverName, Attributes);

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

    STRDEL(DriverName);
    TEMPDEL(Attributes);

    RET(rc);
}

NSISFunction(AddDSN) /* [DriverName] Parameters pairs follows: [DSN=XXX] [UID=0x777] ... */
{
    NS_SQL_CONFIG(ODBC_ADD_DSN);
}

NSISFunction(AddSysDSN) /* [DriverName] Parameters pairs follows: [DSN=XXX] [UID=0x777] ... */
{
    NS_SQL_CONFIG(ODBC_ADD_SYS_DSN);
}

NSISFunction(ConfDSN) /* [DriverName] Parameters pairs follows: [DSN=XXX] [UID=0x777] ... */
{
    NS_SQL_CONFIG(ODBC_CONFIG_DSN);
}

NSISFunction(ConfSysDSN) /* [DriverName] Parameters pairs follows: [DSN=XXX] [UID=0x777] ... */
{
    NS_SQL_CONFIG(ODBC_CONFIG_SYS_DSN);
}

NSISFunction(RemoveDSN) /* [DriverName] Parameters pairs follows: [DSN=XXX] [UID=0x777] ... */
{
    NS_SQL_CONFIG(ODBC_REMOVE_DSN);
}

NSISFunction(RemoveSysDSN) /* [DriverName] Parameters pairs follows: [DSN=XXX] [UID=0x777] ... */
{
    NS_SQL_CONFIG(ODBC_REMOVE_SYS_DSN);
}

NSISFunction(RemoveDefDSN) /* [DriverName] Parameters pairs follows: [DSN=XXX] [UID=0x777] ... */
{
    NS_SQL_CONFIG(ODBC_REMOVE_DEFAULT_DSN);
}

extern "C" BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}
