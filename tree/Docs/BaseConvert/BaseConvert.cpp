/*******************************************************
* CaseConvert.cpp
*	Base Conversion of Very Long Positive Integers based
*	on http://www.codeproject.com/KB/recipes/BaseConverter.aspx#xx2812840xx
*	
*	Jonathan Abramson 2011
*	
*	30-11-11 fixed a bug when converted number is 0's
*	
*******************************************************/

#include <windows.h>
#include <tchar.h>
#include "exdll.h"
#include <cmath>

TCHAR	*szNumber = NULL,
		*szFrom = NULL,
		*szTo = NULL,
		*szOut = NULL;

TCHAR * Convert(int from, int to, const TCHAR * s, TCHAR * out);


/*****************************************************
* FUNCTION NAME: convert()
* PURPOSE: 
*    Convert Very Long Positive Integers
*	between two bases (2 - 36)
*****************************************************/
extern "C"
void __declspec(dllexport) convert(HWND hwndParent,
															 int string_size,
															 TCHAR *variables,
															 stack_t **stacktop,
															 extra_parameters *extra
															 )
{
	EXDLL_INIT();

	szNumber = (TCHAR*)GlobalAlloc(GPTR, string_size * sizeof(TCHAR));
	szFrom = (TCHAR*)GlobalAlloc(GPTR, string_size * sizeof(TCHAR));
	szTo = (TCHAR*)GlobalAlloc(GPTR, string_size * sizeof(TCHAR));
	szOut = (TCHAR*)GlobalAlloc(GPTR, string_size * sizeof(TCHAR));

	int nFrom, nTo;

	if (!popstring(szFrom) && !popstring(szTo) && !popstring(szNumber)) {
		nFrom = _ttoi(szFrom);
		nTo = _ttoi(szTo);
		Convert(nFrom,nTo,szNumber,szOut);
		pushstring(szOut);
	} else {
	}
}


TCHAR * Convert(int from, int to, const TCHAR * s, TCHAR * out)
{
	if ( s == NULL )
		return NULL;
 
	if (from < 2 || from > 36 || to < 2 || to > 36) { return NULL; }
 
	int il = _tcslen(s);
 
	int *fs = new int[il];
	int k = 0;
	int i,j;
 

	for (i = il - 1; i >=0; i-- )
	{
		if (s[i] >= '0' && s[i] <= '9') 
		{ 
			fs[k] = (int)(s[i] - '0'); 
		}
		else
		{
			if (s[i] >= 'A' && s[i] <= 'Z') 
			{ 
				fs[k] = 10 + (int)(s[i] - 'A'); 
			}
			else if (s[i] >= 'a' && s[i] <= 'z') 
			{
				fs[k] = 10 + (int)(s[i] - 'a'); 
			}
			else
			{ 
				delete[]fs;
				return NULL; 
			} //only allow 0-9 A-Z characters
		}
		k++;
	}
 
	for (i=0;i<il;i++)
	{
		if ( fs[i] >= from ) 
			return NULL;
	}
 
	double x = ceil(log((long double)from )  / log ((long double)to));
	int ol = 1+(int)( il * x );
 
	int * ts = new int[ol];
	int * cums = new int [ol];
 
	for (i=0;i<ol;i++)
	{
		ts[i]=0;
		cums[i]=0;
	}
	ts[0]=1;
 

	//evaluate the output
	for (i = 0; i < il; i++) //for each input digit
	{
		for (j = 0; j < ol; j++) //add the input digit times (base:to from^i) to the output cumulator
		{
			cums[j] += ts[j] * fs[i];
			int temp = cums[j];
			int rem = 0;
			int ip = j;
			do // fix up any remainders in base:to
			{
				rem = temp / to;
				cums[ip] = temp - rem * to; 
				ip++;
				if (ip >= ol)
				{
					if ( rem > 0 )
					{
						delete[]ts;
						delete[]cums;
						delete[]fs;
						return NULL;
					}
					break;
				}
				cums[ip] += rem;
				temp = cums[ip];
			}
			while (temp >= to);
		}
 
		for (j = 0; j < ol; j++)
		{
			ts[j] = ts[j] * from;
		}
 
		for (j = 0; j < ol; j++) //check for any remainders
		{
			int temp = ts[j];
			int rem = 0;
			int ip = j;
			do  //fix up any remainders
			{
				rem = temp / to;
				ts[ip] = temp - rem * to; 
				ip++;
				if (ip >= ol)
				{          
					if ( rem > 0 )
					{
						delete[]ts;
						delete[]cums;
						delete[]fs;
						return NULL;
					}
					break;
				}
				ts[ip] += rem;
				temp = ts[ip];
			}
			while (temp >= to);
		}
	}
 
	if ( out == NULL )
	{
		out = (TCHAR*)malloc( sizeof(TCHAR) * (ol + 1));
	}
 
	int spos = 0;
	bool first = false; //leading zero flag
	for (i = ol-1; i >= 0; i--)
	{
		if (cums[i] != 0) 
		{ 
			first = true; 
		}
		if (!first) 
		{ 
			continue; 
		}
 
		if (cums[i] < 10) 
		{ 			
			out[spos] = (char)(cums[i] + '0'); 
		}
		else 
		{ 			
			out[spos] = (char)(cums[i] + 'A' - 10); 
		}
		spos ++;
	}
	if (!first) //all was zeros
		out[spos++] = (char)('0');
	out[spos]=0;
 
	delete[]ts;
	delete[]cums;
	delete[]fs;
 
	return out;
}