#define UNICODE
#include <string>
#include <stdio.h>
#include <iostream>
#include <windows.h>
#include <winnt.h>

#include "WinPETools.h"

using namespace std;

bool RealignPBExeOrDll(const wchar_t *filename, unsigned long fixingOffset);
bool RealignPBObject(char * baseAddr, long blockPtr, long deltaOffset, long *items);

WinPETools::WinPETools()
{
}

WinPETools::~WinPETools()
{
}

/**
 * Dump a PE overlay into a temporary file that can be restored by restoreOverlay
 */
bool WinPETools::backupOverlay(const wchar_t* strFilePath, const wchar_t* backupName, unsigned long offset, unsigned long size)
{
	HANDLE hFile, hFileBak, hMapObj, hBaseAddress;
	unsigned long fileSize, sizeWritten;
	bool ret = true;

	hFile = CreateFile(strFilePath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE){
#ifdef _DEBUG
		cout << "backupOverlay::CreateFile src INVALID_HANDLE_VALUE" << endl;
#endif
		return false;
	}
	fileSize = GetFileSize(hFile, NULL);
	if ((offset + size) > fileSize){
		CloseHandle(hFile);
		return false;
	}
	hFileBak = CreateFile(backupName, GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE){
#ifdef _DEBUG
		cout << "backupOverlay::CreateFile ovr INVALID_HANDLE_VALUE" << endl;
#endif
		CloseHandle(hFile);
		return false;
	}
	hMapObj = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, 0);
	if (!hMapObj){
#ifdef _DEBUG
		cout << "backupOverlay::CreateFileMapping failed" << endl;
#endif
		CloseHandle(hFile);
		CloseHandle(hFileBak);
		return false;
	}
	if (! (hBaseAddress = MapViewOfFile(hMapObj, FILE_MAP_READ, 0, 0, 0 ))){
#ifdef _DEBUG
		cout << "backupOverlay::MapViewOfFile failed" << endl;
#endif
		CloseHandle(hMapObj);
		CloseHandle(hFile);
		CloseHandle(hFileBak);
		return false;
	}
	WriteFile(hFileBak, (char*)hBaseAddress + offset, size, &sizeWritten, NULL);
	if (sizeWritten != size)
		ret = false;

	UnmapViewOfFile(hBaseAddress);
	CloseHandle(hMapObj);
	CloseHandle(hFile);
	CloseHandle(hFileBak);
	return ret;
}

/**
 * Restore an overlay into a PE file
 */
