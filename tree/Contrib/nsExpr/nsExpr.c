/*

nsExpr NSIS plug-in

Author:  Stuart 'Afrow UK' Welch
Company: Afrow Soft Ltd.
Website: http://www.afrowsoft.co.uk
E-mail:  afrowuk@afrowsoft.co.uk
Date:    9th January 2012
Version: 1.0.1.1

A small plug-in that evaluates Boolean and mathematical expressions.

*/

#include <windows.h>
#include "nsExpr.h"

#ifdef UNICODE
#include "nsis_unicode\pluginapi.h"
#else
#include "nsis_ansi\pluginapi.h"
#endif

HANDLE g_hHeap;

struct TOKEN* MatchExpr(TCHAR** ppszExpr, struct TOKEN** ppNext);

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
  return TRUE;
}

void MatchedOperator(TCHAR** ppszExpr, struct TOKEN** ppToken, enum OPERATOR_TYPE eOperator)
{
  (*ppToken)->eToken = TT_OPERATOR;
  (*ppToken)->nValue = eOperator;
  (*ppszExpr)++;
}

BOOL MatchToken(TCHAR** ppszExpr, struct TOKEN** ppToken)
{
  static TCHAR szWord[11];

  // Ignore whitespace.
  while (**ppszExpr == TEXT(' ') || **ppszExpr == TEXT('\t') || **ppszExpr == TEXT('\r') || **ppszExpr == TEXT('\n'))
    *ppszExpr = (*ppszExpr)++;

  // End of expression.
  if (**ppszExpr == TEXT('\0'))
  {
    MemFreeEx(*ppToken);
    *ppToken = NULL;
    return TRUE;
  }

  if (!*ppToken)
  {
    *ppToken = (struct TOKEN*)MemAlloc(sizeof(struct TOKEN));
    if (!*ppToken)
      return FALSE;
  }

  // Opening bracket.
  if (**ppszExpr == TEXT('('))
  {
    (*ppToken)->eToken = TT_BRACKET_OPEN;
    (*ppszExpr)++;
    return TRUE;
  }

  // Closing bracket.
  if (**ppszExpr == TEXT(')'))
  {
    (*ppToken)->eToken = TT_BRACKET_CLOSE;
    (*ppszExpr)++;
    return TRUE;
  }

  // Integer.
  if (**ppszExpr >= TEXT('0') && **ppszExpr <= TEXT('9'))
  {
    int i;
    for (i = 0; **ppszExpr >= TEXT('0') && **ppszExpr <= TEXT('9'); i++, (*ppszExpr)++)
      szWord[i] = **ppszExpr;
    szWord[i] = TEXT('\0');
    (*ppToken)->eToken = TT_INTEGER;
    (*ppToken)->nValue = myatoi(szWord);
    return TRUE;
  }

  // Variable $0-$9.
  if (**ppszExpr == TEXT('r'))
  {
    (*ppszExpr)++;
    if (**ppszExpr >= TEXT('0') && **ppszExpr <= TEXT('9'))
    {
      (*ppToken)->eToken = TT_INTEGER;
      (*ppToken)->nValue = myatoi(getuservariable(**ppszExpr - 48));
      (*ppszExpr)++;
      return TRUE;
    }

    return FALSE;
  }

  // Variable $R0-$R9.
  if (**ppszExpr == TEXT('R'))
  {
    (*ppszExpr)++;
    if (**ppszExpr >= TEXT('0') && **ppszExpr <= TEXT('9'))
    {
      (*ppToken)->eToken = TT_INTEGER;
      (*ppToken)->nValue = myatoi(getuservariable(**ppszExpr - 38));
      (*ppszExpr)++;
      return TRUE;
    }
    
    return FALSE;
  }

  // Ternary conditional ? operator.
  if (**ppszExpr == TEXT('?'))
  {
    MatchedOperator(ppszExpr, ppToken, OT_TERNARY_QUESTION);
    return TRUE;
  }

  // Ternary conditional : operator.
  if (**ppszExpr == TEXT(':'))
  {
    MatchedOperator(ppszExpr, ppToken, OT_TERNARY_COLON);
    return TRUE;
  }

  // Bitwise OR (|) and logical OR (||) operators.
  if (**ppszExpr == TEXT('|'))
  {
    MatchedOperator(ppszExpr, ppToken, OT_BITWISE_OR);
    if (**ppszExpr == TEXT('|'))
      MatchedOperator(ppszExpr, ppToken, OT_LOGICAL_OR);
    return TRUE;
  }

  // Bitwise AND (&) and logical AND (&&) operators.
  if (**ppszExpr == TEXT('&'))
  {
    MatchedOperator(ppszExpr, ppToken, OT_BITWISE_AND);
    if (**ppszExpr == TEXT('&'))
      MatchedOperator(ppszExpr, ppToken, OT_LOGICAL_AND);
    return TRUE;
  }

  // Bitwise XOR (^) operator.
  if (**ppszExpr == TEXT('^'))
  {
    MatchedOperator(ppszExpr, ppToken, OT_BITWISE_XOR);
    return TRUE;
  }

  // Equal (==) operator.
  if (**ppszExpr == TEXT('='))
  {
    (*ppszExpr)++;
    if (**ppszExpr == TEXT('='))
    {
      MatchedOperator(ppszExpr, ppToken, OT_EQUAL);
      return TRUE;
    }
    return FALSE;
  }

  // Not (!) and not equal (!=) operator.
  if (**ppszExpr == TEXT('!'))
  {
    MatchedOperator(ppszExpr, ppToken, OT_NOT);
    if (**ppszExpr == TEXT('='))
      MatchedOperator(ppszExpr, ppToken, OT_NOT_EQUAL);
    return TRUE;
  }

  // Less than (<), less than or equal to (<=) and bitwise left shift (<<) operators.
  if (**ppszExpr == TEXT('<'))
  {
    MatchedOperator(ppszExpr, ppToken, OT_LESS_THAN);
    if (**ppszExpr == TEXT('='))
      MatchedOperator(ppszExpr, ppToken, OT_LESS_THAN_OR_EQUAL);
    else if (**ppszExpr == TEXT('<'))
      MatchedOperator(ppszExpr, ppToken, OT_BITWISE_SHIFT_LEFT);
    return TRUE;
  }

  // Greater than (>), greater than or equal to (>=) and bitwise right shift (>>) operators.
  if (**ppszExpr == TEXT('>'))
  {
    MatchedOperator(ppszExpr, ppToken, OT_GREATER_THAN);
    if (**ppszExpr == TEXT('='))
      MatchedOperator(ppszExpr, ppToken, OT_GREATER_THAN_OR_EQUAL);
    else if (**ppszExpr == TEXT('>'))
      MatchedOperator(ppszExpr, ppToken, OT_BITWISE_SHIFT_RIGHT);
    return TRUE;
  }

  // Add (+) operator.
  if (**ppszExpr == TEXT('+'))
  {
    MatchedOperator(ppszExpr, ppToken, OT_ADD);
    return TRUE;
  }

  // Subtract (-) operator.
  if (**ppszExpr == TEXT('-'))
  {
    MatchedOperator(ppszExpr, ppToken, OT_SUBTRACT);
    return TRUE;
  }

  // Multiply (*) operator.
  if (**ppszExpr == TEXT('*'))
  {
    MatchedOperator(ppszExpr, ppToken, OT_MULTIPLY);
    return TRUE;
  }

  // Divide (/) operator.
  if (**ppszExpr == TEXT('/'))
  {
    MatchedOperator(ppszExpr, ppToken, OT_DIVIDE);
    return TRUE;
  }

  // Binary negate (~) operator.
  if (**ppszExpr == TEXT('~'))
  {
    MatchedOperator(ppszExpr, ppToken, OT_NEGATE);
    return TRUE;
  }

  // Modulus (%) operator.
  if (**ppszExpr == TEXT('%'))
  {
    MatchedOperator(ppszExpr, ppToken, OT_MODULUS);
    return TRUE;
  }
  
  return FALSE;
}

