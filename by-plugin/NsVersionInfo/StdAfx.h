// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#ifndef STDAFX_H
#define STDAFX_H


#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

//#include <afxwin.h>         // MFC core and standard components
//#include <afxext.h>         // MFC extensions
#include <string>
#include <windows.h>
#include "CObject.h"

typedef int POSITION;
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;
#include "DeclSpec.h"


std::string format_arg_list(const char *fmt, va_list args);
std::string format(const char *fmt, ...);
std::wstring wformat_arg_list(const wchar_t *fmt, va_list args);
std::wstring wformat(const wchar_t *fmt, ...);

#endif //STDAFX_H
