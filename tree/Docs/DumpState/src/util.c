
#include <windows.h>
#include "util.h"

#ifndef _ass
void _ass(const char *file, int line, const char *cond) {
	char *def = "couldn't globalalloc!";
	char *buffer = GlobalAlloc(GPTR,8192);
	if(buffer) 
		wsprintfA(buffer, "%s:%d\n\n%s", file, line, cond);
	else
		buffer = def;
	
	MessageBoxA(NULL, buffer, "Assertion failed", MB_OK);

	if(buffer != def)
		GlobalFree(buffer);

}
#endif

void dprintf(const char *fmt, ...) {
	va_list args;
	char *def = "couldn't globalalloc!";
	char *buffer = (char *)GlobalAlloc(GPTR, 8192);
	
	va_start(args, fmt);
	if(buffer)
		wvsprintfA(buffer, fmt, args);
	else
		buffer = def;
	va_end(args);

	OutputDebugStringA(buffer);

	if(buffer != def)
		GlobalFree(buffer);
}

INT_PTR StrToPtr(LPCTSTR s, INT_PTR*pParsedAny)
{
	INT_PTR v = 0, sign = 0, base = 10, allowOr = 1, parsedNum = 0;
	if (*s == ('0') && (s[1] == ('x') || s[1] == ('X')))
	{
		for (s++;;) {
			int c = *(++s);
			if (c >= ('0') && c <= ('9')) c -= ('0');
			else if (c >= ('a') && c <= ('f')) c -= ('a')-10;
			else if (c >= ('A') && c <= ('F')) c -= ('A')-10;
			else break;
			v <<= 4, v += c, ++parsedNum;
		}
	}
	else {
		if (*s == ('-')) sign++; else s--;
		for (;;) {
			int c = *(++s) - ('0');
			if (c < 0 || c >= base) break;
			v *= base, v+=c, ++parsedNum;
		}
	}
	if (sign) v = -v;
	if (allowOr && *s == ('|')) v |= StrToPtr(s+1, pParsedAny); // Support for simple ORed expressions
	if (pParsedAny) *pParsedAny = parsedNum != 0;
	return v;
}