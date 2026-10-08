/*
 * Copyright (c) 2008, Quadrom Services S.L.
 * All rights reserved.
 */
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.
//
// This is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
// License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with this software; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>

#include "exdll.h"

HINSTANCE g_hInstance;

//HWND g_hwndParent;

void __declspec(dllexport) GetFirst(HWND hwndParent, int string_size, char *variables, stack_t **stacktop)
{
  EXDLL_INIT();
  {
    static char *p;
	char *szDrivesBuf;
    DWORD dwBufSize;

	dwBufSize = GetLogicalDriveStrings(0, 0);
	if (!dwBufSize) {
		pushstring("error");
		return;
	}

	szDrivesBuf=(char *) GlobalAlloc(GPTR, dwBufSize);
	if (!szDrivesBuf) {
		pushstring("error");
		return;
	}

	dwBufSize = GetLogicalDriveStrings(dwBufSize, szDrivesBuf);
	if (!dwBufSize) {
		pushstring("error");
		return;
	}
	p = szDrivesBuf;

	{
		UINT oldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
		while (*p) {
			//SetupDiGetDeviceRegistryProperty - correct way to find a USB drive, but a major pain to use (because we
			// couldn't find any documentation nor examples)!!
			// So we will just get some information from the volume, assuming that if the information is 
			// available then the media must be present.
			DWORD lpVolumeSerialNumber;
			if(*p != 'A' && *p != 'B'
				&& GetDriveType(p)==DRIVE_REMOVABLE
				&& GetVolumeInformation(p, NULL, 0, &lpVolumeSerialNumber, NULL, NULL, NULL, 0)
				) {
			break; // found it
			}

			p += lstrlen(p) + 1;
		}
		SetErrorMode(oldErrorMode);
	}

	if (*p) {
		pushstring(p);
		p += lstrlen(p) + 1;
	} else {
		pushstring("");
	}
	GlobalFree(szDrivesBuf);
  }
}

void __declspec(dllexport) GetAll(HWND hwndParent, int string_size, char *variables, stack_t **stacktop)
{
	EXDLL_INIT();
	{
		static char *p;
		char *szDrivesBuf;
		char allRemovableDrives[256];
		int ii;
		DWORD dwBufSize;

		dwBufSize = GetLogicalDriveStrings(0, 0);
		if (!dwBufSize) {
			pushstring("error");
			return;
		}

		szDrivesBuf=(char *) GlobalAlloc(GPTR, dwBufSize);
		if (!szDrivesBuf) {
			pushstring("error");
			return;
		}

		dwBufSize = GetLogicalDriveStrings(dwBufSize, szDrivesBuf);
		if (!dwBufSize) {
			pushstring("error");
			return;
		}
		p = szDrivesBuf;

		memset(allRemovableDrives, '\0', sizeof(allRemovableDrives));
		ii = 0;
		{
			UINT oldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
			while (*p) {
				//SetupDiGetDeviceRegistryProperty - correct way to find a USB drive, but a major pain to use (because we
				// couldn't find any documentation nor examples)!!
				// So we will just get some information from the volume, assuming that if the information is 
				// available then the media must be present.
				DWORD lpVolumeSerialNumber;
				if(*p != 'A' && *p != 'B'
						&& GetDriveType(p)==DRIVE_REMOVABLE
						&& GetVolumeInformation(p, NULL, 0, &lpVolumeSerialNumber, NULL, NULL, NULL, 0)
					) {
					allRemovableDrives[ii++] = *p;
				}
				p += lstrlen(p) + 1;
			}
			SetErrorMode(oldErrorMode);
		}

		if (*allRemovableDrives) {
			pushstring(allRemovableDrives);
			p += lstrlen(p) + 1;
		} else {
			pushstring("");
		}
	GlobalFree(szDrivesBuf);
	}
}

BOOL APIENTRY DllMain(HANDLE hInst, unsigned long ul_reason_for_call, LPVOID lpReserved)
{
	g_hInstance = hInst;
	return TRUE;
}