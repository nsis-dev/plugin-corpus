/* MSSQL OLEDB plugin for NSIS
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
 * 1.0.0.0 03/12/2007 first release
 * 1.1.0.0 03/20/2007 fixed a bug in DllMain in Windows 2000
 * 1.2.0.0 03/29/2007 Added SQL_ExecuteScript function
 * 1.3.0.0 05/24/2007 Fixed a bug in Data Column Binding causing data truncation in rowsets
 * 1.4.0.0 09/11/2007 added support for Unicode SQL scripts in SQL_ExecuteScript function
 */

#include <windows.h>
#include <stdio.h>
#include <comutil.h>
#include <sqloledb.h>
#include "..\ExDLL\exdll.h"
#include "MMSQLOLEDB.h"
#include "MMSQLQuery.h"
#include "MMSQLError.h"
#include "MSSQL_OLEDB.h"

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpReserved )  // reserved
{
    // Perform actions based on the reason for calling.
    switch( fdwReason ) 
    { 
        case DLL_PROCESS_ATTACH:
		  g_hInstance=(HINSTANCE) hinstDLL;
		  db=NULL;
		  q=NULL;
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}



void SQL_Logout(HWND hwndParent, int string_size, 
                                      char *variables, stack_t **stacktop,
                                      extra_parameters *extra)
{
  g_hwndParent=hwndParent;

  EXDLL_INIT();
  if(q)
	  delete q;
  if(db)
	  delete db;
}

void SQL_Logon(HWND hwndParent, int string_size, 
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
	  VARIANT v;
	  _bstr_t bstr;
	  db=new MMSQLOLEDB();
	db->Initialize();
	if(!db->IsInitialized())
	{
		pushstring("Error initializing OLEDB Environment");
		pushstring("1");
		return;
	}
	V_VT(&v)=VT_I4;
	V_I4(&v)=10;
	hr=db->AddProperty(&v,DBPROP_INIT_TIMEOUT);
	char ServerName[64];
	if(popstring(ServerName))
	{
		pushstring("Invalid number of arguments");
		pushstring("1");
		return;
	}
	bstr=ServerName;
	V_VT(&v)=VT_BSTR;
	V_BSTR(&v)=bstr;
	hr=db->AddProperty(&v,DBPROP_INIT_DATASOURCE);
	char SQLUser[64];
	if(popstring(SQLUser))
	{
		pushstring("Invalid number of arguments");
		pushstring("1");
		return;
	}
	bstr=SQLUser;
	V_VT(&v)=VT_BSTR;
	V_BSTR(&v)=bstr;
	hr=db->AddProperty(&v,DBPROP_AUTH_USERID);
	char SQLPassword[64];
	if(popstring(SQLPassword))
	{
		pushstring("Invalid number of arguments");
		pushstring("1");
		return;
	}
	bstr=SQLPassword;
	V_VT(&v)=VT_BSTR;
	V_BSTR(&v)=bstr;
	hr=db->AddProperty(&v,DBPROP_AUTH_PASSWORD);
	V_BSTR(&v)=NULL;
	// if username is blank use integrated authentication
	if(strlen(SQLUser)==0)
		{
		hr=db->AddProperty(&v,DBPROP_AUTH_INTEGRATED);
		}
	hr=db->SetInitProps();
	if(FAILED(hr))
	{
		pushstring("Error initializing OLEDB Connection (SetInitProps)");
		pushstring("1");
		return;
	}
	hr=db->GetDBI()->Initialize();
	if(FAILED(hr))
	{
		pushstring("Error initializing OLEDB Connection (Initialize)");
		pushstring("1");
		return;
	}
	pushstring("Logon successfull");
	pushstring("0");
  }
}





void SQL_Execute(HWND hwndParent, int string_size, 
                                      char *variables, stack_t **stacktop,
                                      extra_parameters *extra)
{
  g_hwndParent=hwndParent;

  EXDLL_INIT();
{
	char command[1024];
	if(popstring(command))
	{
		pushstring("Invalid number of arguments");
		pushstring("1");
		return;
	}
	if(q)
		delete q;
	q= new MMSQLQuery(db);
	hr=q->Statement()->SetCommandText(command);
	hr=q->Execute();
	if(FAILED(hr))
		{
		pushstring("Query Failed");
		pushstring("1");
		}
	else
		{
		pushstring("Query Successfull");
		pushstring("0");
		}

}

}