bool WinPETools::restoreOverlay(const wchar_t* strFilePath, const wchar_t* backupName, unsigned long *offset)
{
	HANDLE hFile, hOvr;
	BYTE buff[4096];
	unsigned long bytesRead, bytesWritten, pos, originalEnd;
	pblFormat pbFormat;

	hFile = CreateFile(strFilePath, FILE_APPEND_DATA, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
#ifdef _DEBUG
		cout << "restoreOverlay::CreateFile dest INVALID_HANDLE_VALUE" << endl;
#endif
		return false;
	originalEnd = GetFileSize(hFile, NULL);
	hOvr = CreateFile(backupName, GENERIC_READ, 0 /*do not share*/, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hOvr == INVALID_HANDLE_VALUE){
#ifdef _DEBUG
		cout << "restoreOverlay::CreateFile ovr INVALID_HANDLE_VALUE" << endl;
#endif
		CloseHandle(hFile);
	}
	while(ReadFile(hOvr, buff, sizeof(buff), &bytesRead, NULL) && bytesRead > 0){
		pos = SetFilePointer(hFile, 0, NULL, FILE_END);
		LockFile(hFile, pos, 0, bytesRead, 0);
		WriteFile(hFile, buff, bytesRead, &bytesWritten, NULL);
		UnlockFile(hFile, pos, 0, bytesRead, 0);
	}
	CloseHandle(hFile);
	CloseHandle(hOvr);

	*offset = originalEnd;

	if (isPBExeOrDll(strFilePath, false, &pbFormat))
		RealignPBExeOrDll(strFilePath, originalEnd);

	return true;
}

/*
IMAGE_DOS_SIGNATURE = MZ
IMAGE_NT_SIGNATURE	= PE
IMAGE_DOS_HEADER
IMAGE_FILE_HEADER
IMAGE_OPTIONAL_HEADER32
*/
/**
 * Tells if a PE executable has an overlay
 * = if the file size is bigger than the size set in the PE header
 */
bool WinPETools::hasOverlay(const wchar_t *filename, unsigned long *OvrOffset, unsigned long *OvrSize)
{
	bool ret = false;
	HANDLE hFile, hMapObj, hBaseAddress;
	unsigned long lastSection = 0L, lastSectionSize = 0L, PESize;
	unsigned long fileSize;

	IMAGE_DOS_HEADER *ImageDosHeader;	//DOS header
	IMAGE_NT_HEADERS *ImageNtHeaders;	//COFF header + Optional header
	IMAGE_SECTION_HEADER *ImageSectionHeader;

#ifdef _DEBUG
	wprintf(L"hasOverlay(%s)\n", filename);
#endif
	hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE){
#ifdef _DEBUG
		cout << "hasOverlay::CreateFile INVALID_HANDLE_VALUE" << endl;
#endif
		return false;
	}

	fileSize = GetFileSize(hFile, NULL);
	if (!fileSize){
		//MSDN says : An attempt to map a file with a length of 0 (zero) fails with an error code of ERROR_FILE_INVALID.
		// Applications should test for files with a length of 0 (zero) and reject those files.
#ifdef _DEBUG
		cout << "Taille nulle" << endl;
#endif
		CloseHandle(hFile);
		return false;
	}
	hMapObj = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, 0);
	if (!hMapObj){
#ifdef _DEBUG
		cout << "hasOverlay::CreateFileMapping failed" << endl;
#endif
		CloseHandle(hFile);
		return false;
	}
	if (! (hBaseAddress = MapViewOfFile(hMapObj, FILE_MAP_READ, 0, 0, 0 ))){
#ifdef _DEBUG
		cout << "hasOverlay::MapViewOfFile failed" << endl;
#endif
		CloseHandle(hMapObj);
		CloseHandle(hFile);
		return false;
	}
	// file is now mapped in memory
	ImageDosHeader = (IMAGE_DOS_HEADER*)hBaseAddress;
	if(ImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE){
#ifdef _DEBUG
		wprintf(L"%s n'est pas un exe MZ\n", filename);
#endif
		goto CleanUp;
	}
	else{
#ifdef _DEBUG
		wprintf(L"%s est un exe MZ\n", filename);
#endif
	}

	ImageNtHeaders = (IMAGE_NT_HEADERS*)(ImageDosHeader->e_lfanew + (DWORD)ImageDosHeader);
	if(ImageNtHeaders->Signature != IMAGE_NT_SIGNATURE){
#ifdef _DEBUG
		wprintf(L"%s n'est pas un exe PE\n", filename);
#endif
		goto CleanUp;
	}
	else
	{
#ifdef _DEBUG
		wprintf(L"%s est un exe PE\n", filename);
#endif
	}

	PESize = ImageNtHeaders->OptionalHeader.SizeOfHeaders; //min size of PE = size of the headers
#ifdef _DEBUG
	printf("PE headersize = %lx\n", PESize);
#endif
	IMAGE_SECTION_HEADER *Img;
	Img = IMAGE_FIRST_SECTION(ImageNtHeaders);
	for(int i = 0; i < ImageNtHeaders->FileHeader.NumberOfSections; i++){
#ifdef _DEBUG
		printf("Section %s off=%lx size=%lx\n",Img[i].Name, Img[i].PointerToRawData, Img[i].SizeOfRawData);
#endif
		if(Img[i].PointerToRawData > lastSection){
			lastSection = Img[i].PointerToRawData;
			lastSectionSize = Img[i].SizeOfRawData;
		}
	}
	PESize += lastSection + lastSectionSize;
#ifdef _DEBUG
	printf("PESize=%lx lastSection=%lx lastSectionSize=%lx\n",PESize,lastSection, lastSectionSize);
