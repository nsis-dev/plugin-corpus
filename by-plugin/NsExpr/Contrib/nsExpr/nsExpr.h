#ifndef __NsExpr_H__
#define __NsExpr_H__

#define NSISFUNC(name) void __declspec(dllexport) name(HWND hWndParent, int string_size, TCHAR* variables, stack_t** stacktop, extra_parameters* extra)

enum TOKEN_TYPE
{
  TT_ERROR,
  TT_BRACKET_OPEN,
  TT_BRACKET_CLOSE,
  TT_INTEGER,
  TT_OPERATOR,
};

enum OPERATOR_TYPE
{
  OT_TERNARY_QUESTION, // ? (in x?y:z)
  OT_TERNARY_COLON, // : (in x?y:z)
  OT_LOGICAL_OR, // x||y
  OT_LOGICAL_AND, // x&&y
  OT_BITWISE_OR, // x|y
  OT_BITWISE_XOR, // x^y
  OT_BITWISE_AND, // x&y
  OT_EQUAL, // x==y
  OT_NOT_EQUAL, // x!=y
  OT_LESS_THAN, // x<y
  OT_LESS_THAN_OR_EQUAL, // x<=y
  OT_GREATER_THAN, // x>y
  OT_GREATER_THAN_OR_EQUAL, // x>=y
  OT_BITWISE_SHIFT_LEFT, // x<<y
  OT_BITWISE_SHIFT_RIGHT, // x>>y
  OT_MULTIPLY, // x*y
  OT_DIVIDE, // x/y
  OT_MODULUS, // x%y
  OT_ADD, // x+y
  OT_SUBTRACT, // x-y (or -x depending on context)
  OT_NEGATE, // ~x
  OT_NOT, // !x
};

struct TOKEN
{
  enum TOKEN_TYPE eToken;
  int nValue;
};

#define TOKEN_SUCCESS(token) ((token) != NULL)

#define MemAlloc(n) HeapAlloc(g_hHeap, HEAP_ZERO_MEMORY, n)
#define MemFree(p) HeapFree(g_hHeap, 0, p)
#define MemFreeEx(p) if (p) HeapFree(g_hHeap, 0, p)

#endif