BOOL IsTokenMatch(struct TOKEN* pToken, enum TOKEN_TYPE eToken)
{
  if (!TOKEN_SUCCESS(pToken))
    return FALSE;

  if (pToken->eToken != eToken)
  {
    pToken->eToken = TT_ERROR;
    return FALSE;
  }

  return TRUE;
}

BOOL IsTokenMatchEx(struct TOKEN* pToken, enum TOKEN_TYPE eToken, int nValue)
{
  if (!TOKEN_SUCCESS(pToken))
    return FALSE;

  if (pToken->eToken != eToken || pToken->nValue != nValue)
  {
    pToken->eToken = TT_ERROR;
    return FALSE;
  }

  return TRUE;
}

// Matches x, (x), -x, ~x and !x.
struct TOKEN* MatchPrimary(TCHAR** ppszExpr, struct TOKEN** ppNext)
{
  struct TOKEN* pLeft = NULL;

  if (MatchToken(ppszExpr, &pLeft) && TOKEN_SUCCESS(pLeft))
  {
    // Open bracket?
    if (pLeft->eToken == TT_BRACKET_OPEN)
    {
      MemFreeEx(pLeft);
      pLeft = MatchExpr(ppszExpr, ppNext);

      if (IsTokenMatch(pLeft, TT_INTEGER))
      {
        // Match a close bracket.
        if (!IsTokenMatch(*ppNext, TT_BRACKET_CLOSE))
        {
          MemFreeEx(pLeft);
          return *ppNext;
        }
        
        // Match next token.
        if (!MatchToken(ppszExpr, ppNext))
        {
          MemFreeEx(pLeft);
          return NULL;
        }
      }
    }
    // Negative number.
    else if (pLeft->eToken == TT_OPERATOR && pLeft->nValue == OT_SUBTRACT)
    {
      MemFreeEx(pLeft);
      pLeft = MatchExpr(ppszExpr, ppNext);

      if (IsTokenMatch(pLeft, TT_INTEGER))
        pLeft->nValue *= -1;
    }
    // Binary negate number.
    else if (pLeft->eToken == TT_OPERATOR && pLeft->nValue == OT_NEGATE)
    {
      MemFreeEx(pLeft);
      pLeft = MatchExpr(ppszExpr, ppNext);

      if (IsTokenMatch(pLeft, TT_INTEGER))
        pLeft->nValue = ~pLeft->nValue;
    }
    // Logical NOT number.
    else if (pLeft->eToken == TT_OPERATOR && pLeft->nValue == OT_NOT)
    {
      MemFreeEx(pLeft);
      pLeft = MatchExpr(ppszExpr, ppNext);

      if (IsTokenMatch(pLeft, TT_INTEGER))
        pLeft->nValue = !pLeft->nValue;
    }
    else if (pLeft->eToken == TT_INTEGER)
    {
      // Match next token.
      if (!MatchToken(ppszExpr, ppNext))
      {
        MemFreeEx(pLeft);
        return NULL;
      }
    }
  }