#endif
	if (fileSize > PESize){
		ret = true;
		if (OvrOffset)
			*OvrOffset = lastSection + lastSectionSize;
		if (OvrSize)
			*OvrSize = fileSize - (lastSection + lastSectionSize);
	}

	//cleanup
CleanUp:
	UnmapViewOfFile(hBaseAddress);
	CloseHandle(hMapObj);
	CloseHandle(hFile);
	return ret;
}

/**
 * check if the given file is a Powerbuilder exe or Dll (it has an attached overlay)
 */
bool WinPETools::isPBExeOrDll(const wchar_t *filename, bool fullcheck, pblFormat *format)
{
	HANDLE hFile, hMapObj, hBaseAddress;
	unsigned long fileSize;
	char *pTLR, *pHDR;
	long offset;
	bool ret = false;

	hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE){
#ifdef _DEBUG
		cout << "isPBExeOrDll::CreateFile INVALID_HANDLE_VALUE" << endl;
#endif
		if (format)
			*format = error;
		return false;
	}

	fileSize = GetFileSize(hFile, NULL);
	if (!fileSize){
		//MSDN says : An attempt to map a file with a length of 0 (zero) fails with an error code of ERROR_FILE_INVALID.
		// Applications should test for files with a length of 0 (zero) and reject those files.
#ifdef _DEBUG
		cout << "Taille nulle" << endl;
#endif
		CloseHandle(hFile);
		if (format)
			*format = notPB;
		return false;
	}
	hMapObj = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, 0);
	if (!hMapObj){
#ifdef _DEBUG
		cout << "isPBExeOrDll::CreateFileMapping failed" << endl;
#endif
		CloseHandle(hFile);
		if (format)
			*format = error;
		return false;
	}
	if (! (hBaseAddress = MapViewOfFile(hMapObj, FILE_MAP_READ, 0, 0, 0 ))){
#ifdef _DEBUG
		cout << "isPBExeOrDll::MapViewOfFile failed" << endl;
#endif
		CloseHandle(hMapObj);
		CloseHandle(hFile);
		if (format)
			*format = error;
		return false;
	}

	if(fileSize > 512){	//512 is the size of the TLR* tailer block
		pTLR = (char *)hBaseAddress + fileSize - 512;
		if (strncmp(pTLR, "TRL*", 4)){
			//not a TRL* block
#ifdef _DEBUG
			fwprintf(stderr, L"%s n'est pas un PB\n", filename);
#endif
			if (format)
				*format = notPB;
			ret = false;
			goto CleanUp;
		}
		if (fullcheck){
			offset = *((unsigned long*)(pTLR + 4));
			pHDR = (char*)hBaseAddress + offset;				// get the starting offset for HDR*
			if(strncmp(pHDR, "HDR*", 4)){
				//not a HDR* block
#ifdef _DEBUG
				fwprintf(stderr, L"Pas trouve de HDR...\n");
#endif
				if (format)
					*format = notPB;
				ret = false;
				goto CleanUp;
			}
			if(*(pHDR + 5)){ //go to the char following the 'P' of 'PowerbuilderNULLNULL' : it is either 'o' or 0
				//non unicode
				pHDR += 18;
				if (!strncmp(pHDR, "0400", 4)){
					if (format)
						*format = PB4;
				}
				else if (!strncmp(pHDR, "0500", 4)){
					if (format)
								*format = PB5;
				}
				else if (!strncmp(pHDR, "0600", 4)){
					if (format)
						*format = PB6;
				}
				else{
					if (format)
						*format = PBUnknown;
				}
			}
			else{
				//unicode
				pHDR +=32;
				if (!wcsncmp((wchar_t*)pHDR, L"0400", 4)){
					if (format)
						*format = PB4;
				}
				else if (!wcsncmp((wchar_t*)pHDR, L"0500", 4)){
					if (format)
						*format = PB5;
				}
				else if (!wcsncmp((wchar_t*)pHDR, L"0600", 4)){
					if (format)
						*format = PB6;
				}
				else{
					if (format)
						*format = PBUnknown;
				}
			}
		}
		ret = true;
	}else{
		// file size < 512 bytes
		if (format)
			*format = notPB;
	}

	CleanUp:
	UnmapViewOfFile(hBaseAddress);
	CloseHandle(hMapObj);
	CloseHandle(hFile);
	return ret;
}

