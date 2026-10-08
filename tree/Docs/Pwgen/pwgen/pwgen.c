/*============================================================================
   pwgen.c - Nullsoft Installer Secure Password Generation Plugin
   Public Domain. Written by: Chase Venters <cventers@southbyte.com>
============================================================================*/

#define _WIN32_WINNT 0x0400
#include <windows.h>
#include <wincrypt.h>
#include "pluginapi.h"

/* ======================================================================== */
/* ============================= Private Data ============================= */
/* ======================================================================== */

/** @brief Password character set */
static const char
pwchars[] = "0123456789"
			"abcdefghijklmnopqrstuvwxyz"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

/* ======================================================================== */
/* =========================== Public Interface =========================== */
/* ======================================================================== */

/** @brief DLL entry point */
BOOL WINAPI
DllMain (
	HANDLE hInst,
	ULONG reason,
	LPVOID reserved
)
{
	return TRUE;
}

/* ======================================================================== */

/** @brief Random password generator */
NSISFunction(GeneratePassword)
{
	PLUGIN_INIT();
	{
		unsigned int digits, i;
		static char temp[256];
		static char pwd[256];
		BOOL ret;

		/* determine number of digits */
		popstring(temp);
		digits = myatou(temp);
		if (digits > 255)
			digits = 255;

		/* generate data */
		if (digits > 0) {
			/* get random data */
			HCRYPTPROV hProv = 0;
			ret = CryptAcquireContext(&hProv, 0, 0, PROV_RSA_FULL,
									  CRYPT_VERIFYCONTEXT);
			if (ret) {
				CryptGenRandom(hProv, digits, (BYTE*)temp);
				CryptReleaseContext(hProv, 0);

				/* translate random data */
				for (i = 0; i < digits; i++)
					pwd[i] = pwchars[temp[i] % (sizeof(pwchars) - 1)];

				/* null-terminate */
				pwd[digits] = 0;

			}
			else
				sprintf(pwd, "ERROR %d", GetLastError());
		}

		/* done */
		pushstring(pwd);
	}
}

/* ======================================================================== */
