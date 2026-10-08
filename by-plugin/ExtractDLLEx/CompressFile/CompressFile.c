#undef NSIS_COMPRESS_USE_ZLIB
#undef NSIS_COMPRESS_USE_LZMA

#define NSIS_COMPRESS_USE_LZMA

#define _WIN32_WINNT 0x510

#include <windows.h>
#include <Wincon.h>
#include <stdio.h>
#include <string.h>
#include "..\zlib\zlib.h"
#include "..\lzma\LzmaDecode.h"

void InitCRC32(DWORD* table) 
{
	register int i, n;
	register DWORD crc;

	if (NULL == table) return;

	for (i = 0; i < 256; i++) {
		crc = i << 24;

    for (n = 0; n < 8; n++) {
			if (crc & 0x80000000) crc = (crc << 1) ^ 0x04C11DB7;
      else crc = crc << 1;
    }

		table[i] = crc;
  }
}

unsigned int CreateCRC32(DWORD* table, unsigned char *data, int len) 
{
	register DWORD result;
	register int i;

	if (NULL == table || len < 4) return 0;

	result = *data++ << 24;
	result |= *data++ << 16;
	result |= *data++ << 8;
	result |= *data++;
	result = ~result;
	len -= 4;

	for (i = 0; i < len; i++) {
		result = (result << 8 | *data++) ^ table[result >> 24];
	}

	return ~result;
}