/**
 * Parse a Powerbuilder exe or dll to fix the offsets in the overlay
 */
bool RealignPBExeOrDll(const wchar_t *filename, unsigned long fixingOffset)
{
	HANDLE hFile, hMapObj, hBaseAddress;
	unsigned long offsetOri;
	long delta;
	long processedItems;
	unsigned long fileSize;
	bool ret = true;

	hFile = CreateFile(filename, GENERIC_READ + GENERIC_WRITE, 0 /*NO SHARING*/, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE){
#ifdef _DEBUG
		cout << "RealignPBExeOrDll::CreateFile INVALID_HANDLE_VALUE" << endl;
#endif
		return false;
	}
	fileSize = GetFileSize(hFile, NULL);
	if (fixingOffset > fileSize){
		CloseHandle(hFile);
		return false;
	}
	hMapObj = CreateFileMapping(hFile, NULL, PAGE_READWRITE , 0, 0, 0);
	if (!hMapObj){
#ifdef _DEBUG
		cout << "RealignPBExeOrDll::CreateFileMapping failed" << endl;
#endif
		CloseHandle(hFile);
		return false;
	}
	if (! (hBaseAddress = MapViewOfFile(hMapObj, FILE_MAP_WRITE, 0, 0, 0 ))){
#ifdef _DEBUG
		cout << "RealignPBExeOrDll::MapViewOfFile failed" << endl;
#endif
		CloseHandle(hMapObj);
		CloseHandle(hFile);
		return false;
	}

	//fix the starting offset
	offsetOri = *(long*)((char *)hBaseAddress + fileSize - 512 + 4);
	delta = fixingOffset - offsetOri;
	if (delta){
#ifdef _DEBUG
		printf("Fixing the PBD from offset %p with offset %lx\n",(char *)hBaseAddress + fixingOffset, delta);
#endif
		long fixed = 0;
		RealignPBObject((char *)hBaseAddress, fixingOffset, delta, &fixed);  // fix from the HDR*
		RealignPBObject((char *)hBaseAddress, fileSize - 512, delta, &fixed);// fix the TRL*
	}
CleanUp:
	UnmapViewOfFile(hBaseAddress);
	CloseHandle(hMapObj);
	CloseHandle(hFile);
	return ret;

}

/**
 * Realign a PB resource block
 */
