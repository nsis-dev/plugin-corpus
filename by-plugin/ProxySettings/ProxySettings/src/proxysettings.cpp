// test.cpp : Definiert den Einstiegspunkt für die DLL-Anwendung.
//

#include "stdafx.h"
#include "proxysettings.h"

// std::map<int, std::string> g_Proxies;

BOOL APIENTRY DllMain( HANDLE hModule,                      // DLL-Main (immer vorhanden)
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}


/***********************************************************************\
*	This Plugin is written for NSIS										*
*	It provides the functions to get the count of User-defined			*
*	connections, getting these connection's names, and to get or to set	*
*	the proxy settings for these connections							*
\***********************************************************************/



//Teststring:
//ftp=ab.c.de:3456;gopher=bla.hastenichgehört.com:01928473;http=hallo.du.da:1234


	// returns the number of connections available
extern "C" void __declspec(dllexport) __cdecl GetConnectionCount(HWND hwndParent,
										int string_size,
										char *variables,
										stack_t **stacktop)
{
    EXDLL_INIT();

    RASENTRYNAME ren;
    memset(&ren, 0, sizeof(ren));
    ren.dwSize	= sizeof(ren);
    DWORD size	= 0;
    DWORD cnt	= 0;
	char ret[8];
	
    int returnedSetError = RasEnumEntries(0, 0, &ren, &size, &cnt);
	if (returnedSetError == 0 || returnedSetError ==  ERROR_BUFFER_TOO_SMALL)
	{
		int err = (cnt == 0) ? ERR_NO_CONN_AVAILABLE : ERR_SUCCESS;
		sprintf(ret, "%d", err);
		setuservariable(INST_R0, ret);
		sprintf(ret, "%d", cnt);
		pushstring(ret);
	}
	else
	{
		int err = ERR_UNKNOWN;
		sprintf(ret, "%d", err);
		setuservariable(INST_R0, ret);
	}
}
	// Returns the connection name of the index given
extern "C" void __declspec(dllexport) __cdecl GetConnectionName(HWND hwndParent,
										int string_size,
										char *variables,
										stack_t **stacktop)	
{
    EXDLL_INIT();
	
	char s[8];
	popstring(s);
	DWORD index = atoi(s);

    RASENTRYNAME* lpRasEntryName = NULL;
	
    RASENTRYNAME ren;
    memset(&ren, 0, sizeof(ren));
    ren.dwSize	= sizeof(ren);
    DWORD size	= 0;
    DWORD cnt	= 0;

	char err[8];
	int returnedSetError = RasEnumEntries(0, 0, &ren, &size, &cnt);
	if (index >= 0 && index < cnt)
	{
		if (returnedSetError == ERROR_BUFFER_TOO_SMALL)
		{
			lpRasEntryName			= reinterpret_cast<RASENTRYNAME*>(new BYTE[size]);
			lpRasEntryName->dwSize	= sizeof(ren);
			returnedSetError		= RasEnumEntries(0, 0, lpRasEntryName, &size, &cnt);
			
			if (returnedSetError == 0)
			{
				sprintf(err, "%d", ERR_SUCCESS);
				setuservariable(INST_R0, err);

				pushstring(lpRasEntryName[index].szEntryName);
			}
			else
			{
				sprintf(err, "%d", ERR_UNKNOWN);
				setuservariable(INST_R0, err);
			}
		}
		else
		{
			sprintf(err, "%d", ERR_UNKNOWN);
			setuservariable(INST_R0, err);
		}
	}
	else
	{
		sprintf(err, "%d", ERR_WRONG_CONN_INDEX);
		setuservariable(INST_R0, err);
	}
	delete lpRasEntryName;
}

	// Returns the whole proxystring set in the registry
extern "C" void __declspec(dllexport) __cdecl GetConnectionProxy(HWND hwndParent,
										int string_size,
										char *variables,
										stack_t **stacktop)
{
    EXDLL_INIT();
	
	INTERNET_PER_CONN_OPTION_LIST optList;
	
	char connName[1024];
	popstring(connName);
	DWORD success = GetProxyString(&optList, connName);
	char buffer[8];
	if (success == 0)
	{
		LPTSTR value = optList.pOptions->Value.pszValue;
		int err = (value != NULL) && (value[0] == '\0') ? ERR_PROXY_NOT_SET : ERR_SUCCESS;

		sprintf(buffer, "%d", err);
		setuservariable(INST_R0, buffer);

		if (value != NULL)
			pushstring(value);
	}
	else
	{
		sprintf(buffer, "%d", ERR_WRONG_CONN_NAME);
		setuservariable(INST_R0, buffer);
	}
}
	// Returns the  proxystring set for the given protocol
