#ifndef WINPETOOLS_H_
#define WINPETOOLS_H_

#include <string>

using namespace std;

class WinPETools
{
public:
	WinPETools();
	virtual ~WinPETools();
	
	typedef enum {
		error,
		notPB,
		PB4,
		PB5,
		PB6,
		PBUnknown
	} pblFormat;
	
	static bool isPBExeOrDll(const wchar_t *filename, bool fullcheck, pblFormat *format);
	static bool hasOverlay(const wchar_t *filename, unsigned long *offset, unsigned long *size);
	static bool backupOverlay(const wchar_t* strFilePath, const wchar_t* backupName, unsigned long offset, unsigned long size);
	static bool restoreOverlay(const wchar_t* strFilePath, const wchar_t* backupName, unsigned long *offset);

};

#endif /*WINPETOOLS_H_*/
