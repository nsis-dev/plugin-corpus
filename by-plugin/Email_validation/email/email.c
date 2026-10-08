/**
 * Email plugin v.001
 *
 *   Angelo Dureghello, Trieste, Italy 09-01-2007
 */

#include <windows.h>
#include <stdio.h>

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

int CheckEmail (char *address)
{
	int			count = 0;
   const char	*c, *domain;
   static char	*rfc822_specials = "()<>@,;:\\\"[]";

	/* first we validate the name portion (name@domain) */
	for (c = address;  *c;  c++) 
	{
		if (*c == '@') break;
		if (*c <= ' ' || *c >= 127) return 0;
		if (strchr(rfc822_specials, *c)) return 0;
	}
	if (!*c) return 0;
	if (c == address || *(c - 1) == '.') return 0;
	/* next we validate the domain portion (name@domain) */
	if (!*(domain = ++c)) return 0;
	do {
		if (*c == '.') {
			if (c == domain || *(c - 1) == '.') return 0;
			count++;
		}
		if (*c <= ' ' || *c >= 127) return 0;
		if (strchr(rfc822_specials, *c)) return 0;
	} 
	while (*++c);

	return 1;
}

void __declspec(dllexport) EmailValidation(HWND hwndParent, int string_size, 
                                      char *variables, stack_t **stacktop)
{
	char			address[MAX_STRLEN]="";

	EXDLL_INIT();
	{
		popstring(address);


		switch(CheckEmail(address))
		{
			default:
			case 0: pushstring("failed");		break;
			case 1: pushstring("ok");			break;
		}
	}
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	g_hInstance=hInst;

	return TRUE;
}
