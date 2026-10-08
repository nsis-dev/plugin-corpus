/* wa_dlg.h
 *
 * Modified for use in skinned button by SuperPat
 * homepage http://www.ultramodernui.com4.ws
 *
 * based on the wa_dgl.h file of wansis, a Plug-in written by Saivert that skins NSIS like Winamp.
 * wansis homepage: http://saivert.inthegray.com
 *
 * Original version can be downloaded at http://www.winamp.com/nsdn
 * Following is original license (which still applies):
 *
 * ------------------------------------------------------------------------------------------------------------            
 *
 * Copyright (C) 2003 Nullsoft, Inc.
 *
 * This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held 
 * liable for any damages arising from the use of this software. 
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial applications, and to 
 * alter it and redistribute it freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. 
 *      If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any source distribution.
 *
 */

void WADlg_init(HBITMAP); // call this on init, or on WM_DISPLAYCHANGE
void WADlg_close();
int WADlg_handleDialogMsgs(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam); 

static HBITMAP wadlg_bitmap; // load this manually

// Modification Slappy, 2010
static COLORREF wadlg_color; // Color of button text

void WADlg_init(HBITMAP hbm, COLORREF hcolor) // call this on init, or on WM_DISPLAYCHANGE
{
  if (wadlg_bitmap)
     DeleteObject(wadlg_bitmap);
     
  wadlg_bitmap = hbm;
  wadlg_color = hcolor;
}

void WADlg_close()
{
  if (wadlg_bitmap)
     DeleteObject(wadlg_bitmap);
  wadlg_bitmap=0;
}

int WADlg_handleDialogMsgs(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Skin a BUTTON
	if (uMsg == WM_DRAWITEM)
	{
		HDC hdc;
		int yoffs;
		DRAWITEMSTRUCT *di = (DRAWITEMSTRUCT *)lParam;
		if (di->CtlType == ODT_BUTTON)
        {
			char wt[256];
			RECT r;
			WNDCLASS wc;

			GetDlgItemText(hwndDlg,wParam,wt,sizeof(wt));

			// If window (control) has been subclassed, we skip it.
			// This was added to handle InstallOptions pages which have
			// one or more Link controls. InstallOptions.dll do it's own
			// owner-drawing on such controls.
			GetClassInfo(g_hInstance, "BUTTON", &wc);
			if (GetWindowLong(di->hwndItem, GWL_WNDPROC) != (LONG)wc.lpfnWndProc)
				return FALSE;

              hdc = CreateCompatibleDC(di->hDC);
              SelectObject(hdc,wadlg_bitmap);
        
        	  r=di->rcItem;
              SetStretchBltMode(di->hDC,COLORONCOLOR);
        
        
              yoffs = (di->itemState & ODS_SELECTED) ? 15 : 0;
    
              BitBlt(di->hDC,r.left,r.top,4,4,hdc,0,yoffs,SRCCOPY); // top left
              StretchBlt(di->hDC,r.left+4,r.top,r.right-r.left-4-4,4,hdc,4,yoffs,47-4-4,4,SRCCOPY); // top center
              BitBlt(di->hDC,r.right-4,r.top,4,4,hdc,47-4,yoffs,SRCCOPY); // top right
        
              StretchBlt(di->hDC,r.left,r.top+4,4,r.bottom-r.top-4-4,hdc,0,4+yoffs,4,15-4-4,SRCCOPY); // left edge
              StretchBlt(di->hDC,r.right-4,r.top+4,4,r.bottom-r.top-4-4,hdc,47-4,4+yoffs,4,15-4-4,SRCCOPY); // right edge
              // center
              StretchBlt(di->hDC,r.left+4,r.top+4,r.right-r.left-4-4,r.bottom-r.top-4-4,hdc,4,4+yoffs,47-4-4,15-4-4,SRCCOPY);
                
              BitBlt(di->hDC,r.left,r.bottom-4,4,4,hdc,0,15-4+yoffs,SRCCOPY); // bottom left
              StretchBlt(di->hDC,r.left+4,r.bottom-4,r.right-r.left-4-4,4,hdc,4,15-4+yoffs,47-4-4,4,SRCCOPY); // bottom center
              BitBlt(di->hDC,r.right-4,r.bottom-4,4,4,hdc,47-4,15-4+yoffs,SRCCOPY); // bottom right
                
        	  // draw text
              SetBkMode(di->hDC,TRANSPARENT);
			  SetTextColor(di->hDC, (di->itemState & ODS_DISABLED) ? 0x00808080 : wadlg_color);
        	  //SetTextColor(di->hDC, (di->itemState & ODS_DISABLED)?0x00808080:wadlg_colors[WADLG_BUTTONFG]);
              if (di->itemState & ODS_SELECTED) {r.left+=2; r.top+=2;}
        			DrawText(di->hDC,wt,-1,&r,DT_VCENTER|DT_SINGLELINE|DT_CENTER);
	
              DeleteDC(hdc);
		}	
	}

  return 0;
}
