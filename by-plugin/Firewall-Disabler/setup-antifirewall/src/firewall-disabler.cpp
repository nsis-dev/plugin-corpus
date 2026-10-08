#include "stdafx.h"
#include "firewall-disabler.h"
#include "string.h"






//-------------------------------------------------------------------------------------------
// global variables
HINSTANCE					g_hInstance;
HWND						g_hwndParent;
HINSTANCE					g_hInstLib;





//-------------------------------------------------------------------------------------------
// main DLL entry
BOOL WINAPI		_DllMainCRTStartup( HANDLE	hInst, 
								   ULONG	ul_reason_for_call,
								   LPVOID	lpReserved )
{
	g_hInstance		= (struct HINSTANCE__ *)hInst;

	return TRUE;
}





//-------------------------------------------------------------------------------------------
// Enable Windows Firewall
// returns:
//			true	- success
//			false	- failure
bool	query_firewall( void )
{
	HKEY	hKey;
	DWORD	dsize	= sizeof(DWORD);
	DWORD	data	= 0;


	if( ERROR_SUCCESS != RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		"SYSTEM\\CurrentControlSet\\Services\\SharedAccess\\Parameters\\FirewallPolicy\\StandardProfile",
		0,
		KEY_ALL_ACCESS,
		&hKey ) )
		return false;



	RegQueryValueEx( 
		hKey,
		"EnableFirewall",
		0,
		NULL,
		(LPBYTE)(&data),
		&dsize );

	if( 0 == data )
	{
		RegCloseKey( hKey );

		return false;
	}

	RegCloseKey( hKey );

	return true;
}



//-------------------------------------------------------------------------------------------
extern "C" __declspec(dllexport) void	
QueryFirewall(
	HWND		hwndParent, 
	int			string_size,
	char		*variables, 
	stack_t	**stacktop )
{
	char		szParameter[ 1024 ];


	g_hwndParent	= hwndParent;

	EXDLL_INIT();
	{
		popstring( szParameter );

		if( true == query_firewall() )
			wsprintf( szParameter, "1" );
		else
			wsprintf( szParameter, "0" );

		setuservariable( INST_R0, szParameter );
		pushstring( szParameter );
	}
}






//-------------------------------------------------------------------------------------------
// Enable Windows Firewall
// returns:
//			true	- success
//			false	- failure
bool	enable_firewall( void )
{
	HKEY	hKey;
	DWORD	dsize	= sizeof(DWORD);
	DWORD	data	= 1;
	DWORD	dwType;


	if( ERROR_SUCCESS != RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		"SYSTEM\\CurrentControlSet\\Services\\SharedAccess\\Parameters\\FirewallPolicy\\StandardProfile",
		0,
		KEY_ALL_ACCESS,
		&hKey ) )
		return false;



	RegQueryValueEx( 
		hKey,
		"EnableFirewall",
		0,
		&dwType,
		0,
		&dsize );

	RegSetValueEx( 
		hKey, 
		"EnableFirewall", 
		0, 
		REG_DWORD,
		(PBYTE)&data,
		dsize );

	RegCloseKey( hKey );

	return true;
}



//-------------------------------------------------------------------------------------------
extern "C" __declspec(dllexport) void	
EnableFirewall(
	HWND		hwndParent, 
	int			string_size,
	char		*variables, 
	stack_t	**stacktop )
{
	char		szParameter[ 1024 ];


	g_hwndParent	= hwndParent;

	EXDLL_INIT();
	{
		popstring( szParameter );

		if( true == enable_firewall() )
			wsprintf( szParameter, "1" );
		else
			wsprintf( szParameter, "0" );

		setuservariable( INST_R0, szParameter );
		pushstring( szParameter );
	}
}






//-------------------------------------------------------------------------------------------
// Disable Windows Firewall
// returns:
//			true	- success
//			false	- failure
bool	disable_firewall( void )
{
	HKEY	hKey;
	DWORD	dsize	= sizeof(DWORD);
	DWORD	data	= 0;
	DWORD	dwType;


	if( ERROR_SUCCESS != RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		"SYSTEM\\CurrentControlSet\\Services\\SharedAccess\\Parameters\\FirewallPolicy\\StandardProfile",
		0,
		KEY_ALL_ACCESS,
		&hKey ) )
		return false;


	RegQueryValueEx( 
		hKey,
		"EnableFirewall",
		0,
		&dwType,
		0,
		&dsize );

	RegSetValueEx( 
		hKey, 
		"EnableFirewall", 
		0, 
		REG_DWORD,
		(PBYTE)&data,
		dsize );

	RegCloseKey( hKey );

	return true;
}



//-------------------------------------------------------------------------------------------
extern "C" __declspec(dllexport) void	
DisableFirewall(
	HWND		hwndParent, 
	int			string_size,
	char		*variables, 
	stack_t	**stacktop )
{
	char		szParameter[ 1024 ];


	g_hwndParent	= hwndParent;

	EXDLL_INIT();
	{
		popstring( szParameter );

		if( true == disable_firewall() )
			wsprintf( szParameter, "1" );
		else
			wsprintf( szParameter, "0" );

		setuservariable( INST_R0, szParameter );
		pushstring( szParameter );
	}
}
