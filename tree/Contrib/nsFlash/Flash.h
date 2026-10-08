#ifndef __FLASH_H__
#define __FLASH_H__

#include <comdef.h>

namespace Flash
{
	struct __declspec(uuid("d27cdb6c-ae6d-11cf-96b8-444553540000")) IShockwaveFlash : IDispatch
	{
		virtual HRESULT __stdcall get_ReadyState ( /*[out,retval]*/ long * pVal ) = 0;
		virtual HRESULT __stdcall get_TotalFrames ( /*[out,retval]*/ long * pVal ) = 0;
		virtual HRESULT __stdcall get_Playing ( /*[out,retval]*/ VARIANT_BOOL * pVal ) = 0;
		virtual HRESULT __stdcall put_Playing ( /*[in]*/ VARIANT_BOOL pVal ) = 0;
		virtual HRESULT __stdcall get_Quality ( /*[out,retval]*/ int * pVal ) = 0;
		virtual HRESULT __stdcall put_Quality ( /*[in]*/ int pVal ) = 0;
		virtual HRESULT __stdcall get_ScaleMode ( /*[out,retval]*/ int * pVal ) = 0;
		virtual HRESULT __stdcall put_ScaleMode ( /*[in]*/ int pVal ) = 0;
		virtual HRESULT __stdcall get_AlignMode ( /*[out,retval]*/ int * pVal ) = 0;
		virtual HRESULT __stdcall put_AlignMode ( /*[in]*/ int pVal ) = 0;
		virtual HRESULT __stdcall get_BackgroundColor ( /*[out,retval]*/ long * pVal ) = 0;
		virtual HRESULT __stdcall put_BackgroundColor ( /*[in]*/ long pVal ) = 0;
		virtual HRESULT __stdcall get_Loop ( /*[out,retval]*/ VARIANT_BOOL * pVal ) = 0;
		virtual HRESULT __stdcall put_Loop ( /*[in]*/ VARIANT_BOOL pVal ) = 0;
		virtual HRESULT __stdcall get_Movie ( /*[out,retval]*/ BSTR * pVal ) = 0;
		virtual HRESULT __stdcall put_Movie ( /*[in]*/ BSTR pVal ) = 0;
		virtual HRESULT __stdcall get_FrameNum ( /*[out,retval]*/ long * pVal ) = 0;
		virtual HRESULT __stdcall put_FrameNum ( /*[in]*/ long pVal ) = 0;
		virtual HRESULT __stdcall raw_SetZoomRect ( /*[in]*/ long left, /*[in]*/ long top, /*[in]*/ long right, /*[in]*/ long bottom ) = 0;
		virtual HRESULT __stdcall raw_Zoom ( /*[in]*/ int factor ) = 0;
		virtual HRESULT __stdcall raw_Pan ( /*[in]*/ long x, /*[in]*/ long y, /*[in]*/ int mode ) = 0;
		virtual HRESULT __stdcall raw_Play ( ) = 0;
		virtual HRESULT __stdcall raw_Stop ( ) = 0;
		virtual HRESULT __stdcall raw_Back ( ) = 0;
		virtual HRESULT __stdcall raw_Forward ( ) = 0;
		virtual HRESULT __stdcall raw_Rewind ( ) = 0;
		virtual HRESULT __stdcall raw_StopPlay ( ) = 0;
		virtual HRESULT __stdcall raw_GotoFrame ( /*[in]*/ long FrameNum ) = 0;
		virtual HRESULT __stdcall raw_CurrentFrame ( /*[out,retval]*/ long * FrameNum ) = 0;
		virtual HRESULT __stdcall raw_IsPlaying ( /*[out,retval]*/ VARIANT_BOOL * Playing ) = 0;
		virtual HRESULT __stdcall raw_PercentLoaded ( /*[out,retval]*/ long * percent ) = 0;
		virtual HRESULT __stdcall raw_FrameLoaded ( /*[in]*/ long FrameNum, /*[out,retval]*/ VARIANT_BOOL * loaded ) = 0;
		virtual HRESULT __stdcall raw_FlashVersion ( /*[out,retval]*/ long * version ) = 0;
		virtual HRESULT __stdcall get_WMode ( /*[out,retval]*/ BSTR * pVal ) = 0;
		virtual HRESULT __stdcall put_WMode ( /*[in]*/ BSTR pVal ) = 0;
		virtual HRESULT __stdcall get_SAlign ( /*[out,retval]*/ BSTR * pVal ) = 0;
		virtual HRESULT __stdcall put_SAlign ( /*[in]*/ BSTR pVal ) = 0;
		virtual HRESULT __stdcall get_Menu ( /*[out,retval]*/ VARIANT_BOOL * pVal ) = 0;
		virtual HRESULT __stdcall put_Menu ( /*[in]*/ VARIANT_BOOL pVal ) = 0;
		virtual HRESULT __stdcall get_Base ( /*[out,retval]*/ BSTR * pVal ) = 0;
		virtual HRESULT __stdcall put_Base ( /*[in]*/ BSTR pVal ) = 0;
		virtual HRESULT __stdcall get_Scale ( /*[out,retval]*/ BSTR * pVal ) = 0;
		virtual HRESULT __stdcall put_Scale ( /*[in]*/ BSTR pVal ) = 0;
		virtual HRESULT __stdcall get_DeviceFont ( /*[out,retval]*/ VARIANT_BOOL * pVal ) = 0;
		virtual HRESULT __stdcall put_DeviceFont ( /*[in]*/ VARIANT_BOOL pVal ) = 0;
		virtual HRESULT __stdcall get_EmbedMovie ( /*[out,retval]*/ VARIANT_BOOL * pVal ) = 0;
		virtual HRESULT __stdcall put_EmbedMovie ( /*[in]*/ VARIANT_BOOL pVal ) = 0;
		virtual HRESULT __stdcall get_BGColor ( /*[out,retval]*/ BSTR * pVal ) = 0;
		virtual HRESULT __stdcall put_BGColor ( /*[in]*/ BSTR pVal ) = 0;
		virtual HRESULT __stdcall get_Quality2 ( /*[out,retval]*/ BSTR * pVal ) = 0;
		virtual HRESULT __stdcall put_Quality2 ( /*[in]*/ BSTR pVal ) = 0;
		virtual HRESULT __stdcall raw_LoadMovie ( /*[in]*/ int layer, /*[in]*/ BSTR url ) = 0;
		virtual HRESULT __stdcall raw_TGotoFrame ( /*[in]*/ BSTR target, /*[in]*/ long FrameNum ) = 0;
		virtual HRESULT __stdcall raw_TGotoLabel ( /*[in]*/ BSTR target, /*[in]*/ BSTR label ) = 0;
		virtual HRESULT __stdcall raw_TCurrentFrame ( /*[in]*/ BSTR target, /*[out,retval]*/ long * FrameNum ) = 0;
		virtual HRESULT __stdcall raw_TCurrentLabel ( /*[in]*/ BSTR target, /*[out,retval]*/ BSTR * pVal ) = 0;
		virtual HRESULT __stdcall raw_TPlay ( /*[in]*/ BSTR target ) = 0;
		virtual HRESULT __stdcall raw_TStopPlay ( /*[in]*/ BSTR target ) = 0;
		virtual HRESULT __stdcall raw_SetVariable ( /*[in]*/ BSTR name, /*[in]*/ BSTR value ) = 0;
		virtual HRESULT __stdcall raw_GetVariable ( /*[in]*/ BSTR name, /*[out,retval]*/ BSTR * pVal ) = 0;
		virtual HRESULT __stdcall raw_TSetProperty ( /*[in]*/ BSTR target, /*[in]*/ int property, /*[in]*/ BSTR value ) = 0;
		virtual HRESULT __stdcall raw_TGetProperty ( /*[in]*/ BSTR target, /*[in]*/ int property, /*[out,retval]*/ BSTR * pVal ) = 0;
		virtual HRESULT __stdcall raw_TCallFrame ( /*[in]*/ BSTR target, /*[in]*/ int FrameNum ) = 0;
		virtual HRESULT __stdcall raw_TCallLabel ( /*[in]*/ BSTR target, /*[in]*/ BSTR label ) = 0;
		virtual HRESULT __stdcall raw_TSetPropertyNum ( /*[in]*/ BSTR target, /*[in]*/ int property, /*[in]*/ double value ) = 0;
		virtual HRESULT __stdcall raw_TGetPropertyNum ( /*[in]*/ BSTR target, /*[in]*/ int property, /*[out,retval]*/ double * pVal ) = 0;
		virtual HRESULT __stdcall raw_TGetPropertyAsNumber ( /*[in]*/ BSTR target, /*[in]*/ int property, /*[out,retval]*/ double * pVal ) = 0;
		virtual HRESULT __stdcall get_SWRemote ( /*[out,retval]*/ BSTR * pVal ) = 0;
		virtual HRESULT __stdcall put_SWRemote ( /*[in]*/ BSTR pVal ) = 0;
		virtual HRESULT __stdcall get_FlashVars ( /*[out,retval]*/ BSTR * pVal ) = 0;
		virtual HRESULT __stdcall put_FlashVars ( /*[in]*/ BSTR pVal ) = 0;
		virtual HRESULT __stdcall get_AllowScriptAccess ( /*[out,retval]*/ BSTR * pVal ) = 0;
		virtual HRESULT __stdcall put_AllowScriptAccess ( /*[in]*/ BSTR pVal ) = 0;
		virtual HRESULT __stdcall get_MovieData ( /*[out,retval]*/ BSTR * pVal ) = 0;
		virtual HRESULT __stdcall put_MovieData ( /*[in]*/ BSTR pVal ) = 0;
		virtual HRESULT __stdcall get_InlineData ( /*[out,retval]*/ IUnknown * * ppIUnknown ) = 0;
		virtual HRESULT __stdcall put_InlineData ( /*[in]*/ IUnknown * ppIUnknown ) = 0;
		virtual HRESULT __stdcall get_SeamlessTabbing ( /*[out,retval]*/ VARIANT_BOOL * pVal ) = 0;
		virtual HRESULT __stdcall put_SeamlessTabbing ( /*[in]*/ VARIANT_BOOL pVal ) = 0;
		virtual HRESULT __stdcall raw_EnforceLocalSecurity ( ) = 0;
		virtual HRESULT __stdcall get_Profile ( /*[out,retval]*/ VARIANT_BOOL * pVal ) = 0;
		virtual HRESULT __stdcall put_Profile ( /*[in]*/ VARIANT_BOOL pVal ) = 0;
		virtual HRESULT __stdcall get_ProfileAddress ( /*[out,retval]*/ BSTR * pVal ) = 0;
		virtual HRESULT __stdcall put_ProfileAddress ( /*[in]*/ BSTR pVal ) = 0;
		virtual HRESULT __stdcall get_ProfilePort ( /*[out,retval]*/ long * pVal ) = 0;
		virtual HRESULT __stdcall put_ProfilePort ( /*[in]*/ long pVal ) = 0;
		virtual HRESULT __stdcall raw_CallFunction ( /*[in]*/ BSTR request, /*[out,retval]*/ BSTR * response ) = 0;
		virtual HRESULT __stdcall raw_SetReturnValue ( /*[in]*/ BSTR returnValue ) = 0;
		virtual HRESULT __stdcall raw_DisableLocalSecurity ( ) = 0;
		virtual HRESULT __stdcall get_AllowNetworking ( /*[out,retval]*/ BSTR * pVal ) = 0;
		virtual HRESULT __stdcall put_AllowNetworking ( /*[in]*/ BSTR pVal ) = 0;
		virtual HRESULT __stdcall get_AllowFullScreen ( /*[out,retval]*/ BSTR * pVal ) = 0;
		virtual HRESULT __stdcall put_AllowFullScreen ( /*[in]*/ BSTR pVal ) = 0;
		virtual HRESULT __stdcall get_AllowFullScreenInteractive ( /*[out,retval]*/ BSTR * pVal ) = 0;
		virtual HRESULT __stdcall put_AllowFullScreenInteractive ( /*[in]*/ BSTR pVal ) = 0;
		virtual HRESULT __stdcall get_IsDependent ( /*[out,retval]*/ VARIANT_BOOL * pVal ) = 0;
		virtual HRESULT __stdcall put_IsDependent ( /*[in]*/ VARIANT_BOOL pVal ) = 0;
	};

	struct __declspec(uuid("d27cdb6e-ae6d-11cf-96b8-444553540000")) ShockwaveFlash;
}

#endif