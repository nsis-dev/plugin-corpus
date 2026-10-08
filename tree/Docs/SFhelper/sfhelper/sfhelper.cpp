//---------------------------------------------------------------------------------
#include <windows.h>

// exdll is found in the source tarball at http://nsis.sourceforge.net/Download
#include "../exdll/exdll.h"
#include <stdio.h>
#include <string>
//---------------------------------------------------------------------------------

HINSTANCE g_hInstance;

HWND g_hwndParent;
char *filename=NULL;

FILE *inputFile = NULL;

char *errorURL_TAG="<!DOCTYPE";


//---------------------------------------------------------------------------------
extern "C"  void __declspec(dllexport)
getMirrors( HWND hwndParent,
           int string_size, 
           char *variables,
		   stack_t **stacktop,
           extra_parameters *extra
) {
//---------------------------------------------------------------------------------
	char *location, *mirrorName;
	char buffer[1024];
	int errorURL_LEN = strlen(errorURL_TAG);
	std::string mirrorList = "";
	char *sep="\"";

	int mirrorCount = 0;

	g_hwndParent=hwndParent;

	EXDLL_INIT();

	filename = (char*)calloc(string_size, 1);
	if (!popstring(filename)) {


		if ( inputFile = fopen(filename,"rb") ) {

			fread( buffer,errorURL_LEN,1,inputFile);

			if (!strncmp(buffer, errorURL_TAG, errorURL_LEN)) {

				while( !feof(inputFile) ) {

					fgets(buffer,1024,inputFile);

					location = strstr(buffer,"?use_mirror=");

					if (location) {
						mirrorName=strtok(&location[12],sep);
						if (mirrorCount>0) mirrorList.append("|");
						mirrorList.append(mirrorName);
						mirrorCount++;
					}
				}

			} else {
				mirrorList="error";
			}

			fclose(inputFile);
		}
	}

	free(filename);
	pushstring(mirrorList.c_str());
}



//---------------------------------------------------------------------------------
extern "C" void __declspec(dllexport)
checkFile( HWND hwndParent,
           int string_size, 
           char *variables,
		   stack_t **stacktop,
           extra_parameters *extra
) {
//---------------------------------------------------------------------------------
	char *errorReturn="error";
	char buffer[12];
	int errorURL_LEN = strlen(errorURL_TAG);

	g_hwndParent=hwndParent;

	EXDLL_INIT();

	filename = (char*)calloc(string_size, 1);
	if (!popstring(filename)) {

		if ( inputFile = fopen(filename,"rb") ) {

			fread( buffer,errorURL_LEN,1,inputFile);

			if (strncmp(buffer, errorURL_TAG, errorURL_LEN)) {
				errorReturn="OK";
			}

			fclose(inputFile);
		
		}
	}

	free(filename);
	pushstring(errorReturn);
}



BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	g_hInstance=(HINSTANCE)hInst;
	return TRUE;
}
