/*****************************************************************
 *                  CDRom NSIS plugin v1.0                       *
 *                                                               *
 * 2005 Shengalts Aleksander aka Instructor (Shengalts@mail.ru)  *
 *****************************************************************/


/* Comment-out and recompile */
#define OPENCD        //Compile with open CD-ROM function
#define CLOSECD       //Compile with close CD-ROM function
#define STATUSCD      //Compile with status CD-ROM function
#define INFOCD        //Compile with CD-ROM information functions
#define ENUMCD        //Compile with find CD-ROMs function




#include <windows.h>
#include <mmsystem.h>

/* Defines */
#define NSIS_MAX_STRLEN 1024

/* NSIS stack structure */
typedef struct _stack_t {
	struct _stack_t *next;
	char text[NSIS_MAX_STRLEN];
} stack_t;

stack_t **g_stacktop;

/* Global variables */
char szBuf[NSIS_MAX_STRLEN]="";
char szError[4]="";
int nVarError;

#ifdef ENUMCD
	char *szDrivesBuf=NULL;
	char *pDrivesBuf=NULL;
	int nGoto=0;
#endif

/* NSIS variables */
enum
{
INST_0,         // $0
INST_1,         // $1
INST_2,         // $2
INST_3,         // $3
INST_4,         // $4
INST_5,         // $5
INST_6,         // $6
INST_7,         // $7
INST_8,         // $8
INST_9,         // $9
INST_R0,        // $R0
INST_R1,        // $R1
INST_R2,        // $R2
INST_R3,        // $R3
INST_R4,        // $R4
INST_R5,        // $R5
INST_R6,        // $R6
INST_R7,        // $R7
INST_R8,        // $R8
INST_R9,        // $R9
INST_CMDLINE,   // $CMDLINE
INST_INSTDIR,   // $INSTDIR
INST_OUTDIR,    // $OUTDIR
INST_EXEDIR,    // $EXEDIR
INST_LANG,      // $LANGUAGE
__INST_LAST
};

/* Funtions prototypes and macros */
int CDRomDoor(char *drive, BOOL eject);
int CDRomStatus(char *drive);
int popstring(char *str);
int getvarindex(char *var);
void setvar(const int varnum, const char *var, int g_stringsize, char *g_variables);

/* NSIS functions code */
#ifdef OPENCD
void __declspec(dllexport) OPEN(HWND hwndParent, int string_size, 
                                      char *variables, stack_t **stacktop)
{
  g_stacktop=stacktop;
  {
	popstring(szBuf);
	popstring(szError);
	nVarError=getvarindex(szError);

	wsprintf(szBuf, "%d", CDRomDoor(szBuf, TRUE));
	setvar(nVarError, szBuf, string_size, variables);
  }
}
#endif //OPENCD

#ifdef CLOSECD
void __declspec(dllexport) CLOSE(HWND hwndParent, int string_size, 
                                      char *variables, stack_t **stacktop)
{
  g_stacktop=stacktop;
  {
	popstring(szBuf);
	popstring(szError);
	nVarError=getvarindex(szError);

	wsprintf(szBuf, "%d", CDRomDoor(szBuf, FALSE));
	setvar(nVarError, szBuf, string_size, variables);
  }
}
#endif //CLOSECD

#ifdef STATUSCD
void __declspec(dllexport) STATUS(HWND hwndParent, int string_size, 
                                      char *variables, stack_t **stacktop)
{
  g_stacktop=stacktop;
  {
	popstring(szBuf);
	popstring(szError);
	nVarError=getvarindex(szError);

	wsprintf(szBuf, "%d", CDRomStatus(szBuf));
	setvar(nVarError, szBuf, string_size, variables);
  }
}
#endif //STATUSCD

