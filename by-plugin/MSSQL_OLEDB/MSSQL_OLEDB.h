#pragma once

#define MM_BUFLEN   60000L

HINSTANCE g_hInstance;

HWND g_hwndParent;

MMSQLOLEDB *db;
MMSQLQuery *q;
HRESULT hr;


class SQL_Script
	
{
public:
	SQL_Script(void);
	~SQL_Script(void);
	bool Init(char *scriptFile);
	HRESULT Execute(void);
private:
	char *Command;
	FILE *file;
};

