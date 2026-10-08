/* NSIS plug-in library
 * By Daniel Collins <solemnwarning@solemnwarning.net>
 *
 * This code is public domain, I grant permission for anyone to use, modify or
 * redistribute this code in any way, for any purpose in any application, under
 * any license.
 *
 * This code has NO WARRANTY, if it fails to work, causes your program to crash
 * or causes any damage, I accept NO RESPONSIBILITY for it. But if you find a
 * bug, email me and I will try to fix it :)
*/

/* Version history:
 *
 * Version 1.0 (14/09/2008)
 *	Initial release
*/

#include <windows.h>
#include <stdlib.h>
#include <string.h>

#define NSIS_VARSIZE 1024

typedef __declspec(dllexport) void nsis_function;

typedef struct _stack_t {
	struct _stack_t *next;
	char text[NSIS_VARSIZE];
} stack_t;

static char *nsis_vars = NULL;
static stack_t **nsis_stack = NULL;

/* Initialize anything required for these functions to work
 * Call this at the start of all plugin functions
*/
static void nsis_init(HWND hwnd, int vsize, char *vars, stack_t **stack) {
	if(vsize != NSIS_VARSIZE) {
		MessageBox(
			hwnd,
			"NSIS_VARSIZE is incorrect!",
			"Plug-in error",
			MB_OK | MB_ICONERROR
		);
		
		exit(1);
	}
	if(!stack) {
		MessageBox(
			hwnd,
			"The stack pointer is NULL!",
			"Plug-in error",
			MB_OK | MB_ICONERROR
		);
		
		exit(1);
	}
	
	nsis_vars = vars;
	nsis_stack = stack;
}

/* Get a pointer to a builtin NSIS variable
 * Returns NULL if an unknown variable is requested
*/
static char *nsis_var(char const *name) {
	char* varnames[] = {
		"0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
		"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8", "R9",
		"CMDLINE",
		"INSTDIR",
		"OUTDIR",
		"EXEDIR",
		"LANG",
		NULL
	};
	
	int n;
	
	for(n = 0; varnames[n]; n++) {
		if(strcmp(name, varnames[n]) == 0) {
			return nsis_vars + (NSIS_VARSIZE * n);
		}
	}
	
	return NULL;
}

/* Push a string onto the stack
 * Returns 1 on success, zero if the GlobalAlloc() call fails
*/
static int nsis_push(char const *value) {
	stack_t *sptr;
	
	sptr = GlobalAlloc(GPTR, sizeof(stack_t));
	if(!sptr) {
		return 0;
	}
	
	strncpy(sptr->text, value, NSIS_VARSIZE);
	sptr->text[NSIS_VARSIZE-1] = '\0';
	
	sptr->next = *nsis_stack;
	*nsis_stack = sptr;
	
	return 1;
}

/* Pop a string off the stack and copy it to buf
 * Returns 1 on success, zero if there was nothing on the stack
 *
 * This function may be used to get arguments which were passed to the calling
 * function, since arguments are pushed onto the stack.
*/
static int nsis_pop(char *buf) {
	stack_t *sptr;
	
	if(!*nsis_stack) {
		return 0;
	}
	
	sptr = *nsis_stack;
	strcpy(buf, sptr->text);
	
	*nsis_stack = sptr->next;
	GlobalFree(sptr);
	
	return 1;
}