#ifdef INFOCD
void __declspec(dllexport) VOLUMENAME(HWND hwndParent, int string_size, 
                                      char *variables, stack_t **stacktop)
{
  g_stacktop=stacktop;
  {
	char szLabel[MAX_PATH]="";

	popstring(szBuf);
	popstring(szError);
	nVarError=getvarindex(szError);

	if (GetDriveType(szBuf) == DRIVE_CDROM && GetVolumeInformation(szBuf, szLabel, sizeof(szLabel), NULL, 0, 0, NULL, 0))
	{
		setvar(nVarError, szLabel, string_size, variables);
		return;
	}
	setvar(nVarError, "", string_size, variables);
  }
}

void __declspec(dllexport) VOLUMESERIALNUMBER(HWND hwndParent, int string_size, 
                                      char *variables, stack_t **stacktop)
{
  g_stacktop=stacktop;
  {
	DWORD dwSerial;

	popstring(szBuf);
	popstring(szError);
	nVarError=getvarindex(szError);

	if (GetDriveType(szBuf) == DRIVE_CDROM && GetVolumeInformation(szBuf, NULL, 0, &dwSerial, 0, 0, NULL, 0))
	{
		wsprintf(szBuf, "%X", dwSerial);
		setvar(nVarError, szBuf, string_size, variables);
		return;
	}
	setvar(nVarError, "", string_size, variables);
  }
}
#endif //INFOCD

#ifdef ENUMCD
void __declspec(dllexport) FINDNEXT(HWND hwndParent, int string_size, 
                                      char *variables, stack_t **stacktop)
{
  g_stacktop=stacktop;
  {
	DWORD dwBufSize;

	popstring(szError);
	nVarError=getvarindex(szError);

	if (nGoto != 1 && (nGoto == -1 || (!(dwBufSize=GetLogicalDriveStrings(0, 0)) ||\
										!(pDrivesBuf=szDrivesBuf=(char *)GlobalAlloc(GPTR, dwBufSize)) ||\
										!GetLogicalDriveStrings(dwBufSize, szDrivesBuf))))
	{
		goto End;
	}

	nGoto=1;

	while (*pDrivesBuf && GetDriveType(pDrivesBuf) != DRIVE_CDROM)
		pDrivesBuf += lstrlen(pDrivesBuf) + 1;

	if (*pDrivesBuf)
	{
		setvar(nVarError, pDrivesBuf, string_size, variables);
		pDrivesBuf += lstrlen(pDrivesBuf) + 1;
		return;
	}

	End:
	nGoto=-1;
	setvar(nVarError, "", string_size, variables);
  }
}

void __declspec(dllexport) FINDCLOSE(HWND hwndParent, int string_size, 
                                      char *variables, stack_t **stacktop)
{
	if (szDrivesBuf) GlobalFree(szDrivesBuf);
}
#endif //ENUMCD

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}



#if defined OPENCD || defined CLOSECD
//Function: Open/Close CDROM door (0 - success, -1 - error)
//          if drive == NULL, then first CD-ROM drive will be used
//          if eject == TRUE, then open CD-ROM door
//          if eject == FALSE, then close CD-ROM door
int CDRomDoor(char *drive, BOOL eject)
{
	MCI_OPEN_PARMS msiOpenParms;
	MCIERROR msiError;
	DWORD dwAction;
	DWORD dwFlags;

	msiOpenParms.dwCallback=0;
	msiOpenParms.lpstrDeviceType="CDAudio";
	msiOpenParms.lpstrElementName=drive;

	dwAction=(eject)?MCI_SET_DOOR_OPEN:MCI_SET_DOOR_CLOSED;
	dwFlags=(!drive || !*drive)?MCI_OPEN_TYPE:MCI_OPEN_TYPE|MCI_OPEN_ELEMENT;

	if (mciSendCommand(0, MCI_OPEN, dwFlags, (DWORD)(LPMCI_OPEN_PARMS) &msiOpenParms) == 0)
	{
		msiError=mciSendCommand(msiOpenParms.wDeviceID, MCI_SET, dwAction, 0);

		mciSendCommand(msiOpenParms.wDeviceID, MCI_CLOSE, MCI_WAIT, 0);

		if (msiError == 0)
			return 0;
	}
	return -1;
}
#endif //OPENCD || CLOSECD

