/*****************************************************************
 *             Stack functions NSIS plugin v1.8                  *
 *                                                               *
 * 2013 Shengalts Aleksander aka Instructor (Shengalts@mail.ru)  *
 *****************************************************************/


//Comment-out and recompile
#define NSIS_STACK     //Compile with NSIS stack functions
#define PRIVAT_STACK   //Compile with privat stack functions
#define STACK_DEBUG    //Compile with debug dialog function



#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include "StrFunc.h"
#include "WideFunc.h"
#include "Resources\resource.h"

//Include string functions
#define WideCharLower
#define xmemcpy
#define xstrcmpiW
#define xatoiW
#define xitoaW
#define xuitoaW
#define dec2hexW
#define xstrcpyW
#define xstrcpynA
#define xstrcpynW
#define xstrlenA
#define xstrlenW
#define xprintfW
#include "StrFunc.h"

//Include stack functions
#define ALLSTACKFUNC
#include "StackFunc.h"

//Include wide functions
#define DialogBoxWide
#define AppendMenuWide
#define ListBox_AddStringWide
#define ListBox_GetTextWide
#include "WideFunc.h"

//Defines
#define NSIS_MAX_STRLEN  1024
#define PS_SIMPLE   1
#define PS_ADVANCED 2

typedef struct _nsis_stack {
  struct _nsis_stack *next;
  wchar_t string[NSIS_MAX_STRLEN];
} nsis_stack;

typedef struct _privat_stack {
  struct _privat_stack *next;
  struct _privat_stack *prev;
  wchar_t string[NSIS_MAX_STRLEN];
} privat_stack;

//ExDll
typedef struct _stack_t {
  struct _stack_t *next;
  wchar_t text[1];
} stack_t;

typedef struct
{
  int autoclose;
  int all_user_var;
  int exec_error;
  int abort;
  int exec_reboot;
  int reboot_called;
  int XXX_cur_insttype;
  int plugin_api_version;
  int silent;
  int instdir_error;
  int rtl;
  int errlvl;
  int alter_reg_view;
  int status_update;
} exec_flags_t;

typedef struct {
  exec_flags_t *exec_flags;
  int (__stdcall *ExecuteCodeSegment)(int, HWND);
  void (__stdcall *validate_filename)(wchar_t *);
} extra_parameters;

stack_t **g_stacktop;
wchar_t *g_variables;
unsigned int g_stringsize;
extra_parameters *g_pluginParms;
BOOL g_unicode=-1;

//Global variables
char szBuf[NSIS_MAX_STRLEN];
wchar_t wszBuf[NSIS_MAX_STRLEN];
HINSTANCE g_hInstance;
nsis_stack *firstL=NULL;
nsis_stack *elementL=NULL;
HSTACK *hStack;

//Funtions prototypes and macros
int StackPushSortStr(stack **first, stack **last, stack **element, wchar_t *wpString, int nUpDown, int nBytes);
int StackSortStr(stack **first, stack **last, int nUpDown);
int StackPushSortInt(stack **first, stack **last, stack **element, wchar_t *wpString, int nUpDown, int bytes);
int StackSortInt(stack **first, stack **last, int nUpDown);
LRESULT CALLBACK DebugDialog(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

INT_PTR FromNsisStr(wchar_t *pDst, void *pNsisStr, INT_PTR nLen);
INT_PTR ToNsisStr(void *pNsisStr, wchar_t *pSrc, INT_PTR nLen);
INT_PTR popintegerWide();
void pushintegerWide(INT_PTR integer);
int popstringAnsi(char *str, int len);
int popstringWide(wchar_t *str, int len);
void pushstringAnsi(const char *str);
void pushstringWide(const wchar_t *str);
void Initialize(int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra);

//NSIS functions code
#ifdef NSIS_STACK
void __declspec(dllexport) _ns_push_front(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    popstringWide(wszBuf, NSIS_MAX_STRLEN);

    if (!StackInsertL((stackL **)g_stacktop, (stackL **)&elementL, 1, sizeof(nsis_stack)))
      ToNsisStr(elementL->string, wszBuf, NSIS_MAX_STRLEN);
  }
}

