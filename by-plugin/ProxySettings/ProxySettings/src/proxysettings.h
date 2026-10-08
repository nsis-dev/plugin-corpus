#pragma once


#include <ExDll.h>
#include <string>
#include <map>
#include <ras.h>
#include <raserror.h>
#include <wininet.h>
#include <shlwapi.h>

// Modeflags for selected Proxy-protocols
#define MODE_SELECT_HTTP					 1
#define MODE_SELECT_HTTPS					 2
#define MODE_SELECT_FTP						 4
#define MODE_SELECT_GOPHER					 8
#define MODE_SELECT_SOCKS					16
#define MODE_SELECT_ALL						32

enum MODESEL
{
	HTTP									=  1,
	HTTPS									=  2,
	FTP										=  4,
	GOPHER									=  8,
	SOCKS									= 16,
	ALL										= 32
};

// Searchstrings
#define STR_HTTP							"http="
#define STR_HTTPS							"https="
#define STR_FTP								"ftp="
#define STR_GOPHER							"gopher="
#define STR_SOCKS							"socks="

// Errorcodes
#define ERR_SUCCESS							0
#define ERR_UNKNOWN							1
#define ERR_NO_CONN_AVAILABLE				2
#define ERR_WRONG_CONN_NAME					3
#define ERR_WRONG_CONN_INDEX				4
#define ERR_PROXY_WRONG_PROTOCOL			5
#define ERR_PROXY_NOT_SET					6


BOOL	SetConnectionOptions (char*, char*);
DWORD	GetProxyString(INTERNET_PER_CONN_OPTION_LIST*, char*);
int		GetProtocolStr(char*, int, char*);