extern "C" void __declspec(dllexport) __cdecl GetConnectionProxyByProtocol(HWND hwndParent,
										int string_size,
										char *variables,
										stack_t **stacktop)
{
    EXDLL_INIT();
	
	int protocol = 0;
	char sprotocol[8];
	char connName[1024];

	popstring(connName);
	popstring(sprotocol);
	protocol = atoi(sprotocol);

	INTERNET_PER_CONN_OPTION_LIST optList;
	char err[8];

	TCHAR retServerName[1024];
	retServerName[0] = '\0';

	DWORD success = GetProxyString(&optList, connName);
	if (success == 0)
	{
		LPTSTR value = optList.pOptions->Value.pszValue;
		if (value == NULL || (value != NULL && value[0] == '\0'))
		{
			sprintf(err, "%d", ERR_PROXY_NOT_SET);
			setuservariable(INST_R0, err);
		}
		else
		{
			int iSuccess = GetProtocolStr(optList.pOptions->Value.pszValue, protocol, retServerName);
			if (iSuccess != ERR_SUCCESS)
			{
				sprintf(err, "%d", iSuccess);
				setuservariable(INST_R0, err);
			}
			else
			{
				sprintf(err, "%d", ERR_SUCCESS);
				setuservariable(INST_R0, err);
			}
		}
	}
	else
	{
		sprintf(err, "%d", ERR_WRONG_CONN_NAME);
		setuservariable(INST_R0, err);
	}

	pushstring(retServerName);

}
// Sets the proxies for a given connection
extern "C" void __declspec(dllexport) __cdecl SetConnectionProxy(HWND hwndParent,
										int string_size,
										char *variables,
										stack_t **stacktop)
{
    EXDLL_INIT();

	char connname[128];
	char proxystring[1024];
	popstring(connname);													// Den Verbindungsnamen holen
	popstring(proxystring);													// den zu schreibenden string holen

	BOOL retErr = SetConnectionOptions(connname, proxystring);
	char buffer[8];
	if (!retErr)
	{	
		int err = GetLastError() == ERROR_INVALID_PARAMETER ? ERR_WRONG_CONN_NAME : ERR_UNKNOWN;
		sprintf(buffer, "%d", err);
	}
	else
	{
		sprintf(buffer, "%d", ERR_SUCCESS);
	}
	setuservariable(INST_R0, buffer);
}	
	// Sets a proxy for one specific protocol to the given connetion
extern "C" void __declspec(dllexport) __cdecl SetConnectionProxyByProtocol(HWND hwndParent,
										int string_size,
										char *variables,
										stack_t **stacktop)
{
    EXDLL_INIT();

	char connname[128];
	char proxystring[1024];
	char writeproxystring[1024];
	char sprotocol[8];

	popstring(connname);													// Den Verbindungsnamen holen
	popstring(proxystring);													// den zu schreibenden string holen
	popstring(sprotocol);													// Das Protokoll des zu setzenden Proxys

	int protocol = atoi(sprotocol);

	const char* token = ";";
	char* p1 = proxystring;
	char* p2 = strtok (p1, token);
	p2 = strtok(NULL, token);	
	proxystring[p2 - p1] = '\0';

	BOOL chkprotocol = 0;
	if (protocol == HTTP)	sprintf(writeproxystring, "http=%s",	proxystring);	else
	if (protocol == HTTPS)	sprintf(writeproxystring, "https=%s",	proxystring);	else
	if (protocol == FTP)	sprintf(writeproxystring, "ftp=%s",		proxystring);	else
	if (protocol == GOPHER)	sprintf(writeproxystring, "gopher=%s",	proxystring);	else
	if (protocol == SOCKS)	sprintf(writeproxystring, "socks=%s",	proxystring);	else
	if (protocol == ALL)	sprintf(writeproxystring, "%s",			proxystring);	else{
							sprintf(writeproxystring, "\0"); // if the protocol given is not valid
							chkprotocol = 1;}

	BOOL retErr = SetConnectionOptions(connname, writeproxystring);
	char err[8];
	int error = ERR_SUCCESS;

	if (!retErr)
	{
		if(GetLastError() == ERROR_INVALID_PARAMETER)
			error = ERR_WRONG_CONN_NAME;
		else
		{
			if(chkprotocol)
				error = ERR_PROXY_WRONG_PROTOCOL;
			else
				error = ERR_UNKNOWN;
		}
		sprintf(err, "%d", error);
	}

	sprintf(err, "%d", error);
	setuservariable(INST_R0, err);
}
	//