void __declspec(dllexport) _ns_pop_front(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    int nError;

    if (!(nError=StackGetElementL((stackL *)*g_stacktop, (stackL **)&elementL, 1)))
    {
      FromNsisStr(wszBuf, elementL->string, NSIS_MAX_STRLEN);
      StackDeleteL((stackL **)g_stacktop, 1);
    }

    pushintegerWide(nError);
    pushstringWide(nError?L"":wszBuf);
  }
}

void __declspec(dllexport) _ns_push_back(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    int nLastIndex;

    popstringWide(wszBuf, NSIS_MAX_STRLEN);

    nLastIndex=StackSizeL((stackL *)*g_stacktop);
    if (!StackInsertL((stackL **)g_stacktop, (stackL **)&elementL, nLastIndex + 1, sizeof(nsis_stack)))
      ToNsisStr(elementL->string, wszBuf, NSIS_MAX_STRLEN);
  }
}

void __declspec(dllexport) _ns_pop_back(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    int nLastIndex;
    int nError;

    nLastIndex=StackSizeL((stackL *)*g_stacktop);
    if (!(nError=StackGetElementL((stackL *)*g_stacktop, (stackL **)&elementL, nLastIndex)))
    {
      FromNsisStr(wszBuf, elementL->string, NSIS_MAX_STRLEN);
      StackDeleteL((stackL **)g_stacktop, nLastIndex);
    }

    pushintegerWide(nError);
    pushstringWide(nError?L"":wszBuf);
  }
}

void __declspec(dllexport) _ns_insert(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    int nIndex;
    int nError;

    popstringWide(wszBuf, NSIS_MAX_STRLEN);
    nIndex=(int)popintegerWide();

    if (!(nError=StackInsertL((stackL **)g_stacktop, (stackL **)&elementL, nIndex, sizeof(nsis_stack))))
      ToNsisStr(elementL->string, wszBuf, NSIS_MAX_STRLEN);

    pushintegerWide(nError);
  }
}

void __declspec(dllexport) _ns_delete(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    int nIndex;
    int nError;

    nIndex=(int)popintegerWide();

    if (!(nError=StackGetElementL((stackL *)*g_stacktop, (stackL **)&elementL, nIndex)))
    {
      FromNsisStr(wszBuf, elementL->string, NSIS_MAX_STRLEN);
      StackDeleteL((stackL **)g_stacktop, nIndex);
    }

    pushintegerWide(nError);
    pushstringWide(nError?L"":wszBuf);
  }
}

void __declspec(dllexport) _ns_read(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    int nIndex;
    int nError;

    nIndex=(int)popintegerWide();

    if (!(nError=StackGetElementL((stackL *)*g_stacktop, (stackL **)&elementL, nIndex)))
      FromNsisStr(wszBuf, elementL->string, NSIS_MAX_STRLEN);

    pushintegerWide(nError);
    pushstringWide(nError?L"":wszBuf);
  }
}

void __declspec(dllexport) _ns_write(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    int nIndex;
    int nError;

    popstringWide(wszBuf, NSIS_MAX_STRLEN);
    nIndex=(int)popintegerWide();

    if (!(nError=StackGetElementL((stackL *)*g_stacktop, (stackL **)&elementL, nIndex)))
      ToNsisStr(elementL->string, wszBuf, NSIS_MAX_STRLEN);

    pushintegerWide(nError);
  }
}

void __declspec(dllexport) _ns_size(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    int nSize;

    nSize=StackSizeL((stackL *)*g_stacktop);

    pushintegerWide(nSize);
  }
}

