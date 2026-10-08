#undef NSIS_COMPRESS_USE_ZLIB
#undef NSIS_COMPRESS_USE_LZMA

#define NSIS_COMPRESS_USE_LZMA

#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include "zlib\zlib.h"
#include "lzma\LzmaDecode.h"

typedef struct _stack_t {
  struct _stack_t *next;
  char text[1]; // this should be the length of string_size
} stack_t;

int popstring(char *str); // 0 on success, 1 on empty stack
void pushstring(char *str);
int LogMessage(const char *pStr);
int SetStatus(const char *pStr);

enum
{
INST_0,         // $0
INST_1,         // $1
INST_2,         // $2
INST_3,         // $3
INST_4,         // $4
INST_5,         // $5
INST_6,         // $6
INST_7,         // $7
INST_8,         // $8
INST_9,         // $9
INST_R0,        // $R0
INST_R1,        // $R1
INST_R2,        // $R2
INST_R3,        // $R3
INST_R4,        // $R4
INST_R5,        // $R5
INST_R6,        // $R6
INST_R7,        // $R7
INST_R8,        // $R8
INST_R9,        // $R9
INST_CMDLINE,   // $CMDLINE
INST_INSTDIR,   // $INSTDIR
INST_OUTDIR,    // $OUTDIR
INST_EXEDIR,    // $EXEDIR
__INST_LAST
};

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

char *getuservariable(int varnum);


HINSTANCE g_hInstance;
HWND g_hwndParent, g_hwndDlg, g_hwndList;
int g_stringsize;
stack_t **g_stacktop;
char *g_variables;

#include <commctrl.h>

