extern "C" {

#include "api.h"

#  define NSISCALL __stdcall

#define EXDLL_INIT()           {  \
        g_stringsize=string_size; \
        g_stacktop=stacktop;      \
        g_variables=variables; }

typedef struct _stack_t {
  struct _stack_t *next;
  char text[1]; // this should be the length of string_size
} stack_t;

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
INST_LANG,      // $LANGUAGE
__INST_LAST
};

static unsigned int g_stringsize;
static stack_t **g_stacktop;
static char *g_variables;

void NSISCALL pushstring(const char *str);
//char * NSISCALL getuservariable(const int varnum);
//void NSISCALL setuservariable(const int varnum, const char *var);

static void __stdcall pushstring(const char *str)
{
  stack_t *th;
  if (!g_stacktop) return;
  th=(stack_t*)GlobalAlloc(GPTR,sizeof(stack_t)+g_stringsize);
  lstrcpynA(th->text,str,g_stringsize);
  th->next=*g_stacktop;
  *g_stacktop=th;
}

//static char * __stdcall getuservariable(const int varnum)
//{
//  if (varnum < 0 || varnum >= __INST_LAST) return NULL;
//  return g_variables+varnum*g_stringsize;
//}
//
//static void __stdcall setuservariable(const int varnum, const char *var)
//{
//	if (var != NULL && varnum >= 0 && varnum < __INST_LAST) 
//		lstrcpyA(g_variables + varnum*g_stringsize, var);
//}
//
}