  return pLeft;
}

// Matches x*y, x/y and x%y.
struct TOKEN* MatchMultiplicative(TCHAR** ppszExpr, struct TOKEN** ppNext)
{
  struct TOKEN* pLeft = MatchPrimary(ppszExpr, ppNext);

  if (IsTokenMatch(pLeft, TT_INTEGER))
  {
    while (TOKEN_SUCCESS(*ppNext) && (*ppNext)->eToken == TT_OPERATOR && ((*ppNext)->nValue >= OT_MULTIPLY && (*ppNext)->nValue <= OT_MODULUS))
    {
      int nOp = (*ppNext)->nValue;
      struct TOKEN* pRight = MatchPrimary(ppszExpr, ppNext);

      if (IsTokenMatch(pRight, TT_INTEGER))
      {
        if ((nOp == OT_DIVIDE || nOp == OT_MODULUS) && pRight->nValue == 0)
        {
          MemFreeEx(pLeft);
          MemFreeEx(pRight);
          return NULL;
        }

        if (nOp == OT_MULTIPLY)
          pLeft->nValue *= pRight->nValue;
        else if (nOp == OT_DIVIDE)
          pLeft->nValue /= pRight->nValue;
        else if (nOp == OT_MODULUS)
          pLeft->nValue %= pRight->nValue;
        MemFreeEx(pRight);
      }
      else
      {
        MemFreeEx(pLeft);
        return pRight;
      }
    }
  }
  