bool RealignPBObject(char * baseAddr, long blockPtr, long deltaOffset, long *items)
{
	if(!strncmp(baseAddr + blockPtr, "TRL*", 4)){
#ifdef _DEBUG
		printf("Fixing the TRL* at offset %p\n",baseAddr + blockPtr);
#endif
		*((unsigned long*)(baseAddr + blockPtr + 4)) += deltaOffset;
		(*items)++;
	}
	else if(!strncmp(baseAddr + blockPtr, "HDR*", 4)){
#ifdef _DEBUG
		printf("Fixing the HDR* at offset %p\n",baseAddr + blockPtr);
#endif
		long fre = 0, scc = 0, nod = 0;
		if(*(baseAddr + blockPtr + 5)){	//check if unicode char at offset 5 of HDR block
			//HDR non unicode
			RealignPBObject(baseAddr, blockPtr + 512, deltaOffset, &fre);	//fix the following FRE*
			RealignPBObject(baseAddr, blockPtr + 284, deltaOffset, &scc);	//fix SCC structs
			RealignPBObject(baseAddr, blockPtr + 512 + 512, deltaOffset, &nod); //fix the first following NOD*
		}
		else{
			//HDR Unicode
			RealignPBObject(baseAddr, blockPtr + 1024, deltaOffset, &fre);	//fix the following FRE*
			RealignPBObject(baseAddr, blockPtr + 1024 + 512, deltaOffset, &nod);	//fix the first following NOD*
		}
		(*items)++;
	}
	else if(!strncmp(baseAddr + blockPtr, "FRE*", 4)){
#ifdef _DEBUG
		printf("Fixing the FRE* at offset %p\n",baseAddr + blockPtr);
#endif
		if(*((unsigned long*)(baseAddr + blockPtr + 4))){
			*((unsigned long*)(baseAddr + blockPtr + 4)) += deltaOffset;				//fix the addr of next FRE* if any
			RealignPBObject(baseAddr, *((unsigned long*)(baseAddr + blockPtr + 4)), deltaOffset, items);	//then jump to it
		}
		(*items)++;
	}
	else if(!strncmp(baseAddr + blockPtr, "NOD*", 4)){
		short count;
		long ent = 0, dummy = 0;
		int p;
#ifdef _DEBUG
		printf("Fixing the NOD* at offset %p\n",baseAddr + blockPtr);
#endif
		//fix the next left block
		if(*((unsigned long*)(baseAddr + blockPtr + 4))){
			*((unsigned long*)(baseAddr + blockPtr + 4)) += deltaOffset;				//fix the addr of next left NOD* if any
			RealignPBObject(baseAddr, *((unsigned long*)(baseAddr + blockPtr + 4)), deltaOffset, items);	//then jump to it
		}
		//fix the parent block
		if(*((unsigned long*)(baseAddr + blockPtr + 8)))
			*((unsigned long*)(baseAddr + blockPtr + 8)) += deltaOffset;
		//fix the next right block
		if(*((unsigned long*)(baseAddr + blockPtr + 12))){
			*((unsigned long*)(baseAddr + blockPtr + 12)) += deltaOffset;			//fix the addr of next right NOD* if any
			RealignPBObject(baseAddr, *((unsigned long*)(baseAddr + blockPtr + 12)), deltaOffset, items);	//then jump to it
		}
		(*items)++;
		//fix the Entries
		count = *(short*)(baseAddr + blockPtr + 20);
		p = (blockPtr + 32);
		while (ent++ < count){
			RealignPBObject(baseAddr, p, deltaOffset, &dummy);	//fix one ENT*
			if (ent == count)
				break; // no need to search for a next one if it is the last
			if (baseAddr[p+5]){
				p += 24; //ENT non unicode
				while (*(baseAddr + p++)); //loop on the chars
			}
			else{
				p += 28; //ENT unicode
				while (*(baseAddr + p++)) //loop on the unicode chars
					p++;
				p++; //because on the last character of name there is 3 \00
			}
		}
	}
	else if(!strncmp(baseAddr + blockPtr, "ENT*", 4)){
		long dat = 0;
		if(*(baseAddr + blockPtr + 5)){	//check if unicode char at offset 5 of ENT block
			//ENT non unicode
#ifdef _DEBUG
			printf(" Fixing the ENT* %s at offset %p\n",baseAddr + blockPtr+24,baseAddr + blockPtr);
#endif
			*((unsigned long*)(baseAddr + blockPtr + 8)) += deltaOffset;				//fix the addr of first DAT block
			RealignPBObject(baseAddr, *((unsigned long*)(baseAddr + blockPtr + 8)), deltaOffset, &dat);	//then jump to it
		}
		else{
			//ENT unicode
#ifdef _DEBUG
			wprintf(L" Fixing the ENT* %s at offset %p\n",baseAddr + blockPtr+28,baseAddr + blockPtr);
#endif
			*((unsigned long*)(baseAddr + blockPtr + 12)) += deltaOffset;			//fix the addr of first DAT block
			RealignPBObject(baseAddr, *((unsigned long*)(baseAddr + blockPtr + 12)), deltaOffset, &dat);	//then jump to it
		}
	}
	else if(!strncmp(baseAddr + blockPtr, "DAT*", 4)){
#ifdef _DEBUG
		printf("  Fixing the DAT* at offset %p\n",baseAddr + blockPtr);
#endif
		if(*((unsigned long*)(baseAddr + blockPtr + 4))){
			*((unsigned long*)(baseAddr + blockPtr + 4)) += deltaOffset;				//fix the addr of next DAT* if any
			RealignPBObject(baseAddr, *((unsigned long*)(baseAddr + blockPtr + 4)), deltaOffset, items);	//then jump to it
		}
	}
	else
		return false;

	return true;
}

