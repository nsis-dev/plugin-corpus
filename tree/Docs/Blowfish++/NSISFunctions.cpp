/*****************************************************************************/
/**                                                                         **/
/** Parts of these sample programs are derivative works based on the work   **/
/** the following individuals:                                              **/
/**                                                                         **/
/** Base64 encoding/decoding - AMMimeUtils by Anders Molin.  Original source**/
/**     is available on the Code Project at                                 **/
/**     http://www.codeproject.com/string/ammimeutils.asp                   **/
/**                                                                         **/
/** Blowfish encryption/decryption - Blowfish by Bruce Schneier.  Original  **/
/**     source is available at Bruce Schneier's website:                    **/
/**     http://www.schneier.com/                                            **/
/**                                                                         **/
/**                                                                         **/
/** Plugin-DLL for NSIS. Allow you to encrypt/decrypt a message using a		**/
/** password key.															**/
/** You are free to use and/or change this software as you need.			**/
/**                                                                         **/
/** For any question or suggestion please contact me at:					**/
/**     Eugen Mihailescu													**/
/**     eugen.mihailescu@cubique.ro											**/
/**     Bucharest, ROMANIA													**/
/**                                                                         **/
/** Last update: 06.12.2009													**/
/**                                                                         **/
/*****************************************************************************/

#include <windows.h>
#include "nsis/api.h"
#include "nsis/pluginapi.h"
#include <stdlib.h>

#include "common/stdafx/stdafx.h"
#include "common/base64/base64.h"
#include "blowfish.h"


#define NSISFunc(name) extern "C" void __declspec(dllexport) name(HWND hwndParent, int string_size, char *variables, stack_t **stacktop)
#define ERR_USAGE_decrypt "Error: incorect function usage.\n\rSyntax:\n\rblowfish::decrypt <EncryptedText> <PasswordKey>"
#define ERR_USAGE_encrypt "Error: incorect function usage.\n\rSyntax:\n\rblowfish::encrypt <TextToEncrypt> <PasswordKey>"
#define ERR_KEY_SIZE "Key (@param2) must be 32 - 448 bits in length (8 - 56 bytes)"

HINSTANCE g_hInstance;

HWND g_hwndParent;


NSISFunc(decrypt)
{
	EXDLL_INIT();// initialize stack & variables we are using to access coresponding stack&variables of NSIS

	BYTE * text = NULL;
	BYTE * key = NULL;
	long textlength = 0;
	long keylength = 0;

	char * parm1 =NULL;
	char * parm2 =NULL;

	//read 1st param from stack; it represents then message to be decrypted
	stack_t *th;
	if (g_stacktop && *g_stacktop) 
	{
	  th=(*g_stacktop);
	  parm1=(char *)LocalAlloc(LPTR,lstrlen(th->text)+1);

	  lstrcpyA(parm1,th->text);
	  *g_stacktop = th->next;
	  GlobalFree((HGLOBAL)th);
	}
  
	if (lstrcmp(parm1, "--End--") == 0)
		parm1= NULL;

	//read 2nd param from stack; it represents then password key used to decrypt the message
	if (g_stacktop && *g_stacktop) 
	{
	  th=(*g_stacktop);
	  parm2=(char *)LocalAlloc(LPTR,lstrlen(th->text)+1);

	  lstrcpyA(parm2,th->text);
	  *g_stacktop = th->next;
	  GlobalFree((HGLOBAL)th);
	}
	
	//if 1st param not supplied then push into stack an error message
	if ( parm1==NULL )
    {
        pushstring(ERR_USAGE_decrypt);
		pushstring("1");
        return;
    }

	//if 2nd param not supplied then push into stack an error message
	if ( parm2==NULL )
    {
        pushstring(ERR_USAGE_decrypt);
		pushstring("1");
        return;
    }

	// Get our key length
	keylength = CBase64Utils::CalculateDecodeBufferSize(lstrlen(parm2));
	key = (BYTE *)LocalAlloc(LPTR,keylength+10);
	
	
	memset(key, 0,  keylength + 10);
	

	// Now decode our key from Base64
	keylength = CBase64Utils::Decode((BYTE*)parm2, key, lstrlen(parm2));
	

	if (keylength < 8 || keylength > 56) // Is our keylength between 32 and 448 bits?
	{ 
		pushstring(ERR_KEY_SIZE);
		pushstring("1");
	}
	else 
	{
		// Now get our text length
	
		textlength = CBase64Utils::CalculateDecodeBufferSize(lstrlen(parm1));
	
		text = (BYTE *)LocalAlloc(LPTR,textlength+10);
	
		memset(text, 0, textlength+10);
	
		
		textlength = CBase64Utils::Decode((BYTE*)parm1, text, lstrlen(parm1));
	

		// Now create and initialize the Blowfish object
		Blowfish b;
	

		b.Set_Passwd((char *)key, keylength);
	

		b.Decrypt((char *)text, textlength);
		

		while (textlength > 0 && text [textlength - 1] == 0) {
			textlength--;
		}	

		// save to stack the decrypted text
		pushstring((char *)text);
		pushstring("0");

		if (text != NULL) {
			LocalFree(text);
			text = NULL;
		}
		
	}

	// release the memory allocated by dynamic variables
		
	if (key != NULL) {
		LocalFree(key);
		key = NULL;
	}
		
	if (parm2 != NULL) {
		GlobalFree((HGLOBAL)parm1);
		parm2 = NULL;
	}
		
	if (parm2 != NULL) {
		GlobalFree((HGLOBAL)parm2);
		parm2 = NULL;
	}
}


