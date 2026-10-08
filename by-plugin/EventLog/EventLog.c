/* EventLog plugin for NSIS
 * Copyright (C) 2007 Stefano Giusto <sgiusto@mmpoint.it>
 *
 * This software is provided 'as-is', without any express or implied 
 * warranty. In no event will the authors be held liable for any damages 
 * arising from the use of this software. 
 *
 * Permission is granted to anyone to use this software for any purpose, 
 * including commercial applications, and to alter it and redistribute it 
 * freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not 
 *      claim that you wrote the original software. If you use this software 
 *      in a product, an acknowledgment in the product documentation would be
 *      appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not 
 *      be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any source 
 *      distribution.
 * ------------------------------------------------------------------------------
 * Version History
 * 1.0.0.0 03/06/2007 first release
 */

#include <windows.h>
#include <stdio.h>
#include "..\ExDLL\exdll.h"

HINSTANCE g_hInstance;

HWND g_hwndParent;

void Logger(char *srv,char *log,WORD type,WORD cat,WORD msg,char *msgText);
void __declspec(dllexport) WriteEventLog(HWND hwndParent, int string_size, 
                                      char *variables, stack_t **stacktop,
                                      extra_parameters *extra)
{
  g_hwndParent=hwndParent;

  EXDLL_INIT();

  
  // note if you want parameters from the stack, pop them off in order.
  // i.e. if you are called via exdll::myFunction file.dat poop.dat
  // calling popstring() the first time would give you file.dat,
  // and the second time would give you poop.dat. 
  // you should empty the stack of your parameters, and ONLY your
  // parameters.

  {
    char server[128],log[128],text[1024],dummy[128];
	WORD type,cat,msg;
	if(popstring(server))
		{
		pushstring("1");
		return;
		}
	if(popstring(log))
		{
		pushstring("1");
		return;
		}
	if(popstring(dummy))
		{
		pushstring("1");
		return;
		}
	type=atoi(dummy);
	if(popstring(dummy))
		{
		pushstring("1");
		return;
		}
	cat=atoi(dummy);
	if(popstring(dummy))
		{
		pushstring("1");
		return;
		}
	msg=atoi(dummy);
	if(popstring(text))
		{
		pushstring("1");
		return;
		}
	Logger(server,log,type,cat,msg,text);
	pushstring("0");
  }
}



void Logger(char *srv,char *log,WORD type,WORD cat,WORD msg,char *msgText)
{
HANDLE h;
char *msgT[1];
msgT[0]=(char *)malloc(1024);
if(msgT[0])
	{
	sprintf_s(msgT[0],1024,"%s",msgText);
	// first try with given server and log
	h=RegisterEventSource(srv,log);
	// then try with given log on local machine
	if(!h)
		h=RegisterEventSource(NULL,log);
	// last try application log on local machine
	if(!h)
		h=RegisterEventSource(NULL,"Application");
	if(h)
		{
		ReportEvent(h,
					type,
					cat,
					msg,
					NULL,
					1,0,(const char **)msgT,NULL);
		DeregisterEventSource(h);
		}
	free(msgT[0]);
	}
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
  g_hInstance=(HINSTANCE) hInst;
	return TRUE;
}


