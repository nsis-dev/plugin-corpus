// PixelShader.cpp : Defines the entry point for the console application.
//

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <d3d9.h>
#include "exdll.h"

typedef IDirect3D9* (WINAPI *DIRECT3DCREATE9)( UINT );
const int sdk9_version = 31;

void __declspec(dllexport) GetPixelShaderVersion(HWND hwndParent, int string_size,
                                      char *variables, stack_t **stacktop)
{
	EXDLL_INIT();
	{
		HMODULE d3d9dll;
		DIRECT3DCREATE9 procDirect3DCreate9;
		IDirect3D9* p_d3d;
		D3DCAPS9 caps;
		char szBuf[MAX_PATH];
		int major, minor;


		// Load Direct3D9 library
		d3d9dll = LoadLibrary("D3D9.DLL");
		if ( d3d9dll == NULL ) {
			pushstring("error");
			return;
		}

		// Get access to Direct3DCreate9 function
		procDirect3DCreate9 = (DIRECT3DCREATE9)GetProcAddress(d3d9dll, "Direct3DCreate9");
		if ( procDirect3DCreate9 == NULL ) {
			pushstring("error");
			FreeLibrary(d3d9dll);
			return;
		}

		// Get access to IDirect3D9
		p_d3d = procDirect3DCreate9( sdk9_version );
		if ( p_d3d == NULL ) {
			pushstring("error");
			FreeLibrary(d3d9dll);
			return;
		}

		ZeroMemory(&caps, sizeof(caps));
		caps.DeviceType = D3DDEVTYPE_HAL;
		caps.AdapterOrdinal = D3DADAPTER_DEFAULT;
		if(D3D_OK != IDirect3D9_GetDeviceCaps(p_d3d , D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps ))
		{
			pushstring("error");
			return;
		}

		major = D3DSHADER_VERSION_MAJOR(caps.PixelShaderVersion);
		minor = D3DSHADER_VERSION_MINOR(caps.PixelShaderVersion);

		sprintf(szBuf, "%d", major);
		setuservariable(INST_0, szBuf);

		sprintf(szBuf, "%d", minor);
		setuservariable(INST_1, szBuf);

		IDirect3D9_Release(p_d3d);

		// Unload library
		FreeLibrary(d3d9dll);
	}
}

BOOL APIENTRY DllMain(HANDLE hInst, unsigned long ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}