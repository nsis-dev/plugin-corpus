/*****************************************************************
 *               name2ip NSIS plugin v1.0                        *
 *                                                               *
 * 2006 Shengalts Aleksander aka Instructor (Shengalts@mail.ru)  *
 *****************************************************************/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock.h>

/* Defines */
#define NSIS_MAX_STRLEN 1024

/* ExDll */
typedef struct _stack_t {
  struct _stack_t *next;
  char text[NSIS_MAX_STRLEN];
} stack_t;

stack_t **g_stacktop;
char *g_variables;
unsigned int g_stringsize;

#define EXDLL_INIT()        \
{                           \
  g_stacktop=stacktop;      \
  g_variables=variables;    \
  g_stringsize=string_size; \
}

/* Global variables */
char szHostName[32];
WSADATA wsaData;
struct hostent *lpHost;
int nAddrIndex;

/* Funtions prototypes and macros */
int popstring(char *str);
void pushstring(const char *str);

/* NSIS functions code */
void __declspec(dllexport) FindFirstIP(HWND hwndParent, int string_size,
                                      char *variables, stack_t **stacktop)
{
  EXDLL_INIT();
  {
    nAddrIndex=0;

    popstring(szHostName);

    if (WSAStartup(MAKEWORD(2,2), &wsaData) == NO_ERROR &&
        (*szHostName || !gethostname(szHostName, sizeof(szHostName))) &&
        (lpHost=gethostbyname(szHostName)))
    {
      pushstring(inet_ntoa(*(struct in_addr *)lpHost->h_addr_list[nAddrIndex++]));
    }
    else
    {
      pushstring("");
    }
  }
}

void __declspec(dllexport) FindNextIP(HWND hwndParent, int string_size,
                                      char *variables, stack_t **stacktop)
{
  EXDLL_INIT();
  {
    if ((lpHost) && lpHost->h_addr_list[nAddrIndex])
    {
      pushstring(inet_ntoa(*(struct in_addr *)lpHost->h_addr_list[nAddrIndex++]));
    }
    else
    {
      pushstring("");
    }
  }
}

void __declspec(dllexport) FindCloseIP(HWND hwndParent, int string_size,
                                      char *variables, stack_t **stacktop)
{
  WSACleanup();
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
  return TRUE;
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
  th=(stack_t*)GlobalAlloc(GPTR,sizeof(stack_t)+g_stringsize);
  lstrcpyn(th->text,str,g_stringsize);
  th->next=*g_stacktop;
  *g_stacktop=th;
}