DWORD GetProxyString(INTERNET_PER_CONN_OPTION_LIST *optList, char* s)
{
	//INTERNET_PER_CONN_OPTION_LIST optList;
	optList->dwSize			= sizeof(optList);
	optList->dwOptionCount	= 1;
	optList->pszConnection	= "";
	optList->dwOptionError	= 0;

	optList->pszConnection	= s;

	INTERNET_PER_CONN_OPTION opt;
	opt.dwOption			= INTERNET_PER_CONN_PROXY_SERVER;
	optList->pOptions		= &opt;

	DWORD sizeList = sizeof(*optList);
	BOOL success = InternetQueryOption(0, INTERNET_OPTION_PER_CONNECTION_OPTION, optList, &sizeList);
	if (!success)
	{
		if(GetLastError() == ERROR_INVALID_PARAMETER)
			return ERR_WRONG_CONN_NAME;
	}
	return ERR_SUCCESS;
}

int GetProtocolStr(char* proxyString, int protocol, char* retServerName)
{
	retServerName[0] = '\0';

	const char* token = ";";
	LPTSTR p1 = proxyString;
	if (!p1)
	{
		return ERR_PROXY_NOT_SET;
	}
	p1=strtok(p1, token);

	while(p1)
	{
		if ((protocol == HTTP)   && (StrNCmpI("http=",   p1, 5) == 0)) { strcpy(retServerName, &p1[5]); break; } else
		if ((protocol == HTTPS)  && (StrNCmpI("https=",  p1, 6) == 0)) { strcpy(retServerName, &p1[6]); break; } else
		if ((protocol == FTP)    && (StrNCmpI("ftp=",    p1, 4) == 0)) { strcpy(retServerName, &p1[4]); break; } else
		if ((protocol == GOPHER) && (StrNCmpI("gopher=", p1, 7) == 0)) { strcpy(retServerName, &p1[7]); break; } else
		if ((protocol == SOCKS)  && (StrNCmpI("socks=",  p1, 6) == 0)) { strcpy(retServerName, &p1[6]); break; }

		p1 = (strtok(NULL, token));
	}

	if (retServerName[0] != '\0')
		return ERR_SUCCESS;
	else
		return ERR_PROXY_WRONG_PROTOCOL;
}


BOOL SetConnectionOptions(char* connName, char* prxEntry)
{
    INTERNET_PER_CONN_OPTION_LIST list;
    DWORD   dwBufSize = sizeof(list);

    // Fill out list struct.
    list.dwSize = sizeof(list);

    // NULL == LAN, otherwise connection-name.
    list.pszConnection = connName;

    // Set two options. Connectiontype + Proxy-Server
    list.dwOptionCount = 2;
    list.pOptions = new INTERNET_PER_CONN_OPTION[2];

    // Make sure the memory was allocated.
    if(list.pOptions == NULL)
    {
        // Return FALSE if the memory wasn't allocated.
        return FALSE;
    }

    // Set flags.
    list.pOptions[0].dwOption		= INTERNET_PER_CONN_FLAGS;
    list.pOptions[0].Value.dwValue	= PROXY_TYPE_PROXY;
    
    // Set proxy name.
    list.pOptions[1].dwOption		= INTERNET_PER_CONN_PROXY_SERVER;
    list.pOptions[1].Value.pszValue = prxEntry;

    // Set the options on the connection.
    BOOL bReturn = InternetSetOption(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &list, dwBufSize);
	
    // Free the allocated memory.
    delete list.pOptions;
		
    return bReturn;
}