void __declspec(dllexport) extract(HWND hwndParent, int string_size, 
                                      char *variables, stack_t **stacktop)
{
	g_hwndParent=hwndParent;
	g_stringsize=string_size;
	g_stacktop=stacktop;
	g_variables=variables;
	g_hwndDlg = g_hwndList = 0;
	
	//Extract file to destination
	{
	  char destination[MAX_PATH+1];
	  char source[MAX_PATH+1];
	  HANDLE hDest, hSrc;
	  char path[MAX_PATH];
	  char buffer[4096];
	  DWORD pos = 0, length;
	  DWORD rlen, wlen;
	  int newpos = 0;
	    	
	  popstring(destination);
	  popstring(source);

	  hDest=CreateFile(destination, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	  if (hDest==INVALID_HANDLE_VALUE)
	  {
		  pushstring("Could not open destination file.");
		  return;
	  }

	  hSrc=CreateFile(source, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
	  if (hSrc==INVALID_HANDLE_VALUE)
	  {
		  CloseHandle(hDest);
		  pushstring("Could not open input file.");
		  return;
	  }
	  
#ifdef NSIS_COMPRESS_USE_ZLIB
		{
			z_stream inflate_stream;
			DWORD rlen, wlen;
			char inbuffer[4096], outbuffer[4096];

			inflateInit(&inflate_stream);
			
			if (!ReadFile(hSrc, inbuffer, 4096, &rlen, 0))
				rlen=0;
			while (rlen>0)
			{
				inflate_stream.next_in = inbuffer;
				inflate_stream.avail_in = rlen;
				for (;;)
				{
					int err;
					int u;
					inflate_stream.next_out = outbuffer;
					inflate_stream.avail_out = 4096;
					
					err=inflate(&inflate_stream);
					
					if (err<0)
					{
						CloseHandle(hDest);
						CloseHandle(hSrc);
						pushstring("Inflate failed, maybe file is corrupt");
						return;
					}
					
					u=(char*)inflate_stream.next_out - outbuffer;
					
					if (!u) break;
					
					if (!WriteFile(hDest, outbuffer, u, &wlen, 0))
						wlen=0;
					if (wlen!=u)
					{
						CloseHandle(hDest);
						CloseHandle(hSrc);
						pushstring("Could not write to destination file");
						return;
					}
					
					if (err==Z_STREAM_END)
					{
						CloseHandle(hDest);
						CloseHandle(hSrc);
						pushstring("success");
					}
				}
				if (!ReadFile(hSrc, inbuffer, 4096, &rlen, 0))
					rlen=0;
			}
		}
#endif //NSIS_COMPRESS_USE_ZLIB

#ifdef NSIS_COMPRESS_USE_LZMA
    {
      unsigned int compressedSize, outSize, outSizeProcessed, lzmaInternalSize;
	    unsigned char properties[5];
      unsigned char prop0;
      int i;
      int lc, lp, pb;
      unsigned char b;
      void *inStream, *outStream, *lzmaInternalData;
      int res;
      DWORD rlen, pos = 0;
      unsigned int crctable[256];
      unsigned int crcsumorg = 0, crcsumcal = 0;
      char buffer[4096];
      
      length = SetFilePointer(hSrc, 0, 0, FILE_END) - 4;
	    if (length==INVALID_SET_FILE_POINTER)
	    {
	      CloseHandle(hDest);
		    CloseHandle(hSrc);
		    pushstring("Could not read input file.");
		    return;
	    }
	    
	    // get the checksum
	    SetFilePointer(hSrc, -4, 0, FILE_END);
	    
	    if (!ReadFile(hSrc, &crcsumorg, sizeof(crcsumorg), &rlen, 0)) {
        CloseHandle(hDest);
				CloseHandle(hSrc);
				pushstring("Could not read crc sum from input file.");
				return;
      }	    
      
	    SetFilePointer(hSrc, 0, 0, FILE_BEGIN);
	    
	    InitCRC32(crctable);
      
      if (!ReadFile(hSrc, buffer, 4096, &rlen, 0))
			  rlen=0;
      
      while (rlen>0) {
        pos += rlen;
        
        crcsumcal += CreateCRC32(crctable, buffer, (pos > length) ? rlen - 4 : rlen);
	          
  	    if (!ReadFile(hSrc, buffer, 4096, &rlen, 0))
				  rlen=0;
      }
      
      if (crcsumorg != crcsumcal) {  
        CloseHandle(hDest);
				CloseHandle(hSrc);
				pushstring("CRC checksum test failed.");
				return;
      }
      
      SetFilePointer(hSrc, 0, 0, FILE_BEGIN);
      
      if (!ReadFile(hSrc, properties, sizeof(properties), &rlen, 0)) {
        CloseHandle(hDest);
				CloseHandle(hSrc);
				pushstring("Could not read input file.");
				return;
      }
      
      outSize = 0;
      for (i = 0; i < 4; i++)
      {
        if (!ReadFile(hSrc, &b, 1, &rlen, 0)) {
          CloseHandle(hDest);
					CloseHandle(hSrc);
					pushstring("Could not read input file.");
					return;
        }
        outSize += (unsigned int)(b) << (i * 8);
      }

      if (outSize == 0xFFFFFFFF)
      {
        CloseHandle(hDest);
				CloseHandle(hSrc);
				pushstring("Stream version is not supported.");
				return;
      }

      for (i = 0; i < 4; i++)
      {
        if (!ReadFile(hSrc, &b, 1, &rlen, 0)) {
          CloseHandle(hDest);
					CloseHandle(hSrc);
					pushstring("Could not read input file.");
					return;
        }
        if (b != 0)
        {
          CloseHandle(hDest);
					CloseHandle(hSrc);
          pushstring("File too long.");
					return;
        }
      }

      compressedSize = length - 13;
      inStream = malloc(compressedSize);
      if (inStream == 0)
      {
        CloseHandle(hDest);
				CloseHandle(hSrc);
        pushstring("Could not allocate enough memory.");
				return;
      }
      if (!ReadFile(hSrc, inStream, compressedSize, &rlen, 0)) {
        CloseHandle(hDest);
				CloseHandle(hSrc);
				free(inStream);
				pushstring("Could not read input file.");
				return;
      }
      
      prop0 = properties[0];
      if (prop0 >= (9*5*5))
      {
        CloseHandle(hDest);
        free(inStream);
        pushstring("Properties error.");
				return;
      }
      for (pb = 0; prop0 >= (9 * 5); 
        pb++, prop0 -= (9 * 5));
      for (lp = 0; prop0 >= 9; 
        lp++, prop0 -= 9);
      lc = prop0;

      lzmaInternalSize = (LZMA_BASE_SIZE + (LZMA_LIT_SIZE << (lc + lp))) * sizeof(CProb); 

      outStream = malloc(outSize);
      lzmaInternalData = malloc(lzmaInternalSize);
      if (outStream == 0 || lzmaInternalData == 0)
      {
        CloseHandle(hDest);
        free(inStream);
        pushstring("Could not allocate enough memory.");
				return;
      }
      
      res = LzmaDecode((unsigned char *)lzmaInternalData, lzmaInternalSize, lc, lp, pb, (unsigned char *)inStream, compressedSize, (unsigned char *)outStream, outSize, &outSizeProcessed);
      outSize = outSizeProcessed;

      if (res != 0)
      {
        CloseHandle(hDest);
        free(inStream);
        free(outStream);
        free(lzmaInternalData);
        pushstring("Unpacking failed, maybe file is corrupt.");
				return;
      }

      if (!WriteFile(hDest, outStream, outSize, &wlen, 0)) {
        CloseHandle(hDest);
        free(inStream);
        free(outStream);
        free(lzmaInternalData);
        pushstring("Could not write to destination file.");
				return;      
      }
      
      free(lzmaInternalData);
      free(outStream);
      free(inStream);
    }
#endif  // NSIS_COMPRESS_USE_LZMA

    CloseHandle(hSrc);
    CloseHandle(hDest);

	  // split files
    hSrc=CreateFile(destination, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
	  if (hSrc==INVALID_HANDLE_VALUE)
	  {
		  CloseHandle(hSrc);
		  pushstring("Could not open input file for splittet files.");
		  return;
	  }
  	
	  while (newpos != INVALID_SET_FILE_POINTER)
	  {	
	    if (!ReadFile(hSrc, buffer, 4096, &rlen, 0) || rlen<=5)
	    {
	      CloseHandle(hSrc);
	      
	      // end loop
	      break;
	    }
    	
	    pos = 0;
	    while (NULL != strchr(buffer + pos, '/'))
	    {
	      // create directory if needed
	      pos = strchr(buffer + pos, '/') - buffer + 1;
	      
	      strncpy(path, buffer, pos);
        path[pos] = '\0';
        
        CreateDirectory(path, NULL);
	    }
	    
	    strcpy(path, buffer);
	    length = *(DWORD*)&buffer[strlen(path) + 1];
	    pos = 0;
	    
	    hDest=CreateFile(path, GENERIC_READ|GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	    if (hDest==INVALID_HANDLE_VALUE)
	    {
		    CloseHandle(hSrc);
		    pushstring("Could not open splitted destination file.");
		    return;
	    }
    	
	    SetFilePointer(hSrc, newpos + strlen(path) + 5, 0, FILE_BEGIN);
  	  
	    if (!ReadFile(hSrc, buffer, 4096, &rlen, 0))
	      rlen=0;
    	
	    while (rlen>0)
	    {
	      if (!WriteFile(hDest, buffer, (pos + rlen) <= length ? rlen : length - pos, &wlen, 0) ||
	          ((pos + rlen) <= length ? rlen : length - pos) != wlen)
	      {
	        CloseHandle(hDest);
		      CloseHandle(hSrc);
		      pushstring("Could not copy splitted destination file");
		      return;	       	 
	      }
  	    
	      pos += wlen;
  	    
	      if (pos >= length) {
	        rlen=0;
	      } else {
	        if (!ReadFile(hSrc, buffer, 4096, &rlen, 0))
			      rlen=0;
			  }
	    }
    	
	    CloseHandle(hDest);
    	
	    newpos = SetFilePointer(hSrc, newpos + strlen(path) + 5 + length, 0, FILE_BEGIN);
	  }
  	
	  CloseHandle(hSrc);
	  
	  DeleteFile(destination);		
  	
	  pushstring("success");
	}
}

BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
  g_hInstance=hInst;
	return TRUE;
}


// utility functions (not required but often useful)
int popstring(char *str)
{
  stack_t *th;
  if (!g_stacktop || !*g_stacktop) return 1;
  th=(*g_stacktop);
  lstrcpy(str,th->text);
  *g_stacktop = th->next;
  GlobalFree((HGLOBAL)th);
  return 0;
}

void pushstring(char *str)
{
  stack_t *th;
  if (!g_stacktop) return;
  th=(stack_t*)GlobalAlloc(GPTR,sizeof(stack_t)+g_stringsize);
  lstrcpyn(th->text,str,g_stringsize);
  th->next=*g_stacktop;
  *g_stacktop=th;
}

char *getuservariable(int varnum)
{
  if (varnum < 0 || varnum >= __INST_LAST) return NULL;
  return g_variables+varnum*g_stringsize;
}