/*
From http://www.dwox.com/PBL_File_Format.txt

Breaking News:
PBL File Format will be extended in the next couple of days.
Arnd Feb. 2008
+--------------------------------------------------------------+
I PBL File Format                                              I
+--------------------------------------------------------------+
Dear PB Fans out there,

these are the results of the analysis I did, written down as
a short ASCII text description (valid thru PB5-11).

With this knowledge you can write your own LibraryDirectory
or Export Function for PowerBuilder PBL/PBD/DLL/EXE files.

Think about the possibility; including files via PBR assignment
and extracting them during runtime. That is a nice gimmick.

Most of the terms used are the results and presumptions of my
analysis.

Thanks to Kevin Cai for Bytes 17-18 of the Node-Block!

Regards

Arnd Schmidt                                            Sep 2007



arnd.schmidt@dwox.com

+--------------------------------------------------------------+
I PBL File Format                                              I
+--------------------------------------------------------------+

Rules and facts:

1.) A PBL is always made out of blocks of 512 Bytes, except the
Node Block, that has a size of 6 blocks, meaning 3072 Bytes.

2.) There is always one Header (HDR*), followed by a
free/used blocks bitmap (FRE*).
Then (after 1024 Byte) follows the first 'NOD*' block.
Theoretically this first 'NOD*' block might(!) point to a
parent node, but I have never seen that.

3.) Object Data (and SCC Informations - pre PB8) are always
stored in single forward linked/chained 'DAT*'-Blocks.

4.) A PBD is a PBL.

5.) DLL and EXE files have a 'TRL*' at the end of the file. This
is pointing to the one and only 'HDR*'-Block.

+--------------------------------------------------------------+
I Library Header Block (512 Byte)                              I
+-----------+------------+-------------------------------------+
I Pos.      I Type       I Information                         I
+-----------+------------+-------------------------------------+
I   1 - 4   I Char(4)    I 'HDR*'                              I
I   5 - 18  I String     I 'PowerBuilder' + 0x00 + 0x00        I
I  19 - 22  I Char(4)    I PBL Format Version? (0400/0500/0600)I
I  23 - 26  I Long       I Creation/Optimization Datetime      I
I  29 - ff  I String     I Library Comment                     I
I 285 - 288 I Long       I Offset of first SCC data block      I
I 289 - 292 I Long       I Size (Net size of SCC data)         I
+-----------+------------+-------------------------------------+

+--------------------------------------------------------------+
I Library Header Block - Unicode (1024 Byte)                   I
+-----------+------------+-------------------------------------+
I Pos.      I Type       I Information                         I
+-----------+------------+-------------------------------------+
I   1 - 4   I Char(4)    I 'HDR*'                              I
I   5 - 32  I StringW    I 'PowerBuilder' + 0x00 + 0x00        I
I  33 - 40  I CharW(4)   I PBL Format Version? (0400/0500/0600)I
I  41 - 44  I Long       I Creation/Optimization Datetime      I
I  45 - ff  I StringW    I Library Comment                     I
+-----------+------------+-------------------------------------+


+--------------------------------------------------------------+
I  Bitmap Block (512 Byte)                                     I
+-----------+------------+-------------------------------------+
I Pos.      I Type       I Information                         I
+-----------+------------+-------------------------------------+
I  1 - 4    I Char(4)    I 'FRE*'                              I
I  5 - 8    I Long       I Offset of next block or 0           I
I  9 - 512  I Bit(504)   I Bitmap, each Bit represents a block I
+-----------+------------+-------------------------------------+
(512 - 8) * 8 = 4032 Blocks are referenced

+--------------------------------------------------------------+
I Node Block (3072 Byte)                                       I
+-----------+------------+-------------------------------------+
I Pos.      I Type       I Information                         I
+-----------+------------+-------------------------------------+
I   1 - 4   I Char(4)    I 'NOD*'                              I
I   5 - 8   I Long       I Offset of next (left ) block or 0   I
I   9 - 12  I Long       I Offset of parent block or 0         I
I  13 - 16  I Long       I Offset of next (right) block or 0   I
I  17 - 18  I Integer    I Space left in chunks, inital = 3040 I
I  21 - 22  I Integer    I Count of entries in that node       I
I  33 - ff  I Chunks     I 'ENT*'-Chunks                       I
+-----------+------------+-------------------------------------+

+--------------------------------------------------------------+
I Entry Chunk (Variable Length)                                I
+-----------+------------+-------------------------------------+
I Pos.      I Type       I Information                         I
+-----------+------------+-------------------------------------+
I   1 - 4   I Char(4)    I 'ENT*'                              I
I   5 - 8   I Char(4)    I PBL version? (0400/0500/0600)       I
I   9 - 12  I Long       I Offset of first data block          I
I  13 - 16  I Long       I Objectsize (Net size of data)       I
I  17 - 20  I Long       I Unix datetime                       I
I  21 - 22  I Integer    I Length of comment                   I
I  23 - 24  I Integer    I Length of objectname                I
I  25 - ff  I String     I Objectname                          I
+-----------+------------+-------------------------------------+

+--------------------------------------------------------------+
I Entry Chunk - Unicode (Variable Length)                      I
+-----------+------------+-------------------------------------+
I Pos.      I Type       I Information                         I
+-----------+------------+-------------------------------------+
I   1 - 4   I Char(4)    I 'ENT*'                              I
I   5 - 12  I CharW(4)   I PBL version? (0400/0500/0600)       I
I  13 - 16  I Long       I Offset of first data block          I
I  17 - 20  I Long       I Objectsize (Net size of data)       I
I  21 - 24  I Long       I Unix datetime                       I
I  25 - 26  I Integer    I Length of comment                   I
I  27 - 28  I Integer    I Length of objectname                I
I  29 - ff  I StringW    I Objectname                          I
+-----------+------------+-------------------------------------+

+--------------------------------------------------------------+
I Data Block (512 Byte)                                        I
+-----------+------------+-------------------------------------+
I Pos.      I Type       I Information                         I
+-----------+------------+-------------------------------------+
I   1 - 4   I Char(4)    I 'DAT*'                              I
I   5 - 8   I Long       I Offset of next data block or 0      I
I   9 - 10  I Integer    I Length of data in block             I
I  11 - XXX I Blob{}     I Data (maximum Length is 502         I
+-----------+------------+-------------------------------------+

+--------------------------------------------------------------+
I Trailer Block (in DLL/EXE) always last block (512 Byte)      I
+-----------+------------+-------------------------------------+
I Pos.      I Type       I Information                         I
+-----------+------------+-------------------------------------+
I   1 - 4   I Char(4)    I 'TRL*'                              I
I   5 - 8   I Long       I Offset of Library Header ('HDR*')   I
+-----------+------------+-------------------------------------+

+--------------------------------------------------------------+
I SCC DATA                                                     I
I     Structure of status information chunks                   I
I     in DAT*-blocks (Variable Length)                         I
+---------+----------------------------------------------------I
I Type    I Information                                        I
+---------+----------------------------------------------------I
I String  I Libraryname (the opposite!)                        I
I String  I Objectname                                         I
I String  I Developername                                      I
I Char(1) I Flag                                               I
+---------+----------------------------------------------------I

+--------------------------------------------------------------+
I PB6/7 Status Flags                                           I
+------+------+------------------------------------------------+
I Icon I Flag I Meaning                                        I
+------+------+------------------------------------------------+
I      I  r   I Object is registered                           I
I      I  d   I Object is Checked Out (locked)                 I
I      I  s   I Object (Working Copy) to be checked in         I
I      I  u   I Unknown?! After an Error occurred.             I
I      I      I (Checked out by user <Unknown>                 I
I      I      I  Could be set to 'r' with an Hex-Editor.)      I
+------+------+------------------------------------------------+

DateTimes are stored in Long format in Unix representation.
Timezone is always GMT (+/- 0:00), so the datetime has to be
converted to LocalDateTime via LocalTimeZone conversation.

In the compiled object data blocks, there are at least 2 more
datetimes, starting at byte 23 and the other one at 27!
Looks like these are the modification and regeneration date...

*/
