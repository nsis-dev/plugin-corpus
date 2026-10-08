

/*
  DESC:
  MODS:
*/


#include "Nsisapi.h"
#include "winerror.h"


/*
  --- NSIS SCRIPTABLE FUNCTION ---
  DESC: Find out the real name of a service from it's display name.
  ARGS: Push the service display name onto the stack.
  RETN: Pop the real service name string from the stack.
  RETN: If the service couldn't be found the result will be an empty string.
	RETN: If an error occured the result will be "Error".
  USGE: Use it like this :-
  USGE: 
  USGE:   Push 'Your Display Name'
  USGE:   CallInstDLL x18Services.dll IsServiceInstalled
  USGE:   Pop $0
  USGE:   ; $0 now contains either 'TheRealServiceName'
*/
void __declspec(dllexport) GetServiceNameFromDisplayName(
	HWND      hwndParent, 
	int       string_size, 
	char*     variables, 
	stack_t** stacktop)
{
	char name[256],error[1024];
	int  found = 0;

	g_hwndParent=hwndParent;
	g_stringsize=string_size;
	g_stacktop=stacktop;
	g_variables=variables;

	if(0 == popstring(name)) {
		SC_HANDLE scm;

		_strlwr(name);

		if(scm = OpenSCManager(0,0,SC_MANAGER_ALL_ACCESS)) {
			DWORD bytes_needed=0,entries_returned=0,resume_handle=0;
			LPENUM_SERVICE_STATUS status_buffer;

			if(EnumServicesStatus(
				scm,
				SERVICE_DRIVER|SERVICE_WIN32,
				SERVICE_STATE_ALL,
				0,0,
				&bytes_needed,
				&entries_returned,
				&resume_handle) || GetLastError() == ERROR_MORE_DATA)
			{
				if(status_buffer = malloc(bytes_needed)) {
					if(EnumServicesStatus(
						scm,
						SERVICE_DRIVER|SERVICE_WIN32,
						SERVICE_STATE_ALL,
						status_buffer,
						bytes_needed,
						&bytes_needed,
						&entries_returned,
						&resume_handle))
					{
						DWORD i;

						for(i=0; i<entries_returned; i++) {
							if(strstr(_strlwr(status_buffer[i].lpDisplayName),name)) {
								pushstring(status_buffer[i].lpServiceName);
								found++;
							}
						}

						if(!found) pushstring("Error: No service has that display name!");
					} else {
						PushLastError();
					}
					free(status_buffer);
				}
			} else {
				PushLastError();
			}
			CloseServiceHandle(scm);
		} else {
			PushLastError();
		}
	} else {
		pushstring("Error: No service display name found on the stack!");
	}

	itoa(found,error,10);
	pushstring(error);
}



/*
  --- NSIS SCRIPTABLE FUNCTION ---
  DESC: Find out if the named service is installed.
  ARGS: Push the service name onto the stack.
  RETN: Pop the result string from the stack.
  RETN: The result will be 'Yes' or 'No' if there were no problems, otherwise
  RETN: the reason for the failure will be in the result string.
  USGE: Use it like this :-
  USGE: 
  USGE:   Push 'MyServiceName'
  USGE:   CallInstDLL x18Services.dll IsServiceInstalled
  USGE:   Pop $0
  USGE:   ; $0 now contains either 'Yes', 'No' or an error description
*/
void __declspec(dllexport) IsServiceInstalled(
	HWND      hwndParent, 
	int       string_size, 
	char*     variables, 
	stack_t** stacktop)
{
	char service[100];

	g_hwndParent=hwndParent;
	g_stringsize=string_size;
	g_stacktop=stacktop;
	g_variables=variables;

	if (0 == popstring(service))
	{
		SC_HANDLE hSCM = OpenSCManager(0,0,SC_MANAGER_ALL_ACCESS);

		if (hSCM)
		{
			SC_HANDLE hService = OpenService(hSCM,service,SERVICE_QUERY_STATUS);

			if (hService)
			{
				pushstring("Yes");
				CloseServiceHandle(hService);
			}
			else
				pushstring("No");

			CloseServiceHandle(hSCM);
		}
		else
			PushLastError();
	}
	else
		pushstring("Expected a service name on the stack.");
}

/*
  --- NSIS SCRIPTABLE FUNCTION ---
  DESC: Find out if the named service is installed and running.
  ARGS: Push the service name onto the stack.
  RETN: Pop the result string from the stack.
  RETN: The result will be 'Yes' or 'No' if there were no problems, otherwise
  RETN: the reason for the failure will be in the result string.
  USGE: Use it like this :-
  USGE: 
  USGE:   Push 'MyServiceName'
  USGE:   CallInstDLL x18Services.dll IsServiceRunning
  USGE:   Pop $0
  USGE:   ; $0 now contains either 'Yes', 'No' or an error description
*/
void __declspec(dllexport) IsServiceRunning(
	HWND      hwndParent, 
	int       string_size, 
	char*     variables, 
	stack_t** stacktop)
{
	char service[100];
	SERVICE_STATUS status;

	g_hwndParent=hwndParent;
	g_stringsize=string_size;
	g_stacktop=stacktop;
	g_variables=variables;

	if (0 == popstring(service))
	{
		SC_HANDLE hSCM = OpenSCManager(0,0,SC_MANAGER_ALL_ACCESS);

		if (hSCM)
		{
			SC_HANDLE hService = OpenService(hSCM,service,SERVICE_QUERY_STATUS);

			if (hService)
			{
				if (QueryServiceStatus(hService,&status))
				{
					if (status.dwCurrentState == SERVICE_STOPPED)
						pushstring("No");
					else
						pushstring("Yes");
				}
				else
					PushLastError();
				
				CloseServiceHandle(hService);
			}
			else
				PushLastError();

			CloseServiceHandle(hSCM);
		}
		else
			PushLastError();
	}
	else
		pushstring("Expected a service name on the stack.");
}