  return pLeft;
}

// Matches x+y and x-y.
struct TOKEN* MatchAdditive(TCHAR** ppszExpr, struct TOKEN** ppNext)
{
  struct TOKEN* pLeft = MatchMultiplicative(ppszExpr, ppNext);

  if (IsTokenMatch(pLeft, TT_INTEGER))
  {
    while (TOKEN_SUCCESS(*ppNext) && (*ppNext)->eToken == TT_OPERATOR && ((*ppNext)->nValue == OT_ADD || (*ppNext)->nValue == OT_SUBTRACT))
    {
      int nOp = (*ppNext)->nValue;
      struct TOKEN* pRight = MatchMultiplicative(ppszExpr, ppNext);

      if (IsTokenMatch(pRight, TT_INTEGER))
      {
        if (nOp == OT_ADD)
          pLeft->nValue += pRight->nValue;
        else if (nOp == OT_SUBTRACT)
          pLeft->nValue -= pRight->nValue;
        MemFreeEx(pRight);
      }
      else
      {
        MemFreeEx(pLeft);
        return pRight;
      }
    }
  }
  
  return pLeft;
}

// Matches x<<y and x>>y.
struct TOKEN* MatchBitwiseShift(TCHAR** ppszExpr, struct TOKEN** ppNext)
{
  struct TOKEN* pLeft = MatchAdditive(ppszExpr, ppNext);

  if (IsTokenMatch(pLeft, TT_INTEGER))
  {
    while (TOKEN_SUCCESS(*ppNext) && (*ppNext)->eToken == TT_OPERATOR && ((*ppNext)->nValue == OT_BITWISE_SHIFT_LEFT || (*ppNext)->nValue == OT_BITWISE_SHIFT_RIGHT))
    {
      int nOp = (*ppNext)->nValue;
      struct TOKEN* pRight = MatchAdditive(ppszExpr, ppNext);

      if (IsTokenMatch(pRight, TT_INTEGER))
      {
        if (nOp == OT_BITWISE_SHIFT_LEFT)
          pLeft->nValue = pLeft->nValue << pRight->nValue;
        else if (nOp == OT_BITWISE_SHIFT_RIGHT)
          pLeft->nValue = pLeft->nValue >> pRight->nValue;
        MemFreeEx(pRight);
      }
      else
      {
        MemFreeEx(pLeft);
        return pRight;
      }
    }
  }

  return pLeft;
}

// Matches x<y, x<=y, x>y and x>=y.
struct TOKEN* MatchRelational(TCHAR** ppszExpr, struct TOKEN** ppNext)
{
  struct TOKEN* pLeft = MatchBitwiseShift(ppszExpr, ppNext);

  if (IsTokenMatch(pLeft, TT_INTEGER))
  {
    while (TOKEN_SUCCESS(*ppNext) && (*ppNext)->eToken == TT_OPERATOR && ((*ppNext)->nValue >= OT_LESS_THAN && (*ppNext)->nValue <= OT_GREATER_THAN_OR_EQUAL))
    {
      int nOp = (*ppNext)->nValue;
      struct TOKEN* pRight = MatchBitwiseShift(ppszExpr, ppNext);

      if (IsTokenMatch(pRight, TT_INTEGER))
      {
        if (nOp == OT_LESS_THAN)
          pLeft->nValue = pLeft->nValue < pRight->nValue;
        else if (nOp == OT_LESS_THAN_OR_EQUAL)
          pLeft->nValue = pLeft->nValue <= pRight->nValue;
        else if (nOp == OT_GREATER_THAN)
          pLeft->nValue = pLeft->nValue > pRight->nValue;
        else if (nOp == OT_GREATER_THAN_OR_EQUAL)
          pLeft->nValue = pLeft->nValue >= pRight->nValue;
        MemFreeEx(pRight);
      }
      else
      {
        MemFreeEx(pLeft);
        return pRight;
      }
    }
  }

  return pLeft;
}