NSISFunc(encrypt)
{
	EXDLL_INIT();// initialize stack & variables we are using to access coresponding stack&variables of NSIS

	BYTE * text = NULL;
	BYTE * text1 = NULL;
	BYTE * key = NULL;
	long textlength = 0;
	long keylength = 0;

	char * parm1 =NULL;
	char * parm2 =NULL;

	//read 1st param from stack; it represents then message to be encrypted
	stack_t *th;
	if (g_stacktop && *g_stacktop) 
	{
	  th=(*g_stacktop);
	  parm1=(char *)LocalAlloc(LPTR,lstrlen(th->text)+1);

	  lstrcpyA(parm1,th->text);
	  *g_stacktop = th->next;
	  GlobalFree((HGLOBAL)th);
	}
  
	if (lstrcmp(parm1, "--End--") == 0)
		parm1= NULL;

	//read 2nd param from stack; it represents then password key used to encrypt the text
	if (g_stacktop && *g_stacktop) 
	{
	  th=(*g_stacktop);
	  parm2=(char *)LocalAlloc(LPTR,lstrlen(th->text)+1);

	  lstrcpyA(parm2,th->text);
	  *g_stacktop = th->next;
	  GlobalFree((HGLOBAL)th);
	}
	
	//if 1st param not supplied then push into stack an error message
	if ( parm1==NULL )
    {
        pushstring(ERR_USAGE_encrypt);
		pushstring("1");
        return;
    }

	//if 2nd param not supplied then push into stack an error message
	if ( parm2==NULL )
    {
        pushstring(ERR_USAGE_encrypt);
		pushstring("1");
        return;
    }

	// Get our key length
	keylength = CBase64Utils::CalculateDecodeBufferSize(lstrlen(parm2));
	key = (BYTE *)LocalAlloc(LPTR,keylength+10);
	memset(key, 0,  keylength + 10);

	// Now decode our key from Base64
	keylength = CBase64Utils::Decode((BYTE*)parm2, key, lstrlen(parm2));	

	if (keylength < 8 || keylength > 56) // Is our keylength between 32 and 448 bits?
	{
		pushstring(ERR_KEY_SIZE);
		pushstring("1");
	}
	else 
	{
		// Now create and initialize the Blowfish object
		Blowfish b;
		b.Set_Passwd((char *)key, keylength);
		text1 = (BYTE*)LocalAlloc(LPTR,lstrlen(parm1)+ 8);
		memset (text1, 0, lstrlen(parm1) + 8);
		memcpy (text1, parm1, lstrlen(parm1));
		ULONG enc_len = (lstrlen(parm1) % 8 == 0) ? lstrlen(parm1) : lstrlen(parm1) + (8 - (lstrlen(parm1) % 8));
		b.Encrypt((char *)text1, enc_len);

		// Now get our text length
		textlength = CBase64Utils::CalculateEncodeBufferSize(enc_len);
		text = (BYTE*)LocalAlloc(LPTR,textlength + 16);
		memset(text, 0, textlength + 16);
		CBase64Utils::Encode(text1, text, enc_len);

		// save to stack the decrypted text
		pushstring((char *)text);
		pushstring("0");

		if (text != NULL) {
			LocalFree(text);
			text = NULL;
		}
		
		if (text1 != NULL) {
			LocalFree(text1);
			text1 = NULL;
		}
	}

	// release the memory allocated by dynamic variables
		
	if (key != NULL) {
		LocalFree(key);
		key = NULL;
	}
		
	if (parm2 != NULL) {
		GlobalFree((HGLOBAL)parm1);
		parm2 = NULL;
	}
		
	if (parm2 != NULL) {
		GlobalFree((HGLOBAL)parm2);
		parm2 = NULL;
	}
}


BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	g_hInstance=(HINSTANCE)hInst;
	return(TRUE);
}