void __declspec(dllexport) _ns_clear(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    StackClearL((stackL **)g_stacktop);
  }
}
#endif //NSIS_STACK


#ifdef PRIVAT_STACK
void __declspec(dllexport) _dll_create(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    hStack=(HSTACK *)GlobalAlloc(GPTR, sizeof(HSTACK));
    pushintegerWide((INT_PTR)hStack);
  }
}

void __declspec(dllexport) _dll_insert(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    privat_stack *element;
    int nIndex;
    int nError;

    hStack=(HSTACK *)popintegerWide();
    popstringWide(wszBuf, NSIS_MAX_STRLEN);
    nIndex=(int)popintegerWide();

    if (!(nError=StackInsertIndex((stack **)&hStack->first, (stack **)&hStack->last, (stack **)&element, nIndex, sizeof(privat_stack))))
      xstrcpynW(element->string, wszBuf, NSIS_MAX_STRLEN);

    pushintegerWide(nError);
  }
}

void __declspec(dllexport) _dll_delete(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    privat_stack *element;
    int nIndex;
    int nError;

    hStack=(HSTACK *)popintegerWide();
    nIndex=(int)popintegerWide();

    if (!(nError=StackGetElement((stack *)hStack->first, (stack *)hStack->last, (stack **)&element, nIndex)))
    {
      xstrcpynW(wszBuf, element->string, NSIS_MAX_STRLEN);
      StackDelete((stack **)&hStack->first, (stack **)&hStack->last, (stack *)element);
    }

    pushintegerWide(nError);
    pushstringWide(nError?L"":wszBuf);
  }
}

void __declspec(dllexport) _dll_read(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    privat_stack *element;
    int nIndex;
    int nError;

    hStack=(HSTACK *)popintegerWide();
    nIndex=(int)popintegerWide();

    if (!(nError=StackGetElement((stack *)hStack->first, (stack *)hStack->last, (stack **)&element, nIndex)))
      xstrcpynW(wszBuf, element->string, NSIS_MAX_STRLEN);

    pushintegerWide(nError);
    pushstringWide(nError?L"":wszBuf);
  }
}

void __declspec(dllexport) _dll_write(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    privat_stack *element;
    int nIndex;
    int nError;

    hStack=(HSTACK *)popintegerWide();
    popstringWide(wszBuf, NSIS_MAX_STRLEN);
    nIndex=(int)popintegerWide();

    if (!(nError=StackGetElement((stack *)hStack->first, (stack *)hStack->last, (stack **)&element, nIndex)))
      xstrcpynW(element->string, wszBuf, NSIS_MAX_STRLEN);

    pushintegerWide(nError);
  }
}

void __declspec(dllexport) _dll_move(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    privat_stack *element;
    int nIndex1;
    int nIndex2;
    int nError;

    hStack=(HSTACK *)popintegerWide();
    nIndex1=(int)popintegerWide();
    nIndex2=(int)popintegerWide();

    if (!(nError=StackGetElement((stack *)hStack->first, (stack *)hStack->last, (stack **)&element, nIndex1)))
      nError=StackMoveIndex((stack **)&hStack->first, (stack **)&hStack->last, (stack *)element, nIndex2);

    pushintegerWide(nError);
  }
}

void __declspec(dllexport) _dll_exchange(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    privat_stack *element1;
    privat_stack *element2;
    int nIndex1;
    int nIndex2;
    int nError;

    hStack=(HSTACK *)popintegerWide();
    nIndex1=(int)popintegerWide();
    nIndex2=(int)popintegerWide();

    if (!(nError=StackGetElement((stack *)hStack->first, (stack *)hStack->last, (stack **)&element1, nIndex1)) &&
        !(nError=StackGetElement((stack *)hStack->first, (stack *)hStack->last, (stack **)&element2, nIndex2)))
    {
      nError=StackExchange((stack **)&hStack->first, (stack **)&hStack->last, (stack *)element1, (stack *)element2);
    }

    pushintegerWide(nError);
  }
}

