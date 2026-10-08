/***********************************************************
 *                                                         *
 * CustomLicense plug-in for NSIS                          *
 *                                                         *
 * Written by Jason Ross aka JasonFriday13 on the forums.  *
 *                                                         *
 * Thanks to the joint effort of brainsucker and kichik    *
 * for the banner plugin, where the myatoi function is     *
 * taken from.                                             *
 *                                                         *
 ***********************************************************/

/* Include the windows.h file, and the NSIS plugin header. */
#include <windows.h>
#ifdef UNICODE
	#include "nsis\unicode\pluginapi.h" /* This means NSIS 2.42 or higher is required. */
#else
	#include "nsis\pluginapi.h" /* This means NSIS 2.42 or higher is required. */
#endif

/* Handle for the program instance. */
HANDLE hInstance;

/******************************************************
  Putting the contents of the code in a function means
  that I have better control of the return values and
  better control of the flow of code (indenting).
 ******************************************************/
unsigned int LoadFileContents(TCHAR FileName[1024], unsigned int hwnd)
{
	/* Declarations of variables. */
	DWORD FileInfo = 0, FileSize = 0;
	HANDLE FileHandle = NULL;
	unsigned int result = 0;
	/* Pointer to buffer for the contents of the license file. */
	LPVOID FilePointer = 0;

	/* Open the file for reading. We want to return an error if the file 
	   cannot be found (hence OPEN_EXISTING), and we only want to read 
	   from the file (hence GENERIC_READ). */
	FileHandle = CreateFile(FileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (!FileHandle || FileHandle == INVALID_HANDLE_VALUE) return GetLastError();
	
	/* Get the file size so we can read the entire file into memory. */
	FileSize = GetFileSize(FileHandle, NULL);
	if (!FileSize) return GetLastError();

	/* Allocate memory for the file based on the size of the file.
	   For some reason, there are some strange characters at the
	   end of the string after it is set in the specified control.
	   As I have found out, adding one to the filesize allows the
	   terminating null character into the string, thus truncating
	   the wierd characters at the end of the string (when I was 
	   testing it). And I used GlobalAlloc at the suggestion of Takhir. */
	FilePointer = GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, FileSize + 1);
	if (!FilePointer) return GetLastError();

	/* Now we read the entire file into memory. */
	result = ReadFile(FileHandle, (LPVOID)FilePointer, FileSize, &FileInfo, NULL);
	if (!result) return GetLastError();	

	SendMessage((HWND)hwnd, WM_SETTEXT, 0, (WPARAM)FilePointer);

	/* Clear the result value so we get a clean error (if one happens). */
	result = 0;

	/* Close the handle to the file. */
	result = CloseHandle(FileHandle);
	if (!result) return GetLastError();

	/* Free the allocated memory. 
	   Forget about a return value (not really needed). */
	GlobalFree(FilePointer);

	return 0;
}

/* This exported function loads the file into memory. */
__declspec(dllexport) void LoadFile(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters* xp)
{
	unsigned int result = 0;
	TCHAR *TempVar0 = NULL;    /* Full file path */
	TCHAR *TempVar1 = NULL;    /* Control HWND */

	EXDLL_INIT();

	TempVar0 = (TCHAR *)GlobalAlloc(GMEM_FIXED, string_size);
	TempVar1 = (TCHAR *)GlobalAlloc(GMEM_FIXED, string_size);

	if ((TempVar0 && TempVar1) == 0)
		result = GetLastError();
	else
	{
		/* Pop the parameters off the stack. */
		popstring(TempVar0);
		popstring(TempVar1);

		/* Call the function so that the contents of the file can be loaded. */
		result = LoadFileContents(TempVar0, myatoi(TempVar1));
	}

	/* Make sure we return the right code. */
	if (result)
		/* Push the error onto the stack. */
		pushstring((const TCHAR *)result);
	else
		/* If no error, return zero. */
		pushstring(TEXT("0"));

	if (TempVar1)
		GlobalFree(TempVar1);
    
	if (TempVar0)
		GlobalFree(TempVar0);
}

/* Entry point for the plugin. */
BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
  hInstance = hInst;
	return 1;
}