// Matches x==y and x!=y.
struct TOKEN* MatchEquality(TCHAR** ppszExpr, struct TOKEN** ppNext)
{
  struct TOKEN* pLeft = MatchRelational(ppszExpr, ppNext);

  if (IsTokenMatch(pLeft, TT_INTEGER))
  {
    while (TOKEN_SUCCESS(*ppNext) && (*ppNext)->eToken == TT_OPERATOR && ((*ppNext)->nValue == OT_EQUAL || (*ppNext)->nValue == OT_NOT_EQUAL))
    {
      int nOp = (*ppNext)->nValue;
      struct TOKEN* pRight = MatchRelational(ppszExpr, ppNext);

      if (IsTokenMatch(pRight, TT_INTEGER))
      {
        if (nOp == OT_EQUAL)
          pLeft->nValue = pLeft->nValue == pRight->nValue;
        else if (nOp == OT_NOT_EQUAL)
          pLeft->nValue = pLeft->nValue != pRight->nValue;
        MemFreeEx(pRight);
      }
      else
      {
        MemFreeEx(pLeft);
        return pRight;
      }
    }
  }

  return pLeft;
}

// Matches x&y.
struct TOKEN* MatchBitwiseAnd(TCHAR** ppszExpr, struct TOKEN** ppNext)
{
  struct TOKEN* pLeft = MatchEquality(ppszExpr, ppNext);

  if (IsTokenMatch(pLeft, TT_INTEGER))
  {
    while (TOKEN_SUCCESS(*ppNext) && (*ppNext)->eToken == TT_OPERATOR && (*ppNext)->nValue == OT_BITWISE_AND)
    {
      struct TOKEN* pRight = MatchEquality(ppszExpr, ppNext);

      if (IsTokenMatch(pRight, TT_INTEGER))
      {
        pLeft->nValue &= pRight->nValue;
        MemFreeEx(pRight);
      }
      else
      {
        MemFreeEx(pLeft);
        return pRight;
      }
    }
  }

  return pLeft;
}

// Matches x^y.
struct TOKEN* MatchBitwiseXor(TCHAR** ppszExpr, struct TOKEN** ppNext)
{
  struct TOKEN* pLeft = MatchBitwiseAnd(ppszExpr, ppNext);

  if (IsTokenMatch(pLeft, TT_INTEGER))
  {
    while (TOKEN_SUCCESS(*ppNext) && (*ppNext)->eToken == TT_OPERATOR && (*ppNext)->nValue == OT_BITWISE_XOR)
    {
      struct TOKEN* pRight = MatchBitwiseAnd(ppszExpr, ppNext);

      if (IsTokenMatch(pRight, TT_INTEGER))
      {
        pLeft->nValue ^= pRight->nValue;
        MemFreeEx(pRight);
      }
      else
      {
        MemFreeEx(pLeft);
        return pRight;
      }
    }
  }

  return pLeft;
}

// Matches x|y.
struct TOKEN* MatchBitwiseOr(TCHAR** ppszExpr, struct TOKEN** ppNext)
{
  struct TOKEN* pLeft = MatchBitwiseXor(ppszExpr, ppNext);

  if (IsTokenMatch(pLeft, TT_INTEGER))
  {
    while (TOKEN_SUCCESS(*ppNext) && (*ppNext)->eToken == TT_OPERATOR && (*ppNext)->nValue == OT_BITWISE_OR)
    {
      struct TOKEN* pRight = MatchBitwiseXor(ppszExpr, ppNext);

      if (IsTokenMatch(pRight, TT_INTEGER))
      {
        pLeft->nValue |= pRight->nValue;
        MemFreeEx(pRight);
      }
      else
      {
        MemFreeEx(pLeft);
        return pRight;
      }
    }
  }

  return pLeft;
}

// Matches x&&y.
struct TOKEN* MatchLogicalAnd(TCHAR** ppszExpr, struct TOKEN** ppNext)
{
  struct TOKEN* pLeft = MatchBitwiseOr(ppszExpr, ppNext);

