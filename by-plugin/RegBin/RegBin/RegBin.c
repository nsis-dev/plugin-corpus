#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "exdll.h"
#define NSIS_MAX_STRLEN 1024

int initCalled = 0;
char *buf = NULL;
int size = 0;
HKEY rootKey = 0;
char subKeyName[NSIS_MAX_STRLEN] = "";
char valueName[NSIS_MAX_STRLEN] = "";

void * my_memcpy(void *outBuf, const void *inBuf, int len)
{
  char *c_out=(char*)outBuf+(len-1);
  char *c_in=(char *)inBuf+(len-1);
  while (len-- > 0)
  {
    *c_out--=*c_in--;
  }
  return outBuf;
}

void reset()
{
    if(buf) {
        GlobalFree(buf);
    }
    buf = NULL;
    size = 0;
    rootKey = 0;
    lstrcpy(subKeyName,"");
    lstrcpy(valueName,"");
}

int appendData(char *input)
{
    char data[NSIS_MAX_STRLEN];
    char *p=input;
    int data_len=0;
    while (*p)
    {
        int c;
        int a,b;
        if (data_len >= NSIS_MAX_STRLEN)
        {
            return 1;
        }
        a=*p;
        if (a >= '0' && a <= '9') a-='0';
        else if (a >= 'a' && a <= 'f') a-='a'-10;
        else if (a >= 'A' && a <= 'F') a-='A'-10;
        else break;
        b=*++p;
        if (b >= '0' && b <= '9') b-='0';
        else if (b >= 'a' && b <= 'f') b-='a'-10;
        else if (b >= 'A' && b <= 'F') b-='A'-10;
        else break;
        p++;
        c=(a<<4)|b;
        data[data_len++]=c;
    }
    if(*p) {
        return 1;
    }
    if(buf) {
        char *temp = (char *)GlobalAlloc(GPTR,(size + data_len)*sizeof(char));
        my_memcpy(temp, buf, size);
        my_memcpy((temp+size), data, data_len);
        GlobalFree(buf);
        buf = temp;
        size += data_len;
    }
    else {
        buf = (char *)GlobalAlloc(GPTR,(data_len)*sizeof(char));
        my_memcpy(buf, data, data_len);
        size = data_len;
    }
    return 0;
}

int popData()
{
    char data[NSIS_MAX_STRLEN];
    popstring(data);
    return appendData(data);
}

int popArgs()
{
    char rootKeyName[NSIS_MAX_STRLEN];
    int rv = 0;

    popstring(rootKeyName);
    popstring(subKeyName);
    popstring(valueName);
    if((rv = popData())) {
        return rv;
    }

    if(!lstrcmpi(rootKeyName,"HKCR") || !lstrcmpi(rootKeyName,"HKEY_CLASSES_ROOT")) {
        rootKey = HKEY_CLASSES_ROOT;
    }
    else if(!lstrcmpi(rootKeyName,"HKLM") || !lstrcmpi(rootKeyName,"HKEY_LOCAL_MACHINE")) {
        rootKey = HKEY_LOCAL_MACHINE;
    }
    else if(!lstrcmpi(rootKeyName,"HKCU") || !lstrcmpi(rootKeyName,"HKEY_CURRENT_USER")) {
        rootKey = HKEY_CURRENT_USER;
    }
    else if(!lstrcmpi(rootKeyName,"HKU") || !lstrcmpi(rootKeyName,"HKEY_USERS")) {
        rootKey = HKEY_USERS;
    }
    else if(!lstrcmpi(rootKeyName,"HKCC") || !lstrcmpi(rootKeyName,"HKEY_CURRENT_CONFIG")) {
        rootKey = HKEY_CURRENT_CONFIG;
    }
    else if(!lstrcmpi(rootKeyName,"HKDD") || !lstrcmpi(rootKeyName,"HKEY_DYN_DATA")) {
        rootKey = HKEY_DYN_DATA;
    }
    else if(!lstrcmpi(rootKeyName,"HKPD") || !lstrcmpi(rootKeyName,"HKEY_PERFORMANCE_DATA")) {
        rootKey = HKEY_PERFORMANCE_DATA;
    }
    else {
        rv = 1;
    }
    return rv;
}

int doWrite()
{
    HKEY hk;
    int rv = 0;

    if(!(rv = RegCreateKey(rootKey, subKeyName, &hk))) {
        rv = RegSetValueEx(hk,valueName,0,REG_BINARY,buf,size);
        RegCloseKey(hk);
    }
    reset();
    return rv;
}

BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}

void __declspec(dllexport) init(HWND hwndParent, int string_size, 
                                char *variables, stack_t **stacktop)
{
    EXDLL_INIT();

    if (initCalled) {
        reset();
        pushstring("error");
        return;
    }
    initCalled++;
    if(popArgs()) {
        pushstring("error");
    }
    else {
        pushstring("success");
    }
}

void __declspec(dllexport) append(HWND hwndParent, int string_size, 
                                  char *variables, stack_t **stacktop)
{
    EXDLL_INIT();
    if (!initCalled) {
        pushstring("error");
        return;
    }

    if(popData()) {
        pushstring("error");
    }
    else {
        pushstring("success");
    }
}

void __declspec(dllexport) write(HWND hwndParent, int string_size, 
                                 char *variables, stack_t **stacktop)
{
    EXDLL_INIT();
    if (!initCalled) {
        pushstring("error");
        return;
    }
    initCalled--;

    if(popData()) {
        pushstring("error");
    }
    else {
        if(doWrite()) {
            pushstring("error");
        }
        else {
            pushstring("success");
        }
    }
}

void __declspec(dllexport) writeRegBin(HWND hwndParent, int string_size, 
                                    char *variables, stack_t **stacktop)
{
    EXDLL_INIT();
    if (initCalled) {
        reset();
        pushstring("error");
        return;
    }
    if(popArgs()) {
        pushstring("error");
    }
    else {
        if(doWrite()) {
            pushstring("error");
        }
        else {
            pushstring("success");
        }
    }
}