int main(int argc, char *argv[ ], char *envp[ ])
{
	HANDLE hDest, hSrc, hTmp;
	DWORD i;
	char buffer[4096];
	DWORD pos = 0, length;
	DWORD rlen, wlen;
	SHELLEXECUTEINFO shellinfo;
	WIN32_FIND_DATA data;
	HANDLE handle;
	char path[MAX_PATH];
	DWORD crctable[256];
	DWORD crcsum = 0;
	
	if (argc < 3)
	{
		printf("CompressFile destination source [source] [[source] [[...]]]\r\n\r\n- use always relative paths! e.g. test/test.txt\r\n- use / instead of \\ for paths\r\n- do not begin relative paths with /\r\n- .. in relative paths is not supported\r\n- you can use wildcards like * and ?\r\n");
		return -1;
	}
	
	hDest=CreateFile(argv[1], GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if (hDest==INVALID_HANDLE_VALUE)
	{
		printf("failed to open destination file %s\n", argv[1]);
		return -1;
	}
	
	hTmp=CreateFile("pack.tmp", GENERIC_READ|GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if (hTmp==INVALID_HANDLE_VALUE)
	{
	  CloseHandle(hDest);
	  DeleteFile(argv[1]);
		printf("failed to open temp file pack.tmp\n");
		return -1;
	}
	
	for (i = 2; (int)i < argc; i++) {
	  handle = FindFirstFile(argv[i], &data);
	  
	  if (handle != INVALID_HANDLE_VALUE)
    {
      do
      {
        if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
          if (NULL != strrchr(argv[i], '/'))
          {
            strncpy(path, argv[i], strrchr(argv[i], '/') - argv[i] + 1);
            path[strrchr(argv[i], '/') - argv[i] + 1] = '\0';
            strcat(path, data.cFileName);
          } 
          else 
          {
            strcpy(path, data.cFileName);
            path[strlen(data.cFileName)] = '\0';
          }
                    
          hSrc=CreateFile(path, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
	        if (hSrc==INVALID_HANDLE_VALUE)
	        {
		        CloseHandle(hDest);
		        CloseHandle(hTmp);
		        DeleteFile("pack.tmp");
		        DeleteFile(argv[1]);
		        printf("failed to open source file %s\n", path);
		        return -1;
	        }
      	  
	        length = SetFilePointer(hSrc, 0, 0, FILE_END);
	        if (length==INVALID_SET_FILE_POINTER)
	        {
	          CloseHandle(hDest);
		        CloseHandle(hTmp);
		        DeleteFile("pack.tmp");
		        DeleteFile(argv[1]);
		        printf("failed to get length of source file %s\n", path);
		        return -1;
	        }
      	  
	        SetFilePointer(hSrc, 0, 0, FILE_BEGIN);
      	  
	        if (WriteFile(hTmp, path, strlen(path) + 1, &wlen, 0)) {	  
	          if (WriteFile(hTmp, &length, sizeof(length), &wlen, 0)) {
	            // merge the sources
	            if (!ReadFile(hSrc, buffer, 4096, &rlen, 0))
			          rlen=0;
      		
		          while (rlen>0)
		          {
	       	      if (!WriteFile(hTmp, buffer, rlen, &wlen, 0) ||
	       	          rlen != wlen)
	       	      {
	       	        CloseHandle(hDest);
		              CloseHandle(hTmp);
		              CloseHandle(hSrc);
		              DeleteFile("pack.tmp");
		              DeleteFile(argv[1]);
		              printf("failed to merge source files %s\n", path);
		              return -1;	       	 
	       	      }
        	      
  	            if (!ReadFile(hSrc, buffer, 4096, &rlen, 0))
				          rlen=0;
	            }
	          }
	        }
      	  
	        CloseHandle(hSrc);
	      }
	    }
	    while (FindNextFile(handle, &data));
    } else {
      FindClose(handle);
      CloseHandle(hDest);
		  CloseHandle(hTmp);
		  DeleteFile("pack.tmp");
		  DeleteFile(argv[1]);
		  printf("failed to open source file %s\n", argv[i]);
		  return -1;
    }
    
    FindClose(handle);
	}
	
	SetFilePointer(hTmp, 0, 0, FILE_BEGIN);
	
#ifdef NSIS_COMPRESS_USE_ZLIB
	{
		z_stream deflate_stream;
		char inbuffer[4096], outbuffer[4096];
		
		deflateInit(&deflate_stream, Z_BEST_COMPRESSION);
		
		if (!ReadFile(hTmp, inbuffer, 4096, &rlen, 0))
			rlen=0;
		while (rlen>0)
		{
			deflate_stream.next_in = inbuffer;
			deflate_stream.avail_in = rlen;
			for (;;)
			{
				int err;
				int u;
				deflate_stream.next_out = outbuffer;
				deflate_stream.avail_out = 4096;
				
				err=deflate(&deflate_stream, Z_SYNC_FLUSH);
				
				if (err<0)
				{
					CloseHandle(hDest);
					CloseHandle(hTmp);
					printf("Failure\n");
					return -1;
				}
				
				u=(char*)deflate_stream.next_out - outbuffer;
				
				if (!u) break;
				
				if (!WriteFile(hDest, outbuffer, u, &wlen, 0))
					wlen=0;
				if (wlen!=u)
				{
					printf("Failure\n");
					CloseHandle(hDest);
					CloseHandle(hTmp);
					return -1;
				}

				if (err==Z_STREAM_END)
				{
					CloseHandle(hDest);
					CloseHandle(hTmp);
					printf("Successful\n");
					return 0;
				}
				if (u<4096)
					break;
			}
			if (!ReadFile(hTmp, inbuffer, 4096, &rlen, 0))
				rlen=0;
		}
	}
#endif //NSIS_COMPRESS_USE_ZLIB

  CloseHandle(hDest);
	CloseHandle(hTmp);

#ifdef NSIS_COMPRESS_USE_LZMA
  {
    hTmp=CreateFile("lzma.exe", GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
	  if (hTmp==INVALID_HANDLE_VALUE)
	  {
	    printf("failed to open lzma.exe\n");
	    DeleteFile("pack.tmp");
		  DeleteFile(argv[1]);
	    return -1;
	  }
	  
	  CloseHandle(hTmp);
	  
	  strcpy(buffer, "e pack.tmp ");
    strcat(buffer, argv[1]);
    
	  shellinfo.cbSize = sizeof(SHELLEXECUTEINFO);
	  shellinfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	  shellinfo.hwnd = GetConsoleWindow();
	  shellinfo.lpVerb = "open";
	  shellinfo.lpFile = "lzma.exe";
	  shellinfo.lpParameters = buffer;
	  shellinfo.lpDirectory = NULL;
	  shellinfo.nShow = SW_SHOWNORMAL;
	      
    ShellExecuteEx(&shellinfo);
    WaitForSingleObject(shellinfo.hProcess, INFINITE);
    
    // add checksum to the file because lzma do not have a checksum!
    InitCRC32(crctable);
    
    hTmp=CreateFile(argv[1], GENERIC_READ|GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
	  if (hTmp==INVALID_HANDLE_VALUE)
	  {
	    printf("failed to open temp file %s\n", argv[1]);
	    DeleteFile("pack.tmp");
		  DeleteFile(argv[1]);
		  return -1;
	  }
	  
	  if (!ReadFile(hTmp, buffer, 4096, &rlen, 0))
			rlen=0;

		while (rlen>0)
		{
	    crcsum += CreateCRC32(crctable, buffer, rlen);
	          
  	  if (!ReadFile(hTmp, buffer, 4096, &rlen, 0))
				rlen=0;
	  }
	  
	  SetFilePointer(hTmp, 0, 0, FILE_END);
	  
	  if (!WriteFile(hTmp, &crcsum, sizeof(crcsum), &wlen, 0))
	  {
	    CloseHandle(hTmp);
	    DeleteFile("pack.tmp");
		  DeleteFile(argv[1]);
		  printf("failed to add crc sum to %s\n", argv[1]);
		  return -1;	       	 
	  }
	  
	  CloseHandle(hTmp);
  }
#endif //NSIS_COMPRESS_USE_LZMA

  DeleteFile("pack.tmp");
  
  printf("Successful\n");
	return 0;
}