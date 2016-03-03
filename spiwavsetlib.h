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

// spiwavsetlib.h

#ifdef SPIWAVSETLIB_EXPORTS
#define SPIWAVSETLIB_API __declspec(dllexport) 
#else
#define SPIWAVSETLIB_API __declspec(dllimport) 
#endif

#include <windows.h>
#include <stdio.h>

void SPIWAVSETLIB_API WavSetLib_Initialize(HWND hWnd, int nIDStatic, int nStaticWidth_inpixel, int nStaticHeight_inpixel, int nStaticFontWidth_inpixel, int nStaticFontHeight_inpixel, int nStaticAlignment, FILE* pFILE=NULL);
void SPIWAVSETLIB_API WavSetLib_Terminate();

void SPIWAVSETLIB_API StatusReplaceText(const CHAR* pszText);
void SPIWAVSETLIB_API StatusReplaceText(const WCHAR* lpszText);
void SPIWAVSETLIB_API StatusReplaceTextA(const CHAR* pszText);
void SPIWAVSETLIB_API StatusReplaceTextW(const WCHAR* lpszText);
void SPIWAVSETLIB_API StatusAddText(const CHAR* pszText);
void SPIWAVSETLIB_API StatusAddText(const WCHAR* lpszText);
void SPIWAVSETLIB_API StatusAddTextA(const CHAR* pszText);
void SPIWAVSETLIB_API StatusAddTextW(const WCHAR* lpszText);


#include <string>
#include <fstream>
#include <vector>

#include <iostream>
#include <sstream>
using namespace std;

//#include <stdio.h>
#include <stdlib.h>
#include "portaudio.h"

#include <ctime>


#include "spiws_wavset.h"
#include "spiws_instrument.h"
#include "spiws_instrumentset.h"
#include "spiws_midievent.h"
#include "spiws_midieventset.h"
#include "spiws_midiutility.h"
#include "spiws_partition.h"
#include "spiws_partitionset.h"