void __declspec(dllexport) _dll_delete_range(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    int nIndex1;
    int nIndex2;
    int nError;

    hStack=(HSTACK *)popintegerWide();
    nIndex1=(int)popintegerWide();
    nIndex2=(int)popintegerWide();

    nError=StackDeleteRange((stack **)&hStack->first, (stack **)&hStack->last, nIndex1, nIndex2);

    pushintegerWide(nError);
  }
}

void __declspec(dllexport) _dll_move_range(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    int nIndex1;
    int nIndex2;
    int nIndex3;
    int nError;

    hStack=(HSTACK *)popintegerWide();
    nIndex1=(int)popintegerWide();
    nIndex2=(int)popintegerWide();
    nIndex3=(int)popintegerWide();

    nError=StackMoveRange((stack **)&hStack->first, (stack **)&hStack->last, nIndex1, nIndex2, nIndex3);

    pushintegerWide(nError);
  }
}

void __declspec(dllexport) _dll_reverse_range(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    privat_stack *element1;
    privat_stack *element2;
    int nIndex1;
    int nIndex2;
    int nError;

    hStack=(HSTACK *)popintegerWide();
    nIndex1=(int)popintegerWide();
    nIndex2=(int)popintegerWide();

    if (!(nError=StackGetElement((stack *)hStack->first, (stack *)hStack->last, (stack **)&element1, nIndex1)) &&
        !(nError=StackGetElement((stack *)hStack->first, (stack *)hStack->last, (stack **)&element2, nIndex2)))
    {
      StackReverseRange((stack **)&hStack->first, (stack **)&hStack->last, (stack *)element1, (stack *)element2);
    }

    pushintegerWide(nError);
  }
}

void __declspec(dllexport) _dll_push_sort(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    privat_stack *element;
    int nIndex;

    hStack=(HSTACK *)popintegerWide();
    popstringWide(wszBuf, NSIS_MAX_STRLEN);
    nIndex=(int)popintegerWide();

    if (!StackPushSortStr((stack **)&hStack->first, (stack **)&hStack->last, (stack **)&element, wszBuf, nIndex, sizeof(privat_stack)))
      xstrcpynW(element->string, wszBuf, NSIS_MAX_STRLEN);
  }
}

void __declspec(dllexport) _dll_push_sort_int(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    privat_stack *element;
    int nIndex;

    hStack=(HSTACK *)popintegerWide();
    popstringWide(wszBuf, NSIS_MAX_STRLEN);
    nIndex=(int)popintegerWide();

    if (!StackPushSortInt((stack **)&hStack->first, (stack **)&hStack->last, (stack **)&element, wszBuf, nIndex, sizeof(privat_stack)))
      xstrcpynW(element->string, wszBuf, NSIS_MAX_STRLEN);
  }
}

void __declspec(dllexport) _dll_sort_all(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    int nIndex;

    hStack=(HSTACK *)popintegerWide();
    nIndex=(int)popintegerWide();

    StackSortStr((stack **)&hStack->first, (stack **)&hStack->last, nIndex);
  }
}

void __declspec(dllexport) _dll_sort_all_int(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    int nIndex;

    hStack=(HSTACK *)popintegerWide();
    nIndex=(int)popintegerWide();

    StackSortInt((stack **)&hStack->first, (stack **)&hStack->last, nIndex);
  }
}

void __declspec(dllexport) _dll_size(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    int nSize;

    hStack=(HSTACK *)popintegerWide();

    nSize=StackSize((stack *)hStack->first, (stack *)hStack->last);

    pushintegerWide(nSize);
  }
}

void __declspec(dllexport) _dll_clear(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    hStack=(HSTACK *)popintegerWide();

    StackClear((stack **)&hStack->first, (stack **)&hStack->last);
  }
}

