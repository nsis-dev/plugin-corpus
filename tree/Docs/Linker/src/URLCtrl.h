// urlctrl - altered to support listview columns, Jonathan Abramson 2011
//
//  based upon "urlctrl" by jbrown, altered static text ctrl - bittmann 2004
//  please visit: http://www.catch22.net/tuts/
//
// note: is using SetWindowLong(GWL_USERDATA)
// note: include "urlctrl.h" after <windows.h> and <tchar.h>

#ifndef _URLCTRL_H
#define _URLCTRL_H

#ifdef __cplusplus
extern "C" {
#endif


// supported window MESSAGES:
//  WM_SETFONT     - set font (self-modifying: always underlined).
//  WM_SETTEXT     - set display text.
//                   maximum of MAX_PATH TCHARs (including terminator).


// CONVERT STATIC CONTROL TO URLCTRL
// may be called n-times on the same window: first call
// creates the urlcrtl, further calls do updates.
//
//  hwnd           - static control window.
//  url            - url to open (not display text).
//                   if url is NULL, window text is used as url.
//                   maximum of MAX_PATH TCHARs (including terminator).
//  unvisited      - color of unvisited link (NULL=default).
//  visited        - color of visited link (NULL=default).
//  flags          - combination of UCF_xxx values.

#define UCF_TXT_DEFAULT  0 //text orientation...
#define UCF_TXT_LEFT     0
#define UCF_TXT_RIGHT    1
#define UCF_TXT_HCENTER  2
#define UCF_TXT_TOP      0
#define UCF_TXT_VCENTER  4
#define UCF_TXT_BOTTOM   8 //...text orientation
#define UCF_LNK_VISITED 16 //link visited
#define UCF_KBD         32 //keyboard support (tabbing,focus,space key)
#define UCF_FIT         64 //automatic resizing

#define MAX_LV_ITEMS 40
#define MAX_LV_SUBITEMS 10

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <shellapi.h> //shellexecute
#include <Commctrl.h>


#define this_malloc(size) (void*)GlobalAlloc(GMEM_FIXED,(size))
#define this_free(ptr)    GlobalFree(ptr)

typedef struct
{	TCHAR szURL[MAX_PATH];//url!=display
	WNDPROC oldproc;//old window proc
	HCURSOR hcur;//cursor
	HFONT hfont;//underlined font
	COLORREF crUnvisited;//color unvisited
	COLORREF crVisited;//color visited
	BOOL fClicking;//internal state
	DWORD dwFlags;//combination of UCF_xxx values
} URLCTRL;

typedef struct
{	TCHAR szURL[MAX_PATH];//url!=display
	BOOL fClicking;//internal state
} URLCTRL_LV;

typedef struct
{	URLCTRL_LV *urls[MAX_LV_ITEMS][MAX_LV_SUBITEMS];//url!=display
	WNDPROC oldproc;//old window proc
	HCURSOR hcur;//cursor
	HFONT hfont;//underlined font
	COLORREF crUnvisited;//color unvisited
	COLORREF crVisited;//color visited
	BOOL fClicking;//internal state
	DWORD dwFlags;//combination of UCF_xxx values
} URLCTRLS;

typedef struct
{	WNDPROC oldparentproc;//old parent window proc
	URLCTRLS *urlctrls;
	int idFrom; //original idFrom
} PARENTCTRL;


BOOL util_url_draw(HWND,HDC,RECT*);//draw (LPRECT==NULL) OR calc (LPRECT!=NULL)
BOOL util_url_fit(HWND,BOOL fRedraw);//resize window
BOOL util_url_open(HWND);//open url
LRESULT CALLBACK urlctrl_proc(HWND,UINT,WPARAM,LPARAM);

BOOL urlctrl_set(HWND,TCHAR *url,COLORREF *unvisited,COLORREF *visited,DWORD flags);

BOOL urlctrl_lv_set(HWND,UINT,UINT,TCHAR *url,COLORREF *unvisited,COLORREF *visited,DWORD flags);

LRESULT ProcessCustomDraw (LPARAM lParam, URLCTRLS* urlctrls);
HFONT CreateUndelineFont(HFONT hFont) ;


// RESIZE URLCTRL TO FIT DISPLAY TEXT
// may be called after urlctrl_set, WM_SETTEXT, WM_SETFONT.
// not needed, if urlctrl has set UCF_FIT flag.

BOOL urlctrl_fit(HWND);


#ifdef __cplusplus
}
#endif

#endif //_URLCTRL_H