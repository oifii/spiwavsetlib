/*
 * Copyright (c) 2012-2016 Stephane Poirier
 *
 * stephane.poirier@oifii.org
 *
 * Stephane Poirier
 * 3532 rue Ste-Famille, #3
 * Montreal, QC, H2X 2L1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// spiwavsetlib.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "spiwavsetlib.h"
#include <stdio.h>

HWND global_hwnd=NULL;
int global_idstatic=-1;
int	global_staticwidth=-1;
int	global_staticheight=-1;
int	global_fontwidth=-1;
int	global_fontheight=-1;
int global_staticalignment=-1;
#define MAX_GLOBALASCIITEXT	4096
CHAR global_asciitext[MAX_GLOBALASCIITEXT+1];
#define MAX_GLOBALTEXT	4096
WCHAR global_text[MAX_GLOBALTEXT+1];
FILE* global_pfile=NULL;

CHAR pCHAR[1024];
WCHAR pWCHAR[1024];


void WavSetLib_Initialize(HWND hWnd, int nIDStatic, int nStaticWidth_inpixel, int nStaticHeight_inpixel, int nStaticFontWidth_inpixel, int nStaticFontHeight_inpixel, int nStaticAlignment, FILE* pFILE)
{
	global_hwnd = hWnd; //i.e. hwnd dlg window or the hwnd parent for the static 
	global_idstatic = nIDStatic; //i.e. id dlg item for the static
	global_staticwidth = nStaticWidth_inpixel;
	global_staticheight = nStaticHeight_inpixel;
	global_fontwidth = nStaticFontWidth_inpixel;
	global_fontheight = nStaticFontHeight_inpixel;
	global_staticalignment = nStaticAlignment;

	global_pfile = pFILE;
}

void WavSetLib_Terminate()
{
}

void InvalidateControlClientRect(HWND hwnd)
{
	RECT myScreenRECT;
	GetWindowRect(hwnd, &myScreenRECT);
	POINT myTopLeftPOINT;
	myTopLeftPOINT.x = myScreenRECT.left;
	myTopLeftPOINT.y = myScreenRECT.top;
	POINT myBottomRightPOINT;
	myBottomRightPOINT.x = myScreenRECT.right;
	myBottomRightPOINT.y = myScreenRECT.bottom;
	ScreenToClient(global_hwnd, &myTopLeftPOINT);
	ScreenToClient(global_hwnd, &myBottomRightPOINT);
	RECT myClientRECT;
	myClientRECT.left = myTopLeftPOINT.x;
	myClientRECT.top = myTopLeftPOINT.y;
	myClientRECT.right = myBottomRightPOINT.x;
	myClientRECT.bottom = myBottomRightPOINT.y;
	InvalidateRect(global_hwnd, &myClientRECT, FALSE);
}

int CountSubstringA(const CHAR* str, const CHAR* sub)
{
    int length = strlen(sub);
    if (length == 0) return 0;
    int count = 0;
    for (str = strstr(str, sub); str; str = strstr(str + length, sub))
        ++count;
    return count;
}

int CountSubstringW(const WCHAR* str, const WCHAR* sub)
{
    int length = wcslen(sub);
    if (length == 0) return 0;
    int count = 0;
    for (str = wcsstr(str, sub); str; str = wcsstr(str + length, sub))
        ++count;
    return count;
}

void StatusReplaceText(const CHAR* pszText)
{
	StatusReplaceTextA(pszText);
}

void StatusReplaceText(const WCHAR* lpszText)
{
	StatusReplaceTextW(lpszText);
}

void StatusReplaceTextA(const CHAR* pszText)
{
	if(global_hwnd==NULL || global_idstatic==-1 || global_staticwidth==-1 || global_staticheight==-1 || global_fontwidth==-1 || global_fontheight==-1)
	{
		printf(pszText);
	}
	else
	{
		//HWND hStatic = GetDlgItem(global_hwnd, IDC_MAIN_STATIC);
		HWND hStatic = GetDlgItem(global_hwnd, global_idstatic);
		InvalidateControlClientRect(hStatic);
		SetWindowTextA(hStatic, pszText);
	}
}

void StatusReplaceTextW(const WCHAR* lpszText)
{
	if(global_hwnd==NULL || global_idstatic==-1 || global_staticwidth==-1 || global_staticheight==-1 || global_fontwidth==-1 || global_fontheight==-1)
	{
		wprintf(lpszText);
	}
	else
	{
		//HWND hStatic = GetDlgItem(global_hwnd, IDC_MAIN_STATIC);
		HWND hStatic = GetDlgItem(global_hwnd, global_idstatic);
		InvalidateControlClientRect(hStatic);
		SetWindowTextW(hStatic, lpszText);
	}
}

void StatusAddText(const CHAR* pszText)
{
	StatusAddTextA(pszText);
}

void StatusAddText(const WCHAR* lpszText)
{
	StatusAddTextW(lpszText);
}

void StatusAddTextA(const CHAR* pszText)
{
	if(global_hwnd==NULL || global_idstatic==-1 || global_staticwidth==-1 || global_staticheight==-1 || global_fontwidth==-1 || global_fontheight==-1)
	{
		printf("%s", pszText);
	}
	else
	{
		int maxnumberofchar = global_staticwidth*global_staticheight/(global_fontwidth*global_fontheight);  
		int maxnumberofline = global_staticheight/global_fontheight;  

		int llength = strlen(pszText);

		//HWND hStatic = GetDlgItem(global_hwnd, IDC_MAIN_STATIC);
		HWND hStatic = GetDlgItem(global_hwnd, global_idstatic);
		int glength = GetWindowTextA(hStatic, global_asciitext, MAX_GLOBALASCIITEXT);
		int numberofline = CountSubstringA(global_asciitext, "\n"); //"\r\n"
		if((llength+glength+2)>=MAX_GLOBALASCIITEXT || (llength+glength+2)>=maxnumberofchar || (numberofline+1)>=maxnumberofline)
		{
			//erase previous global_text
			StatusReplaceTextA(pszText);
		}
		else
		{
			//keep previous global_text
			strcat(global_asciitext, pszText);
			if(global_staticalignment!=0) InvalidateControlClientRect(hStatic);
			SetWindowTextA(hStatic, global_asciitext);
		}	
	}
	if(global_pfile)
	{
		fprintf(global_pfile, "%s", pszText);
	}
}

void StatusAddTextW(const WCHAR* lpszText)
{
	if(global_hwnd==NULL || global_idstatic==-1 || global_staticwidth==-1 || global_staticheight==-1 || global_fontwidth==-1 || global_fontheight==-1)
	{
		wprintf(L"%s", lpszText);
	}
	else
	{
		int maxnumberofchar = global_staticwidth*global_staticheight/(global_fontwidth*global_fontheight);  
		int maxnumberofline = global_staticheight/global_fontheight;  

		int llength = wcslen(lpszText);

		//HWND hStatic = GetDlgItem(global_hwnd, IDC_MAIN_STATIC);
		HWND hStatic = GetDlgItem(global_hwnd, global_idstatic);
		int glength = GetWindowTextW(hStatic, global_text, MAX_GLOBALTEXT);
		int numberofline = CountSubstringW(global_text, L"\n"); //"\r\n"
		if((llength+glength+2)>=MAX_GLOBALTEXT || (llength+glength+2)>=maxnumberofchar || (numberofline+1)>=maxnumberofline)
		{
			//erase previous global_text
			StatusReplaceTextW(lpszText);
		}
		else
		{
			//keep previous global_text
			wcscat(global_text, lpszText);
			if(global_staticalignment!=0) InvalidateControlClientRect(hStatic);
			SetWindowTextW(hStatic, global_text);
		}	
	}
	if(global_pfile)
	{
		fwprintf(global_pfile, L"%s", lpszText);
	}
}