  if (IsTokenMatch(pLeft, TT_INTEGER))
  {
    while (TOKEN_SUCCESS(*ppNext) && (*ppNext)->eToken == TT_OPERATOR && (*ppNext)->nValue == OT_LOGICAL_AND)
    {
      struct TOKEN* pRight = MatchBitwiseOr(ppszExpr, ppNext);

      if (IsTokenMatch(pRight, TT_INTEGER))
      {
        pLeft->nValue = pLeft->nValue && pRight->nValue;
        MemFreeEx(pRight);
      }
      else
      {
        MemFreeEx(pLeft);
        return pRight;
      }
    }
  }

  return pLeft;
}

// Matches x||y.
struct TOKEN* MatchLogicalOr(TCHAR** ppszExpr, struct TOKEN** ppNext)
{
  struct TOKEN* pLeft = MatchLogicalAnd(ppszExpr, ppNext);

  if (IsTokenMatch(pLeft, TT_INTEGER))
  {
    while (TOKEN_SUCCESS(*ppNext) && (*ppNext)->eToken == TT_OPERATOR && (*ppNext)->nValue == OT_LOGICAL_OR)
    {
      struct TOKEN* pRight = MatchLogicalAnd(ppszExpr, ppNext);

      if (IsTokenMatch(pRight, TT_INTEGER))
      {
        pLeft->nValue = pLeft->nValue || pRight->nValue;
        MemFreeEx(pRight);
      }
      else
      {
        MemFreeEx(pLeft);
        return pRight;
      }
    }
  }

  return pLeft;
}

// Matches x?y:z.
struct TOKEN* MatchTernaryConditional(TCHAR** ppszExpr, struct TOKEN** ppNext)
{
  struct TOKEN* pLeft = MatchLogicalOr(ppszExpr, ppNext);

  if (IsTokenMatch(pLeft, TT_INTEGER))
  {
    while (TOKEN_SUCCESS(*ppNext) && (*ppNext)->eToken == TT_OPERATOR && (*ppNext)->nValue == OT_TERNARY_QUESTION)
    {
      struct TOKEN* pTrue = MatchExpr(ppszExpr, ppNext);

      if (IsTokenMatch(pTrue, TT_INTEGER))
      {
        if (IsTokenMatchEx(*ppNext, TT_OPERATOR, OT_TERNARY_COLON))
        {
          struct TOKEN* pFalse = MatchLogicalOr(ppszExpr, ppNext);

          if (IsTokenMatch(pFalse, TT_INTEGER))
          {
            if (pLeft->nValue)
              pLeft->nValue = pTrue->nValue;
            else
              pLeft->nValue = pFalse->nValue;

            MemFreeEx(pTrue);
            MemFreeEx(pFalse);
          }
          else
          {
            MemFreeEx(pLeft);
            MemFreeEx(pTrue);
            return pFalse;
          }
        }
        else
        {
          MemFreeEx(pLeft);
          MemFreeEx(pTrue);
          return *ppNext;
        }
      }
      else
      {
        MemFreeEx(pLeft);
        return pTrue;
      }
    }
  }
  
  return pLeft;
}

// Matches an expression.
struct TOKEN* MatchExpr(TCHAR** ppszExpr, struct TOKEN** ppNext)
{
  return MatchTernaryConditional(ppszExpr, ppNext);
}

NSISFUNC(Eval)
{
  BOOL fSuccess = FALSE;
  TCHAR* pszArg;

  EXDLL_INIT();
  g_hHeap = GetProcessHeap();

  pszArg = (TCHAR*)MemAlloc(sizeof(TCHAR) * (string_size + 1));
  if (pszArg)
  {
    if (popstring(pszArg) == 0)
    {
      TCHAR* pszExpr = pszArg;
      struct TOKEN* pNext = NULL;
      struct TOKEN* pToken = MatchExpr(&pszExpr, &pNext);
      if (TOKEN_SUCCESS(pToken))
      {
        if (pToken->eToken == TT_INTEGER)
        {
          if (pNext == NULL)
          {
            wsprintf(pszArg, TEXT("%d"), pToken->nValue);
            pushstring(pszArg);
            fSuccess = TRUE;
          }
          else
            MemFree(pNext);
        }

        MemFreeEx(pToken);
      }
    }

    MemFreeEx(pszArg);
  }

  if (!fSuccess)
    pushstring(TEXT("error"));
}