void __declspec(dllexport) _dll_destroy(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    hStack=(HSTACK *)popintegerWide();

    if (hStack)
    {
      StackClear((stack **)&hStack->first, (stack **)&hStack->last);
      GlobalFree((HGLOBAL)hStack);
    }
  }
}
#endif //PRIVAT_STACK

#ifdef STACK_DEBUG
void __declspec(dllexport) _Debug(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  Initialize(string_size, variables, stacktop, extra);
  {
    hStack=(HSTACK *)popintegerWide();

    firstL=(nsis_stack *)*stacktop;
    DialogBoxWide(g_hInstance, MAKEINTRESOURCEW(IDD_DIALOG), hwndParent, (DLGPROC)DebugDialog);
  }
}
#endif //STACK_DEBUG

void __declspec(dllexport) _Unload(HWND hwndParent, int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
  g_hInstance=(HINSTANCE)hInst;
  return TRUE;
}



/* Functions */
LRESULT CALLBACK DebugDialog(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  static HWND hNSISList;
  static HWND hPrivatList;
  static int nNSISShow;
  static int nPrivatShow;
  int nIndex;
  int nLen=0;
  int nMax;

  if (uMsg == WM_INITDIALOG)
  {
    hNSISList=GetDlgItem(hDlg, IDC_NSIS_LIST);
    hPrivatList=GetDlgItem(hDlg, IDC_PRIVAT_LIST);

#ifdef NSIS_STACK
    nNSISShow=0;
    SendMessage(hDlg, WM_COMMAND, IDC_NSIS_LIST_LABEL, 0);
#endif

#ifdef PRIVAT_STACK
    nPrivatShow=0;
    SendMessage(hDlg, WM_COMMAND, IDC_PRIVAT_LIST_LABEL, 0);
#endif
    return TRUE;
  }
  else if (uMsg == WM_CONTEXTMENU)
  {
    if ((HWND)wParam == hNSISList ||
        (HWND)wParam == hPrivatList)
    {
      POINT pt;
      RECT r;
      HMENU hMenu;
      int nCount;

      if (lParam == -1)
      {
        GetWindowRect((HWND)wParam, &r);
        pt.x=r.left;
        pt.y=r.top;
      }
      else
      {
        pt.x=LOWORD(lParam);
        pt.y=HIWORD(lParam);
      }
      if (!(nCount=SendMessage((HWND)wParam, LB_GETSELCOUNT, 0, 0)))
        return TRUE;

      if (hMenu=CreatePopupMenu())
      {
        AppendMenuWide(hMenu, nCount?MF_STRING:MF_STRING|MF_GRAYED, 1, L"Copy");

        if (TrackPopupMenu(hMenu, TPM_NONOTIFY|TPM_RETURNCMD, pt.x, pt.y, 0, (HWND)wParam, NULL) == 1)
        {
          HGLOBAL hDataTargetA=NULL;
          HGLOBAL hDataTargetW=NULL;
          char *pDataTargetA;
          wchar_t *pDataTargetW;
          wchar_t *wpCount;
          int *lpSelItems;
          int i;
          UINT_PTR dwAnsiLen=0;
          UINT_PTR dwUnicodeLen=0;

          if (OpenClipboard(NULL))
          {
            EmptyClipboard();

            if (lpSelItems=(int *)GlobalAlloc(GPTR, nCount * sizeof(int)))
            {
              SendMessage((HWND)wParam, LB_GETSELITEMS, nCount, (LPARAM)lpSelItems);

              for (i=0; i < nCount; ++i)
              {
                dwUnicodeLen+=SendMessage((HWND)wParam, LB_GETTEXTLEN, lpSelItems[i], 0) + 2;
              }

              if (hDataTargetW=GlobalAlloc(GMEM_MOVEABLE, (dwUnicodeLen + 1) * sizeof(wchar_t)))
              {
                if (pDataTargetW=(wchar_t *)GlobalLock(hDataTargetW))
                {
                  pDataTargetW[0]=L'\0';
                  wpCount=pDataTargetW;

                  for (i=0; i < nCount; ++i)
                  {
                    wpCount+=ListBox_GetTextWide((HWND)wParam, lpSelItems[i], wpCount);
                    wpCount+=xstrcpyW(wpCount, L"\r\n");
                  }

                  //Get Ansi text
                  dwAnsiLen=WideCharToMultiByte(CP_ACP, 0, pDataTargetW, dwUnicodeLen, NULL, 0, NULL, NULL);

                  if (hDataTargetA=GlobalAlloc(GMEM_MOVEABLE, dwAnsiLen))
                  {
                    if (pDataTargetA=GlobalLock(hDataTargetA))
                    {
                      WideCharToMultiByte(CP_ACP, 0, pDataTargetW, dwUnicodeLen, pDataTargetA, dwAnsiLen, NULL, NULL);
                      GlobalUnlock(hDataTargetA);
                    }
                  }
                  GlobalUnlock(hDataTargetW);
                }
                if (hDataTargetW) SetClipboardData(CF_UNICODETEXT, hDataTargetW);
                if (hDataTargetA) SetClipboardData(CF_TEXT, hDataTargetA);
                CloseClipboard();

                GlobalFree(hDataTargetW);
              }
              GlobalFree(lpSelItems);
            }
          }
        }
        DestroyMenu(hMenu);
      }
    }
    return TRUE;
  }
  else if (uMsg == WM_COMMAND)
  {
    if (LOWORD(wParam) == IDC_NSIS_LIST_LABEL)
    {
#ifdef NSIS_STACK
      if (nNSISShow == PS_SIMPLE)
        nNSISShow=PS_ADVANCED;
      else
        nNSISShow=PS_SIMPLE;
      SendMessage(hNSISList, LB_RESETCONTENT, 0, 0);

      //Fill list
      elementL=firstL;

      for (nIndex=1, nMax=0; elementL; ++nIndex)
      {
        if (nNSISShow == PS_SIMPLE)
          nLen=xprintfW(wszBuf, g_unicode?L"%s":L"%S", elementL->string);
        else if (nNSISShow == PS_ADVANCED)
          nLen=xprintfW(wszBuf, g_unicode?L"{%s}->{%s}":L"{%S}->{%S}", elementL->string, elementL->next?elementL->next->string:L"");
        ListBox_AddStringWide(hNSISList, wszBuf);
        if (nLen > nMax) nMax=nLen;

        elementL=elementL->next;
      }
      xprintfW(wszBuf, L"[%d elements]", nIndex - 1);
      ListBox_AddStringWide(hNSISList, wszBuf);
      SendMessage(hNSISList, LB_SETHORIZONTALEXTENT, nMax * 8, 0);
#endif
    }
    else if (LOWORD(wParam) == IDC_PRIVAT_LIST_LABEL)
    {
#ifdef PRIVAT_STACK
      if (nPrivatShow == PS_SIMPLE)
        nPrivatShow=PS_ADVANCED;
      else
        nPrivatShow=PS_SIMPLE;
      SendMessage(hPrivatList, LB_RESETCONTENT, 0, 0);

      //Fill list
      if (hStack)
      {
        privat_stack *element=(privat_stack *)hStack->first;

        for (nIndex=1, nMax=0; element; ++nIndex)
        {
          if (nPrivatShow == PS_SIMPLE)
            nLen=xprintfW(wszBuf, L"%s", element->string);
          else if (nPrivatShow == PS_ADVANCED)
            nLen=xprintfW(wszBuf, L"{%s}<-{%s}->{%s}", element->prev?element->prev->string:L"", element->string, element->next?element->next->string:L"");
          ListBox_AddStringWide(hPrivatList, wszBuf);
          if (nLen > nMax) nMax=nLen;

          element=element->next;
        }
        xprintfW(wszBuf, L"[%d elements]", nIndex - 1);
        ListBox_AddStringWide(hPrivatList, wszBuf);
        SendMessage(hPrivatList, LB_SETHORIZONTALEXTENT, nMax * 8, 0);
      }
#endif
    }
    else if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
    {
      EndDialog(hDlg, 0);
    }
    return TRUE;
  }
  return FALSE;
}