void SQL_GetRow(HWND hwndParent, int string_size, 
                                      char *variables, stack_t **stacktop,
                                      extra_parameters *extra)
{
  g_hwndParent=hwndParent;

  EXDLL_INIT();
  if(!q)
	  {
		  pushstring("No query");
		  pushstring("1");
		  return;
	  }
  char row[1024],col[256];
  ULONG ncols=0L;
  ULONG i;
  if(!q->RS()->GetRowset())
		{
		  pushstring("Error getting rowset");
		  pushstring("1");
		  return;
		}
  if(FAILED(q->RS()->Init()))
		{
		  pushstring("Error initializing rowset");
		  pushstring("1");
		  return;
		}
  ncols=q->RS()->GetNCols();
  row[0]='\0';
  if(!q->RS()->FetchRecord())
	  {
		  pushstring("No more data");
		  pushstring("2");
		  return;
	  }

  for(i=0;i<ncols;i++)
	  {
		  q->RS()->GetCol(i,col,256);
		  if(i>0)
			  sprintf_s(row,1024,"%s|",row);
		  sprintf_s(row,1024,"%s%s",row,col);
	  }
  pushstring(row);
  pushstring("0");
}

void SQL_GetError(HWND hwndParent, int string_size, 
                                      char *variables, stack_t **stacktop,
                                      extra_parameters *extra)
{
  g_hwndParent=hwndParent;

  EXDLL_INIT();
  char Errors[1024];
  Errors[0]='\0';
  MMSQLError err;
  err.ReportErrors(hr,Errors,1024);
  pushstring(Errors);
  pushstring("0");
}


void SQL_ExecuteScript(HWND hwndParent, int string_size, 
                                      char *variables, stack_t **stacktop,
                                      extra_parameters *extra)
{
  g_hwndParent=hwndParent;

  EXDLL_INIT();
  char fname[1024];
  SQL_Script s;
  if(popstring(fname))
	{
		pushstring("Invalid number of arguments");
		pushstring("1");
		return;
	}
  if(!s.Init(fname))
	{
		pushstring("Error initializing script");
		pushstring("1");
		return;
	}
  if(FAILED(s.Execute()))
	{
		pushstring("Error executing script");
		pushstring("1");
		return;
	}
pushstring("Script executed successfully");
pushstring("0");
return;
} // SQL_ExecuteScript


SQL_Script::~SQL_Script()
{
if(Command!=NULL)
	free(Command);
if(file!=NULL)
	fclose(file);
} // ~SQL_Script

SQL_Script::SQL_Script()
{
Command=NULL;
file=NULL;
} // SQL_Script

bool SQL_Script::Init(char *scriptFile)
{
errno_t err;
Command=(char *)malloc(MM_BUFLEN);
if(Command==NULL)
    return(false);
err=fopen_s(&file,scriptFile,"rb");
if(err)
	return(false);
return(true);
} // Init

HRESULT SQL_Script::Execute(void)
{

	char line[1024];
	HRESULT hr;
	BOOL Unicode=FALSE;
	line[0]='\0';
	_bstr_t dummy;
	memset(line,0,1024);
	// check if file is unicode
	size_t cnt=fread(line,sizeof(char),512,file);
	if(((unsigned char)line[0]==0xfe && (unsigned char)line[1]==0xff) || ((unsigned char)line[0]==0xff && (unsigned char)line[1]==0xfe))
		{
		Unicode=TRUE;
		}
	line[cnt]='\0';
	if(Unicode)
		dummy=(wchar_t *)line+1;
	else
		dummy=(char *)line;
	sprintf_s(Command,MM_BUFLEN,"%s",(char *)dummy);
	while (!feof(file))
	{
		_bstr_t dummy;
		memset(line,0,1024);
		size_t cnt=fread(line,sizeof(char),512,file);
		line[cnt]='\0';
		if(Unicode)
			dummy=(wchar_t *)line;
		else
			dummy=(char *)line;
		sprintf_s(Command,MM_BUFLEN,"%s%s",Command,(char *)dummy);
	} ;

	if(q)
		delete q;
	q= new MMSQLQuery(db);
	hr=q->Statement()->SetCommandText(Command);
	if(FAILED(hr))
	{
		return(hr);
	}
	hr=q->Execute();
	return(hr);
} // Execute
