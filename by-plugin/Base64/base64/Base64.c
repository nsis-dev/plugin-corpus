/**
 * Base64 plugin v.001
 *
 *   Angelo Dureghello, Trieste, Italy 09-01-2007
 */

#include <windows.h>
#include <stdio.h>

#define FORCE_SWITCH  "/FORCE"
#define NOSAFE_SWITCH "/NOSAFE"
#define MAX_STRLEN       1024

/* NSIS stack structure */
typedef struct _stack_t {
	struct	_stack_t *next;
	char		text[MAX_STRLEN];
} stack_t;

stack_t			**g_stacktop;
char				*g_variables;
unsigned int	g_stringsize;
HINSTANCE		g_hInstance;

#define EXDLL_INIT()					\
{											\
	g_stacktop		= stacktop;    \
	g_variables		= variables;   \
	g_stringsize	= string_size; \
}

//Function: Removes the element from the top of the NSIS stack and puts it in the buffer
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

//Function: Adds an element to the top of the NSIS stack
void pushstring(const char *str)
{
	stack_t *th;

	if (!g_stacktop) return;
	
	th=(stack_t*)GlobalAlloc(GPTR, sizeof(stack_t)+g_stringsize);
	lstrcpyn(th->text,str,g_stringsize);
	th->next=*g_stacktop;
	
	*g_stacktop=th;
}

unsigned char c2c64(unsigned char c) 
{

  if(c==63) return '/';
  if(c==62) return '+';
  if(c>51) return '0'+c-52;
  if(c>25) return 'a'+c-26;
  
  return 'A'+c;
}

unsigned char c642c(unsigned char c) 
{
  if(c=='/') return 63;
  if(c=='+') return 62;
  if(c>'Z') return 26+c-'a';
  if(c>'9') return c-'A';
  
  return c-'0'+(26*2);
}
 
unsigned int Decode64(unsigned char *dest, unsigned char *src, unsigned int len) 
{
	unsigned int a;
	unsigned char *p;
	unsigned char b;
   p=dest;
	
	for(a=0;a<len && *src && *src!='=';a++) {
		b=c642c(*src++);
		switch(a&0x03) {
			case 0x00: *p=b<<2; break;
			case 0x01: *p|=(b>>4); p++; *p=b<<4; break;
			case 0x02: *p|=(b>>2); p++; *p=b<<6; break;
			case 0x03: *p|=b; p++; break;
		}
	}
	//angelo, patch 
	*p=0;

	return p-dest;
}

unsigned int Encode64(unsigned char *dest, unsigned char *src, unsigned int len) 
{
	unsigned int a,cnt;
	
	a=cnt=0;
	while(cnt<len) {
		
		dest[0] = c2c64(src[0] >> 2);
		dest[1] = c2c64(((src[0] & 3) << 4) | (src[1] >> 4));
		dest[2] = c2c64(((src[1] & 0xF) << 2) | (src[2] >> 6));
		dest[3] = c2c64(src[2] & 0x3F);

		cnt+=3;
		a+=4;
		src+=3;
		dest+=4;
	}

	switch((unsigned char)(cnt-len)) {
		case 2: dest[-2]='='; // no break;
		case 1: dest[-1]='='; break;
	}		
		
	return a;
}


void __declspec(dllexport) Encrypt(HWND hwndParent, int string_size, 
                                      char *variables, stack_t **stacktop)
{
	char				string_to_encode	[MAX_STRLEN]="";
	char				string_encoded		[MAX_STRLEN]="";
	char				length				[16]="";

	EXDLL_INIT();
	{
		popstring	(string_to_encode);
		popstring	(length);

		Encode64		(string_encoded, string_to_encode, atoi(length));

		pushstring	(string_encoded);
	}
}

void __declspec(dllexport) Decrypt(HWND hwndParent, int string_size, 
                                      char *variables, stack_t **stacktop)
{
	char				string_to_decode	[MAX_STRLEN]="";
	char				string_decoded		[MAX_STRLEN]="";
	char				length				[16]="";

	EXDLL_INIT();
	{
		popstring	(string_to_decode);
	   popstring	(length);

		Decode64		(string_decoded, string_to_decode, atoi(length));

		pushstring	(string_decoded);	
	}
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	g_hInstance=hInst;

	return TRUE;
}