// Function: Synchronize linear functions stack with NSIS stack
int StackPushSortStr(stack **first, stack **last, stack **element, wchar_t *wpString, int nUpDown, int nBytes)
{
  privat_stack *tmp;
  int i;

  if (nUpDown != 1 && nUpDown != -1) return 1;

  for (tmp=(privat_stack *)*first; tmp; tmp=tmp->next)
  {
    i=xstrcmpiW(tmp->string, wpString);

    if (i == 0 || i == nUpDown)
      break;
  }
  return StackInsertBefore(first, last, (stack *)tmp, element, nBytes);
}

int StackSortStr(stack **first, stack **last, int nUpDown)
{
  privat_stack *tmp1;
  privat_stack *tmp2;
  privat_stack *tmpNext;
  int i;

  if (nUpDown != 1 && nUpDown != -1) return 1;

  for (tmp1=(privat_stack *)*first; tmp1; tmp1=tmpNext)
  {
    tmpNext=tmp1->next;

    for (tmp2=(privat_stack *)*first; tmp2 != tmp1; tmp2=tmp2->next)
    {
      i=xstrcmpiW(tmp2->string, tmp1->string);

      if (i == 0 || i == nUpDown)
      {
        StackMoveBefore(first, last, (stack *)tmp1, (stack *)tmp2);
        break;
      }
    }
  }
  return 0;
}

