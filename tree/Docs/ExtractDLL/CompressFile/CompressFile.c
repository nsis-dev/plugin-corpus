#include "windows.h"
#include "stdio.h"
#include "..\zlib\zlib.h"

int main(int argc, char *argv[ ], char *envp[ ])
{
	HANDLE hDest, hSrc;
	
	if (argc!=3)
	{
		printf("wrong number of arguments");
		return -1;
	}
	
	hDest=CreateFile(argv[2], GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if (hDest==INVALID_HANDLE_VALUE)
	{
		printf("failed to open destination file\n");
		return -1;
	}
	
	hSrc=CreateFile(argv[1], GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
	if (hSrc==INVALID_HANDLE_VALUE)
	{
		CloseHandle(hDest);
		printf("failed to open source file\n");
		return -1;
	}
	
	#ifdef NSIS_COMPRESS_USE_ZLIB
	{
		z_stream deflate_stream;
		DWORD rlen, wlen;
		char inbuffer[4096], outbuffer[4096];
		
		deflateInit(&deflate_stream, Z_BEST_COMPRESSION);
		
		if (!ReadFile(hSrc, inbuffer, 4096, &rlen, 0))
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
					CloseHandle(hSrc);
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
					CloseHandle(hSrc);
					return -1;
				}

				if (err==Z_STREAM_END)
				{
					CloseHandle(hDest);
					CloseHandle(hSrc);
					printf("Successful\n");
					return 0;
				}
				if (u<4096)
					break;
			}
			if (!ReadFile(hSrc, inbuffer, 4096, &rlen, 0))
				rlen=0;
		}
	}
#endif //NSIS_COMPRESS_USE_ZLIB
	CloseHandle(hDest);
	CloseHandle(hSrc);
	printf("Successful\n");
	return 0;
}