#ifdef STATUSCD
//Function: Gets CDROM status (1 - ready, 0 - not ready, -1 - error)
//          if drive == NULL, then first CD-ROM drive will be used
int CDRomStatus(char *drive)
{
	MCI_OPEN_PARMS msiOpenParms;
	MCI_STATUS_PARMS msiStatusParms;
	MCIERROR msiError;
	DWORD dwFlags;

	msiOpenParms.dwCallback=0;
	msiOpenParms.lpstrDeviceType="CDAudio"; 
	msiOpenParms.lpstrElementName=drive;
	msiStatusParms.dwItem=MCI_STATUS_READY;

	dwFlags=(!drive || !*drive)?MCI_OPEN_TYPE:MCI_OPEN_TYPE|MCI_OPEN_ELEMENT;

	if (mciSendCommand(0, MCI_OPEN, dwFlags, (DWORD)(LPMCI_OPEN_PARMS) &msiOpenParms) == 0)
	{
		msiError=mciSendCommand(msiOpenParms.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPMCI_STATUS_PARMS) &msiStatusParms);

		mciSendCommand(msiOpenParms.wDeviceID, MCI_CLOSE, MCI_WAIT, 0);

		if (msiError == 0)
			return msiStatusParms.dwReturn;
	}
	return -1;
}
#endif //STATUSCD

//Function: Removes the element from the top of the NSIS stack and puts it in the buffer
int popstring(char *str)
{
	stack_t *th;
	if (!g_stacktop || !*g_stacktop) return 1;
	th=(*g_stacktop);
	lstrcpy(str,th->text);
	*g_stacktop = th->next;
	GlobalFree((HGLOBAL)th);
	return 0;
}

//Function: Sets user variable
void setvar(const int varnum, const char *var, int g_stringsize, char *g_variables)
{
	if (var != NULL && varnum >= 0 && varnum < __INST_LAST)
		lstrcpy(g_variables + varnum*g_stringsize, var);
}

// Function: Converts .r0-.r9 and .R0-.R9 to variable index
int getvarindex(char *var)
{
	int nVar=-1;

	if (!lstrcmp(var, ".r0")) nVar=INST_0;
	else if (!lstrcmp(var, ".r1")) nVar=INST_1;
	else if (!lstrcmp(var, ".r2")) nVar=INST_2;
	else if (!lstrcmp(var, ".r3")) nVar=INST_3;
	else if (!lstrcmp(var, ".r4")) nVar=INST_4;
	else if (!lstrcmp(var, ".r5")) nVar=INST_5;
	else if (!lstrcmp(var, ".r6")) nVar=INST_6;
	else if (!lstrcmp(var, ".r7")) nVar=INST_7;
	else if (!lstrcmp(var, ".r8")) nVar=INST_8;
	else if (!lstrcmp(var, ".r9")) nVar=INST_9;
	else if (!lstrcmp(var, ".R0")) nVar=INST_R0;
	else if (!lstrcmp(var, ".R1")) nVar=INST_R1;
	else if (!lstrcmp(var, ".R2")) nVar=INST_R2;
	else if (!lstrcmp(var, ".R3")) nVar=INST_R3;
	else if (!lstrcmp(var, ".R4")) nVar=INST_R4;
	else if (!lstrcmp(var, ".R5")) nVar=INST_R5;
	else if (!lstrcmp(var, ".R6")) nVar=INST_R6;
	else if (!lstrcmp(var, ".R7")) nVar=INST_R7;
	else if (!lstrcmp(var, ".R8")) nVar=INST_R8;
	else if (!lstrcmp(var, ".R9")) nVar=INST_R9;

	return nVar;
}