int StackPushSortInt(stack **first, stack **last, stack **element, wchar_t *wpString, int nUpDown, int bytes)
{
  privat_stack *tmp;
  INT_PTR nFirst;
  INT_PTR nSecond;

  if (nUpDown != 1 && nUpDown != -1) return 1;

  nFirst=xatoiW(wpString, NULL);

  for (tmp=(privat_stack *)*first; tmp; tmp=tmp->next)
  {
    nSecond=xatoiW(tmp->string, NULL);

    if ((nUpDown == 1 && nSecond >= nFirst) ||
        (nUpDown == -1 && nSecond <= nFirst))
      break;
  }
  return StackInsertBefore((stack **)first, (stack **)last, (stack *)tmp, (stack **)element, bytes);
}

int StackSortInt(stack **first, stack **last, int nUpDown)
{
  privat_stack *tmp1;
  privat_stack *tmp2;
  privat_stack *tmpNext;
  INT_PTR nFirst;
  INT_PTR nSecond;

  if (nUpDown != 1 && nUpDown != -1) return 1;

  for (tmp1=(privat_stack *)*first; tmp1; tmp1=tmpNext)
  {
    nFirst=xatoiW(tmp1->string, NULL);
    tmpNext=tmp1->next;

    for (tmp2=(privat_stack *)*first; tmp2 != tmp1; tmp2=tmp2->next)
    {
      nSecond=xatoiW(tmp2->string, NULL);

      if ((nUpDown == 1 && nSecond >= nFirst) ||
          (nUpDown == -1 && nSecond <= nFirst))
      {
        StackMoveBefore(first, last, (stack *)tmp1, (stack *)tmp2);
        break;
      }
    }
  }
  return 0;
}

INT_PTR FromNsisStr(wchar_t *pDst, void *pNsisStr, INT_PTR nLen)
{
  if (g_unicode)
    return xstrcpynW(pDst, (wchar_t *)pNsisStr, nLen);
  else
    return MultiByteToWideChar(CP_ACP, 0, (char *)pNsisStr, -1, pDst, nLen);
}

