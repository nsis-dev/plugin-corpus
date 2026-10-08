/*
  nsMCI DLL v.1.3 - nsMCI.cpp
  October 22th, 2005

  Copyright (C) 2001-2005 Florian Heidenreich (nsis@mp3tag.de)

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include <windows.h>
#include <mmsystem.h>

#include "../../ExDLL/exdll.h"

#pragma comment(lib, "winmm.lib")

HINSTANCE   g_hInstance;
HWND        g_hwndParent;

/////////////////////////////////////////////////////////////////////////////

int WINAPI myatoi(char *s)
{
  unsigned int v=0;
  int sign=1; // sign of positive
  char m=10; // base of 0
  char t='9'; // cap top of numbers at 9

  if (*s == '-')
  {
    s++;  //skip over -
    sign=-1; // sign flip
  }

  if (*s == '0')
  {
    s++; // skip over 0
    if (s[0] >= '0' && s[0] <= '7')
    {
      m=8; // base of 8
      t='7'; // cap top at 7
    }
    if ((s[0] & ~0x20) == 'X')
    {
      m=16; // base of 16
      s++; // advance over 'x'
    }
  }

  for (;;)
  {
    int c=*s++;
    if (c >= '0' && c <= t) c-='0';
    else if (m==16 && (c & ~0x20) >= 'A' && (c & ~0x20) <= 'F') c = (c & 7) + 9;
    else break;
    v*=m;
    v+=c;
  }
  return ((int)v)*sign;
}

/////////////////////////////////////////////////////////////////////////////

void __declspec(dllexport) SendString(HWND hwndParent, int string_size,
                                      char *variables, stack_t **stacktop)
{
   // init global variables
   g_hwndParent = hwndParent;
   g_stringsize = string_size;
   g_stacktop   = stacktop;
   g_variables  = variables;

  {
      // declarations
      char                       bufMCICommand[1024]     = {0,};
      char                       bufMCIReturn[1024]      = {0,};
      char                       bufMCIErrorReturn[1024] = {0,};
      MCIERROR                   mciErr                  = 0;

      while (popstring (bufMCICommand) != 1)
      {
         mciErr = LOWORD(mciSendString(bufMCICommand, bufMCIReturn, sizeof(bufMCIReturn), g_hwndParent));
		 wsprintf(bufMCIErrorReturn, "%d", mciErr);
      }
	  pushstring(bufMCIErrorReturn);
      pushstring(bufMCIReturn);
	}
}

void __declspec(dllexport) GetErrorString(HWND hwndParent, int string_size,
                                          char *variables, stack_t **stacktop)
{
   // init global variables
   g_hwndParent = hwndParent;
   g_stringsize = string_size;
   g_stacktop   = stacktop;
   g_variables  = variables;

  {
      // declarations
      char                       bufMCIReturn[1024]   = {0,};
      MCIERROR                   mciErr               = 0;

      popstring(bufMCIReturn);
	  mciErr = (MCIERROR)myatoi(bufMCIReturn);
      lstrcpy(bufMCIReturn,"");

      if (mciErr != 0)
	  {
        // get error message
		mciGetErrorString (mciErr, bufMCIReturn, sizeof(bufMCIReturn));
	  }
	  pushstring(bufMCIReturn);
	}
}

/////////////////////////////////////////////////////////////////////////////

void __declspec(dllexport) GetFirstCDROMDrive(HWND hwndParent, int string_size,
                                           char *variables, stack_t **stacktop)
{
   // init global variables
   g_hwndParent = hwndParent;
   g_stringsize = string_size;
   g_stacktop   = stacktop;
   g_variables  = variables;

   {
      // declarations
      char                       bufDrive[3] = {0,};
      int                        i = 0;

      // find first cdrom drive
      for (i = 0; i <= 26; i++)
      {
         wsprintf(bufDrive, "%c:", i - 1 + 'A');
         if (GetDriveType (bufDrive) == DRIVE_CDROM)
            break;
      }

	  pushstring(bufDrive);
  }
}

/////////////////////////////////////////////////////////////////////////////

BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
  g_hInstance=hInst;
	return TRUE;
}
