// base64.cpp
// Base64 Encoding/Decoding Code Implementation File
//
// This implementation of Base64 encoding is based on Anders Molin's
// AMMimeUtils implementation at http://www.codeproject.com/string/ammimeutils.asp#xx266398xx.
// Here are the major changes from his original source:
//    - Added functions to calculate encode and decode buffer sizes
//    - Removed CR/LF insertion every 76th character
//    - Changed signature of Encode and Decode functions
//    - Changed data types from signed int and char to unsigned BYTE and ULONG
//    - Applied a couple of bug fixes:
//        - Fixed a bug that caused incorrect length of decoded output returned,
//	      - Fixed bug that caused incorrect length of encoded output returned
//        - Fixed a bug that caused input strings less than 4 bytes long to be
//          encoded as '===='
//	  - Changed names of some variables and constants to make them more intuitive
//
#include "../stdafx/stdafx.h"
#include "../base64/base64.h"

// BASE64_DIGITS is the array of valid Base64 encoded digits
static const BYTE BASE64_DIGITS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
// SKIP is the skipped byte placeholder
#define SKIP '\202'
// BASE64_UNDIGITS is the array of digits to unencode Base64
static const BYTE BASE64_UNDIGITS[] =
{
	SKIP,  SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
	SKIP,  SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
	SKIP,  SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
	SKIP,  SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
	SKIP,  SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
	SKIP,  SKIP,   SKIP,   62,     SKIP,   SKIP,   SKIP,   63,
	52,    53,     54,     55,     56,     57,     58,     59,
	60,    61,     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
	SKIP,  0,      1,      2,      3,      4,      5,      6,
	7,     8,      9,      10,     11,     12,     13,     14,
	15,    16,     17,     18,     19,     20,     21,     22,
	23,    24,     25,     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
	SKIP,  26,     27,     28,     29,     30,     31,     32,
	33,    34,     35,     36,     37,     38,     39,     40,
	41,    42,     43,     44,     45,     46,     47,     48,
	49,    50,     51,     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
	SKIP,  SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
	SKIP,  SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
	SKIP,  SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
	SKIP,  SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
	SKIP,  SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
	SKIP,  SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
	SKIP,  SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
	SKIP,  SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
	SKIP,  SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
	SKIP,  SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
	SKIP,  SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
	SKIP,  SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
	SKIP,  SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
	SKIP,  SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
	SKIP,  SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
	SKIP,  SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP
};
// CalculateEncodeBufferSize()
// This function determines an exact Base64 encoded buffer size for an input size
//
// Parameters:
//		ULONG inputsize		The exact size of the input string
// Returns:
//		ULONG				The exact size of the encoded buffer
//
ULONG CBase64Utils::CalculateEncodeBufferSize(ULONG inputsize) {
	ULONG result = 0;
	result = (inputsize % 3 == 0) ? (inputsize / 3) * 4 : ((inputsize / 3) + 1) * 4;
	return result;
}
// CalculateDecodeBufferSize()
// This function determines an approximate Base64 decoded buffer size for an input size
//
// Parameters:
//		ULONG inputsize		The exact size of the input string
// Returns:
//		ULONG				The approximate size of the encoded buffer
//
ULONG CBase64Utils::CalculateDecodeBufferSize(ULONG inputsize) {
	return ((inputsize + 1) * 3) / 4;
}
// Encode()
// This function encodes an input buffer and outputs to Base64 format
//
// Parameters:
//		BYTE *input			The input string buffer
//		BYTE *output		The output string buffer
//		ULONG inlength		The exact size of the input buffer
//
void CBase64Utils::Encode(BYTE *input, BYTE *output, ULONG inlength)
{
	// First we get the exact encode buffer length size
	ULONG alsize = CalculateEncodeBufferSize(inlength) * sizeof(BYTE);
	// Add 1 byte for C-style terminal-zero padding
	memset(output, 0, alsize + (1 * sizeof(BYTE)));
	// Initialize variables
	ULONG count = 0;
	BYTE *fresult = output;
	BYTE *s = input;
	ULONG tmp = 0;
	// Let's step through the buffer and encode it...
	while (count <= inlength)
	{
		if(inlength < 3)
			break;

		if (count % 3 == 0 && count != 0)
		{
			tmp >>= 8;
			tmp &= 0xFFFFFF;
			// We have 4 new Base64 digits, add them to output
			int mid = tmp;
			mid >>= 18;
			mid &= 0x3F;
			*(fresult++) = BASE64_DIGITS[mid];
			mid = tmp;
			mid >>= 12;
			mid &= 0x3F;
			*(fresult++) = BASE64_DIGITS[mid];
			mid = tmp;
			mid >>= 6;
			mid &= 0x3F;
			*(fresult++) = BASE64_DIGITS[mid];
			mid = tmp;
			mid &= 0x3F;
			*(fresult++) = BASE64_DIGITS[mid];
			// Reset tmp
			tmp = 0;
			if (inlength - count < 3)
				break;
		}
		BYTE mid = (256 - (0 - *s));
		tmp |= mid;
		tmp <<= 8;
		count++;
		s++;
	}
	// Do we have some chars left...
	int rest = (inlength - count) % 3;
	if (rest != 0)
	{
		tmp = 0;
		int i;
		for (i = 0; i < 3; i++)
		{
			if (i < rest)
			{
				unsigned char mid = (256 - (0 - *s));
				tmp |= mid;
				tmp <<= 8;
				count++;
				s++;
			}
			else
			{
				tmp |= 0;
				tmp <<= 8;
			}
		}
		tmp >>= 8;
		tmp &= 0xFFFFFF;
		// We have some new Base64 digits, add them to output
		int mid = tmp;
		if (rest >= 1)
		{
			mid >>= 18;
			mid &= 0x3F;
			*(fresult++) = BASE64_DIGITS[mid];
			mid = tmp;
			mid >>= 12;
			mid &= 0x3F;
			*(fresult++) = BASE64_DIGITS[mid];
		}
		if (rest >= 2)
		{
			mid = tmp;
			mid >>= 6;
			mid &= 0x3F;
			*(fresult++) = BASE64_DIGITS[mid];
		}
		if (rest >= 3)
		{
			mid = tmp;
			mid &= 0x3F;
			*(fresult++) = BASE64_DIGITS[mid];
		}
		for (int c = 3; c > rest; c--)
		{
			*(fresult++) = '=';
		}
	}
	fresult = NULL;
	s = NULL;
}
// Decode()
// This function decodes an input buffer in Base64 format and outputs in binary format
//
// Parameters:
//		BYTE *input			The input string buffer
//		BYTE *output		The output string buffer
//		ULONG inlength		The exact size of the input buffer
//	Returns:
//		ULONG				The exact number of decoded characters
//
ULONG CBase64Utils::Decode(BYTE *input, BYTE *output, ULONG inlength)
{
	// Initialize variables
	ULONG std = 0;
	ULONG count = 1;
	ULONG resultlen = 0;
	BYTE *s = input;
	BYTE *result = output;
	// Keep going until we hit an '=' sign or we run out of characters
	while (*s != '=' && count <= inlength)
	{
		// Check to see if it's a legal base64 char...
		while (BASE64_UNDIGITS[*s] == SKIP)
		{
			s++;
			inlength--;
			if (count >= inlength)
			{
				break;
			}
		}
		// Add the Base64 decode digit to std...
		std |= BASE64_UNDIGITS[*(s++) & 0xFF];
		std <<= 6;
		if (count % 4 == 0) // We have 3 more real chars...
		{
			// Put std in the next 3 chars in output
			int tmp;
			std >>= 6;
			tmp = std;
			tmp >>= 16;
			tmp &= 0xFF;
			*(result++) = (tmp);
			tmp = std;
			tmp >>= 8;
			tmp &= 0xFF;
			*(result++) = (tmp);
			tmp = std;
			tmp &= 0xFF;
			*(result++) = (tmp);
			std = 0; //empty std
			resultlen += 3;
		}
		count++;
	}
	// Find and decode the remaining chars, if any...
	count--;
	if (count % 4 != 0)
	{
		// We have some remaining chars, now decode them...
		for (ULONG i = 0; i < 4 - (count % 4); i++)
			std <<= 6;
		resultlen += (count % 4) - 1;
		int tmp;
		std >>= 6;
		tmp = std;
		tmp >>= 16;
		tmp &= 0xFF;
		*(result++) = (tmp);
		tmp = std;
		tmp >>= 8;
		tmp &= 0xFF;
		*(result++) = (tmp);
		tmp = std;
		tmp &= 0xFF;
		*(result++) = (tmp);
	}
	// Return the exact length of the result
	result = NULL;
	s = NULL;
	return (resultlen);
}