INT_PTR ToNsisStr(void *pNsisStr, wchar_t *pSrc, INT_PTR nLen)
{
  if (g_unicode)
    return xstrcpynW((wchar_t *)pNsisStr, pSrc, nLen);
  else
    return WideCharToMultiByte(CP_ACP, 0, pSrc, -1, (char *)pNsisStr, nLen, NULL, NULL);
}

INT_PTR popintegerWide()
{
  wchar_t wszInt[32];

  wszInt[0]=L'\0';
  popstringWide(wszInt, 31);
  return xatoiW(wszInt, NULL);
}

void pushintegerWide(INT_PTR integer)
{
  wchar_t wszInt[32];

  xitoaW(integer, wszInt);
  pushstringWide(wszInt);
}

int popstringAnsi(char *str, int len)
{
  stack_t *th;

  if (g_stacktop && *g_stacktop)
  {
    th=(*g_stacktop);
    if (g_unicode)
    {
      if (len=WideCharToMultiByte(CP_ACP, 0, (const wchar_t *)th->text, -1, str, len, NULL, NULL))
        str[--len]='\0';
    }
    else
    {
      len=xstrcpynA(str, (const char *)th->text, len);
    }
    *g_stacktop=th->next;
    GlobalFree((HGLOBAL)th);
    return len;
  }
  return -1;
}

int popstringWide(wchar_t *str, int len)
{
  stack_t *th;

  if (g_stacktop && *g_stacktop)
  {
    th=(*g_stacktop);
    if (g_unicode)
    {
      len=xstrcpynW(str, th->text, len);
    }
    else
    {
      if (len=MultiByteToWideChar(CP_ACP, 0, (const char *)th->text, -1, str, len))
        str[--len]=L'\0';
    }
    *g_stacktop=th->next;
    GlobalFree((HGLOBAL)th);
    return len;
  }
  return -1;
}

void pushstringAnsi(const char *str)
{
  stack_t *th;

  if (g_stacktop)
  {
    if (g_unicode)
    {
      th=(stack_t *)GlobalAlloc(GPTR, sizeof(stack_t) + NSIS_MAX_STRLEN * sizeof(wchar_t));
      MultiByteToWideChar(CP_ACP, 0, str, -1, (wchar_t *)th->text, NSIS_MAX_STRLEN);
    }
    else
    {
      th=(stack_t *)GlobalAlloc(GPTR, sizeof(stack_t) + NSIS_MAX_STRLEN);
      xstrcpynA((char *)th->text, str, NSIS_MAX_STRLEN);
    }
    th->next=*g_stacktop;
    *g_stacktop=th;
  }
}

void pushstringWide(const wchar_t *str)
{
  stack_t *th;

  if (g_stacktop)
  {
    if (g_unicode)
    {
      th=(stack_t *)GlobalAlloc(GPTR, sizeof(stack_t) + NSIS_MAX_STRLEN * sizeof(wchar_t));
      xstrcpynW(th->text, str, NSIS_MAX_STRLEN);
    }
    else
    {
      th=(stack_t *)GlobalAlloc(GPTR, sizeof(stack_t) + NSIS_MAX_STRLEN);
      WideCharToMultiByte(CP_ACP, 0, str, -1, (char *)th->text, NSIS_MAX_STRLEN, NULL, NULL);
    }
    th->next=*g_stacktop;
    *g_stacktop=th;
  }
}

void Initialize(int string_size, wchar_t *variables, stack_t **stacktop, extra_parameters *extra)
{
  g_stacktop=stacktop;
  g_variables=variables;
  g_stringsize=string_size;
  g_pluginParms=extra;

  if (g_unicode == -1)
  {
    wchar_t wszCurPath[]=L"C:\\";

    g_pluginParms->validate_filename(wszCurPath);
    g_unicode=(wszCurPath[2] == L'\\')?FALSE:TRUE;

    //Initialize WideFunc.h header
    WideInitialize();
    WideGlobal_bOldWindows=!g_unicode;
  }
}
