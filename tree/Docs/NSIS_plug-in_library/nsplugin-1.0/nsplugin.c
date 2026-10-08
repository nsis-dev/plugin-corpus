/* Example NSIS plug-in
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

#include <windows.h>
#include "nsplugin.h"

nsis_function hello(HWND hwnd, int vsize, char *vars, stack_t **stack) {
	nsis_init(hwnd, vsize, vars, stack);
	
	MessageBox(
		hwnd,
		"Read the comments in nsplugin.h",
		"Hello world",
		MB_OK
	);
}
