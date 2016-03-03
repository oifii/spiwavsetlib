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

#include "stdafx.h"

//#define SPIWAVSETLIB_API
#include "spiwavsetlib.h"


#include <string>
using namespace std;
#include <assert.h>

extern CHAR pCHAR[1024];
extern WCHAR pWCHAR[1024];

/////////////////////////////////////////
//MIDI_note_number_to_frequency_chart.png
/////////////////////////////////////////
//midi note range from 0 to 127, from C -1 to G9

//   do           re           mi    fa           sol          la           si
const char* notename[132] =
{
    "C-1", "C#-1", "D-1", "D#-1", "E-1", "F-1", "F#-1", "G-1", "G#-1", "A-1", "A#-1", "B-1",  
    "C0", "C#0", "D0", "D#0", "E0", "F0", "F#0", "G0", "G#0", "A0", "A#0", "B0",  
    "C1", "C#1", "D1", "D#1", "E1", "F1", "F#1", "G1", "G#1", "A1", "A#1", "B1",  
    "C2", "C#2", "D2", "D#2", "E2", "F2", "F#2", "G2", "G#2", "A2", "A#2", "B2",  
    "C3", "C#3", "D3", "D#3", "E3", "F3", "F#3", "G3", "G#3", "A3", "A#3", "B3",  
    "C4", "C#4", "D4", "D#4", "E4", "F4", "F#4", "G4", "G#4", "A4", "A#4", "B4",  
    "C5", "C#5", "D5", "D#5", "E5", "F5", "F#5", "G5", "G#5", "A5", "A#5", "B5",  
    "C6", "C#6", "D6", "D#6", "E6", "F6", "F#6", "G6", "G#6", "A6", "A#6", "B6",  
    "C7", "C#7", "D7", "D#7", "E7", "F7", "F#7", "G7", "G#7", "A7", "A#7", "B7",  
    "C8", "C#8", "D8", "D#8", "E8", "F8", "F#8", "G8", "G#8", "A8", "A#8", "B8",  
    "C9", "C#9", "D9", "D#9", "E9", "F9", "F#9", "G9", "G#9", "A9", "A#9", "B9"
};

const float notefreq[132] =
{
       8.18f,    8.66f,    9.18f,    9.72f,    10.30f,    10.91f,    11.56f,    12.25f,    12.98f,    13.75f,    14.57f,    15.43f, 
      16.35f,   17.32f,   18.35f,   19.45f,    20.60f,    21.83f,    23.12f,    24.50f,    25.96f,    27.50f,    29.14f,    30.87f, 
      32.70f,   34.65f,   36.71f,   38.89f,    41.20f,    43.65f,    46.25f,    49.00f,    51.91f,    55.00f,    58.27f,    61.74f, 
      65.41f,   69.30f,   73.42f,   77.78f,    82.41f,    87.31f,    92.50f,    98.00f,   103.83f,   110.00f,   116.54f,   123.47f, 
     130.81f,  138.59f,  146.83f,  155.56f,   164.81f,   174.61f,   185.00f,   196.00f,   207.65f,   220.00f,   233.08f,   246.94f, 
     261.63f,  277.18f,  293.66f,  311.13f,   329.63f,   349.23f,   369.99f,   392.00f,   415.30f,   440.00f,   466.16f,   493.88f, 
     523.25f,  554.37f,  587.33f,  622.25f,   659.26f,   698.46f,   739.99f,   783.99f,   830.61f,   880.00f,   932.33f,   987.77f, 
    1046.50f, 1108.73f, 1174.66f, 1244.51f,  1318.51f,  1396.91f,  1479.98f,  1567.98f,  1661.22f,  1760.00f,  1864.66f,  1975.53f, 
    2093.00f, 2217.46f, 2349.32f, 2489.02f,  2637.02f,  2793.83f,  2959.96f,  3135.96f,  3322.44f,  3520.00f,  3729.31f,  3951.07f, 
    4186.01f, 4434.92f, 4698.64f, 4978.03f,  5274.04f,  5587.65f,  5919.91f,  6271.92f,  6644.87f,  7040.00f,  7458.62f,  7902.13f, 
    8372.01f, 8869.84f, 9397.27f, 9956.06f, 10548.08f, 11175.30f, 11839.82f, 12543.85f, 13289.75f, 14080.00f, 14917.24f, 15804.26f
};

//note remaping, begin
const char* notenameremap_c_minorpentatonic[132] =
{
    "C0", "C0",  "C0", "C0",  "C0", "C0", "C0",  "C0", "C0",  "C0", "C0",  "C0",  
    "C0", "C0",  "C0", "C0",  "C0", "C0", "C0",  "C0", "C0",  "C0", "C0",  "C0",  
    "C1", "C1",  "C1", "C1",  "G1", "G1", "G1",  "G1", "G1",  "G1", "G2",  "G2",  
    "C2", "C2",  "C2", "D#2", "D#2","F2", "F2",  "G2", "G2",  "G2", "A#2", "A#2",  
    "C3", "C3",  "C3", "D#3", "D#3","F3", "F3",  "G3", "G3",  "G3", "A#3", "A#3",  
	"C4", "C4",  "D4", "D#4", "D#4","F4", "F4",  "G4", "G#4", "A#4","A#4", "A#4",  
	"C4", "C4",  "D4", "D#4", "D#4","F4", "F4",  "G4", "G#4", "A#4","A#4", "A#4",  
	"C4", "C4",  "D4", "D#4", "D#4","F4", "F4",  "G4", "G#4", "A#4","A#4", "A#4",  
	"C4", "C4",  "D4", "D#4", "D#4","F4", "F4",  "G4", "G#4", "A#4","A#4", "A#4",  
	"C4", "C4",  "D4", "D#4", "D#4","F4", "F4",  "G4", "G#4", "A#4","A#4", "A#4",  
	"C4", "C4",  "D4", "D#4", "D#4","F4", "F4",  "G4", "G#4", "A#4","A#4", "A#4"  
};
int GetNoteRemap_C_MinorPentatonic(int midinotenumber)
{
	return GetMidiNoteNumberFromString(notenameremap_c_minorpentatonic[midinotenumber]);
}
const char* notenameremap_c_minorpentatonic_verylow[132] =
{
	"C-1","C-1","D-1","D#-1","D#-1","F-1","F-1","G-1","G#-1","A#-1","A#-1","A#-1",  
	"C0", "C0",  "D0", "D#0", "D#0", "F0", "F0", "G0", "G#0", "A#0", "A#0", "A#0",  
	"C-1","C-1","D-1","D#-1","D#-1","F-1","F-1","G-1","G#-1","A#-1","A#-1","A#-1",  
	"C0", "C0",  "D0", "D#0", "D#0", "F0", "F0", "G0", "G#0", "A#0", "A#0", "A#0",  
	"C-1","C-1","D-1","D#-1","D#-1","F-1","F-1","G-1","G#-1","A#-1","A#-1","A#-1",  
	"C0", "C0",  "D0", "D#0", "D#0", "F0", "F0", "G0", "G#0", "A#0", "A#0", "A#0",  
	"C-1","C-1","D-1","D#-1","D#-1","F-1","F-1","G-1","G#-1","A#-1","A#-1","A#-1",  
	"C0", "C0",  "D0", "D#0", "D#0", "F0", "F0", "G0", "G#0", "A#0", "A#0", "A#0",  
	"C-1","C-1","D-1","D#-1","D#-1","F-1","F-1","G-1","G#-1","A#-1","A#-1","A#-1",  
	"C0", "C0",  "D0", "D#0", "D#0", "F0", "F0", "G0", "G#0", "A#0", "A#0", "A#0",  
	"C0", "C0",  "D0", "D#0", "D#0", "F0", "F0", "G0", "G#0", "A#0", "A#0", "A#0"
};
const char* notenameremap_c_minorpentatonic_low[132] =
{
	"C1", "C1",  "D1", "D#1", "D#1", "F1", "F1", "G1", "G#1", "A#1", "A#1", "A#1",  
	"C2", "C2",  "D2", "D#2", "D#2", "F2", "F2", "G2", "G#2", "A#2", "A#2", "A#2",  
	"C1", "C1",  "D1", "D#1", "D#1", "F1", "F1", "G1", "G#1", "A#1", "A#1", "A#1",  
	"C2", "C2",  "D2", "D#2", "D#2", "F2", "F2", "G2", "G#2", "A#2", "A#2", "A#2",  
	"C1", "C1",  "D1", "D#1", "D#1", "F1", "F1", "G1", "G#1", "A#1", "A#1", "A#1",  
	"C2", "C2",  "D2", "D#2", "D#2", "F2", "F2", "G2", "G#2", "A#2", "A#2", "A#2",  
	"C1", "C1",  "D1", "D#1", "D#1", "F1", "F1", "G1", "G#1", "A#1", "A#1", "A#1",  
	"C2", "C2",  "D2", "D#2", "D#2", "F2", "F2", "G2", "G#2", "A#2", "A#2", "A#2",  
	"C1", "C1",  "D1", "D#1", "D#1", "F1", "F1", "G1", "G#1", "A#1", "A#1", "A#1",  
	"C2", "C2",  "D2", "D#2", "D#2", "F2", "F2", "G2", "G#2", "A#2", "A#2", "A#2",  
	"C2", "C2",  "D2", "D#2", "D#2", "F2", "F2", "G2", "G#2", "A#2", "A#2", "A#2"  
};
const char* notenameremap_c_minorpentatonic_mid[132] =
{
	"C3", "C3",  "D3", "D#3", "D#3", "F3", "F3", "G3", "G#3", "A#3", "A#3", "A#3",  
	"C3", "C3",  "D3", "D#3", "D#3", "F3", "F3", "G3", "G#3", "A#3", "A#3", "A#3",  
	"C4", "C4",  "D4", "D#4", "D#4", "F4", "F4", "G4", "G#4", "A#4", "A#4", "A#4",  
	"C5", "C5",  "D5", "D#5", "D#5", "F5", "F5", "G5", "G#5", "A#5", "A#5", "A#5",  
	"C3", "C3",  "D3", "D#3", "D#3", "F3", "F3", "G3", "G#3", "A#3", "A#3", "A#3",  
	"C4", "C4",  "D4", "D#4", "D#4", "F4", "F4", "G4", "G#4", "A#4", "A#4", "A#4",  
	"C5", "C5",  "D5", "D#5", "D#5", "F5", "F5", "G5", "G#5", "A#5", "A#5", "A#5",  
	"C3", "C3",  "D3", "D#3", "D#3", "F3", "F3", "G3", "G#3", "A#3", "A#3", "A#3",  
	"C4", "C4",  "D4", "D#4", "D#4", "F4", "F4", "G4", "G#4", "A#4", "A#4", "A#4",  
	"C5", "C5",  "D5", "D#5", "D#5", "F5", "F5", "G5", "G#5", "A#5", "A#5", "A#5",  
	"C5", "C5",  "D5", "D#5", "D#5", "F5", "F5", "G5", "G#5", "A#5", "A#5", "A#5"  
};
const char* notenameremap_c_minorpentatonic_high[132] =
{
	"C6", "C6",  "D6", "D#6", "D#6", "F6", "F6", "G6", "G#6", "A#6", "A#6", "A#6",  
	"C6", "C6",  "D6", "D#6", "D#6", "F6", "F6", "G6", "G#6", "A#6", "A#6", "A#6",  
	"C7", "C7",  "D7", "D#7", "D#7", "F7", "F7", "G7", "G#7", "A#7", "A#7", "A#7",  
	"C6", "C6",  "D6", "D#6", "D#6", "F6", "F6", "G6", "G#6", "A#6", "A#6", "A#6",  
	"C7", "C7",  "D7", "D#7", "D#7", "F7", "F7", "G7", "G#7", "A#7", "A#7", "A#7",  
	"C6", "C6",  "D6", "D#6", "D#6", "F6", "F6", "G6", "G#6", "A#6", "A#6", "A#6",  
	"C7", "C7",  "D7", "D#7", "D#7", "F7", "F7", "G7", "G#7", "A#7", "A#7", "A#7",  
	"C6", "C6",  "D6", "D#6", "D#6", "F6", "F6", "G6", "G#6", "A#6", "A#6", "A#6",  
	"C7", "C7",  "D7", "D#7", "D#7", "F7", "F7", "G7", "G#7", "A#7", "A#7", "A#7",  
	"C6", "C6",  "D6", "D#6", "D#6", "F6", "F6", "G6", "G#6", "A#6", "A#6", "A#6",  
	"C7", "C7",  "D7", "D#7", "D#7", "F7", "F7", "G7", "G#7", "A#7", "A#7", "A#7"  
};
const char* notenameremap_c_minorpentatonic_veryhigh[132] =
{
	"C8", "C8",  "D8", "D#8", "D#8", "F8", "F8", "G8", "G#8", "A#8", "A#8", "A#8",  
	"C8", "C8",  "D8", "D#8", "D#8", "F8", "F8", "G8", "G#8", "A#8", "A#8", "A#8",  
	"C9", "C9",  "D9", "D#9", "D#9", "F9", "F9", "G9", "G#8", "A#8", "A#8", "A#8",  
	"C8", "C8",  "D8", "D#8", "D#8", "F8", "F8", "G8", "G#8", "A#8", "A#8", "A#8",  
	"C9", "C9",  "D9", "D#9", "D#9", "F9", "F9", "G9", "G#8", "A#8", "A#8", "A#8",  
	"C8", "C8",  "D8", "D#8", "D#8", "F8", "F8", "G8", "G#8", "A#8", "A#8", "A#8",  
	"C9", "C9",  "D9", "D#9", "D#9", "F9", "F9", "G9", "G#8", "A#8", "A#8", "A#8",  
	"C8", "C8",  "D8", "D#8", "D#8", "F8", "F8", "G8", "G#8", "A#8", "A#8", "A#8",  
	"C9", "C9",  "D9", "D#9", "D#9", "F9", "F9", "G9", "G#8", "A#8", "A#8", "A#8",  
	"C8", "C8",  "D8", "D#8", "D#8", "F8", "F8", "G8", "G#8", "A#8", "A#8", "A#8",  
	"C9", "C9",  "D9", "D#9", "D#9", "F9", "F9", "G9", "G#8", "A#8", "A#8", "A#8"  
};

const char* notenameremap_c3[132] =
{
    "C3", "C3",  "C3", "C3",  "C3", "C3", "C3",  "C3", "C3",  "C3", "C3",  "C3",  
    "C3", "C3",  "C3", "C3",  "C3", "C3", "C3",  "C3", "C3",  "C3", "C3",  "C3",  
    "C3", "C3",  "C3", "C3",  "C3", "C3", "C3",  "C3", "C3",  "C3", "C3",  "C3",  
    "C3", "C3",  "C3", "C3",  "C3", "C3", "C3",  "C3", "C3",  "C3", "C3",  "C3",  
    "C3", "C3",  "C3", "C3",  "C3", "C3", "C3",  "C3", "C3",  "C3", "C3",  "C3",  
    "C3", "C3",  "C3", "C3",  "C3", "C3", "C3",  "C3", "C3",  "C3", "C3",  "C3",  
    "C3", "C3",  "C3", "C3",  "C3", "C3", "C3",  "C3", "C3",  "C3", "C3",  "C3",  
    "C3", "C3",  "C3", "C3",  "C3", "C3", "C3",  "C3", "C3",  "C3", "C3",  "C3",  
    "C3", "C3",  "C3", "C3",  "C3", "C3", "C3",  "C3", "C3",  "C3", "C3",  "C3",  
    "C3", "C3",  "C3", "C3",  "C3", "C3", "C3",  "C3", "C3",  "C3", "C3",  "C3",  
    "C3", "C3",  "C3", "C3",  "C3", "C3", "C3",  "C3", "C3",  "C3", "C3",  "C3"  
};
int GetNoteRemap_C3(int midinotenumber)
{
	return GetMidiNoteNumberFromString(notenameremap_c3[midinotenumber]);
}


int GetNoteRemap(const char* pstring, int midinotenumber)
{
	int midinote=-1;
	if(strcmp(pstring, "NONE")==0)
	{
		return midinotenumber;
	}
	else if(strcmp(pstring, "C_MINORPENTATONIC")==0)
	{
		return GetMidiNoteNumberFromString(notenameremap_c_minorpentatonic[midinotenumber]);
	}
	else if(strcmp(pstring, "CS_MINORPENTATONIC")==0)
	{
		midinote = GetMidiNoteNumberFromString(notenameremap_c_minorpentatonic[midinotenumber]);
		midinote = midinote+1;
		if(midinote>127) midinote=GetMidiNoteNumberFromString("C#8");
		return midinote;
	}
	else if(strcmp(pstring, "D_MINORPENTATONIC")==0)
	{
		midinote = GetMidiNoteNumberFromString(notenameremap_c_minorpentatonic[midinotenumber]);
		midinote = midinote+2;
		if(midinote>127) midinote=GetMidiNoteNumberFromString("D8");
		return midinote;
	}
	else if(strcmp(pstring, "DS_MINORPENTATONIC")==0)
	{
		midinote = GetMidiNoteNumberFromString(notenameremap_c_minorpentatonic[midinotenumber]);
		midinote = midinote+3;
		if(midinote>127) midinote=GetMidiNoteNumberFromString("D#8");
		return midinote;
	}
	else if(strcmp(pstring, "E_MINORPENTATONIC")==0)
	{
		midinote = GetMidiNoteNumberFromString(notenameremap_c_minorpentatonic[midinotenumber]);
		midinote = midinote+4;
		if(midinote>127) midinote=GetMidiNoteNumberFromString("E8");
		return midinote;
	}
	else if(strcmp(pstring, "F_MINORPENTATONIC")==0)
	{
		midinote = GetMidiNoteNumberFromString(notenameremap_c_minorpentatonic[midinotenumber]);
		midinote = midinote+5;
		if(midinote>127) midinote=GetMidiNoteNumberFromString("F8");
		return midinote;
	}
	else if(strcmp(pstring, "FS_MINORPENTATONIC")==0)
	{
		midinote = GetMidiNoteNumberFromString(notenameremap_c_minorpentatonic[midinotenumber]);
		midinote = midinote+6;
		if(midinote>127) midinote=GetMidiNoteNumberFromString("F#8");
		return midinote;
	}
	else if(strcmp(pstring, "G_MINORPENTATONIC")==0)
	{
		midinote = GetMidiNoteNumberFromString(notenameremap_c_minorpentatonic[midinotenumber]);
		midinote = midinote+7;
		if(midinote>127) midinote=GetMidiNoteNumberFromString("G8");
		return midinote;
	}
	else if(strcmp(pstring, "GS_MINORPENTATONIC")==0)
	{
		midinote = GetMidiNoteNumberFromString(notenameremap_c_minorpentatonic[midinotenumber]);
		midinote = midinote+8;
		if(midinote>127) midinote=GetMidiNoteNumberFromString("G#8");
		return midinote;
	}
	else if(strcmp(pstring, "A_MINORPENTATONIC")==0)
	{
		midinote = GetMidiNoteNumberFromString(notenameremap_c_minorpentatonic[midinotenumber]);
		midinote = midinote+9;
		if(midinote>127) midinote=GetMidiNoteNumberFromString("A8");
		return midinote;
	}
	else if(strcmp(pstring, "AS_MINORPENTATONIC")==0)
	{
		midinote = GetMidiNoteNumberFromString(notenameremap_c_minorpentatonic[midinotenumber]);
		midinote = midinote+10;
		if(midinote>127) midinote=GetMidiNoteNumberFromString("A#8");
		return midinote;
	}
	else if(strcmp(pstring, "B_MINORPENTATONIC")==0)
	{
		midinote = GetMidiNoteNumberFromString(notenameremap_c_minorpentatonic[midinotenumber]);
		midinote = midinote+11;
		if(midinote>127) midinote=GetMidiNoteNumberFromString("B8");
		return midinote;
	}
	else if(strcmp(pstring, "C_MINORPENTATONIC_VERYLOW")==0)
	{
		return GetMidiNoteNumberFromString(notenameremap_c_minorpentatonic_verylow[midinotenumber]);
	}
	else if(strcmp(pstring, "C_MINORPENTATONIC_LOW")==0)
	{
		return GetMidiNoteNumberFromString(notenameremap_c_minorpentatonic_low[midinotenumber]);
	}
	else if(strcmp(pstring, "C_MINORPENTATONIC_MID")==0)
	{
		return GetMidiNoteNumberFromString(notenameremap_c_minorpentatonic_mid[midinotenumber]);
	}
	else if(strcmp(pstring, "C_MINORPENTATONIC_HIGH")==0)
	{
		return GetMidiNoteNumberFromString(notenameremap_c_minorpentatonic_high[midinotenumber]);
	}
	else if(strcmp(pstring, "C_MINORPENTATONIC_VERYHIGH")==0)
	{
		return GetMidiNoteNumberFromString(notenameremap_c_minorpentatonic_veryhigh[midinotenumber]);
	}
	/*
	else if(strcmp(pstring, "C3")==0)
	{
		return GetMidiNoteNumberFromString(notenameremap_c3[midinotenumber]);
	}
	else
	{
		assert(false);
		return midinotenumber;
	}
	*/
	else //pstring can be any note name, see note name table here above
	{
		return GetMidiNoteNumberFromString(pstring);
	}
}
//note remaping, end

int GetMidiNoteNumberFromString(const char* pstring)
{
	int notemidi = -1;
	bool octaveconfirmed = false;
	int octave = -1;
	int note = -1;
	bool sharp = false;
	string notename = "";
	string filename = pstring;
	//0) remove path
	int pos = filename.rfind("\\");
	if(pos!=string::npos)
	{
		filename = filename.substr(pos+1,filename.size());
	}
	//1) find octave, sharp and notename
	string::iterator it;
	//for(it=filename.end()-1; it>filename.begin(); it--)
	for(it=filename.end()-1; it>=filename.begin(); it--)
	//for(it=filename.end()-1; it>filename.begin(); it--)
	{
		//if char between 0 and 9 inclusively 
		//if( *it>47 && *it<58 && it>(filename.begin()+1) )
		if( *it>47 && *it<58 && it>filename.begin() ) 
		{
			//if previous char == #, == - or is between A and G inclusively
			if( *(it-1)==35 || *(it-1)==45 || (*(it-1)>64 && *(it-1)<72) || (*(it-1)>96 && *(it-1)<104) )
			{
				octaveconfirmed = true; //when char == # or -, we should also ensure that *(it-2) is a letter between A and G
				octave = atoi(&((char)*it));
			}
		}
		//else if between A and G inclusively
		else if( octaveconfirmed==true && ((*it>64 && *it<72) || (*it>96 && *it<104)) )
		{
			notename = toupper(*it);
			if( *(it+1)==35)
			{
				sharp = true;
				if(*it==69 || *it==66)
				{
					//for E and B, there is no semitone #
					assert(false);
				}
			}
			break;
		}
	}
	if(!notename.empty())
	{
		//2) now, find midi note number in very first octave (octave -1)
		int notemidioctaveminusone=-1;
		if(notename.compare("A")==0)
		{
			notemidioctaveminusone = 9;
		}
		else if(notename.compare("B")==0)
		{
			notemidioctaveminusone = 11;
		}
		else if(notename.compare("C")==0)
		{
			notemidioctaveminusone = 0;
		}
		else if(notename.compare("D")==0)
		{
			notemidioctaveminusone = 2;
		}
		else if(notename.compare("E")==0)
		{
			notemidioctaveminusone = 4;
		}
		else if(notename.compare("F")==0)
		{
			notemidioctaveminusone = 5;
		}
		else if(notename.compare("G")==0)
		{
			notemidioctaveminusone = 7;
		}
		if(sharp)
		{
			notemidioctaveminusone += 1;
		}
		assert(notemidioctaveminusone>-1 && notemidioctaveminusone<12);
		//3) now, find midi note number for proper octave
		notemidi = notemidioctaveminusone + 12*(octave+1);
		//2014july06, spi, begin
		//if(notemidi<0 || notemidi>127) assert(false);
		//2014july06, spi, end
	}
	return notemidi;
}

float  GetFrequencyFromMidiNoteNumber(int midinotenumber)
{
	assert(midinotenumber>=0 && midinotenumber<128);
	float frequency_hz;
	//frequency_hz = (440.0f/ 32.0f) * (2 ^ ((midinotenumber - 9.0f) / 12.0f));
	frequency_hz = (440.0f/ 32.0f) * pow(2.0f,((midinotenumber - 9.0f) / 12.0f));
	//frequency_hz = notefreq[midinotenumber];
	return frequency_hz;
}

const char*  GetNoteNameFromMidiNoteNumber(int midinotenumber)
{
	return notename[midinotenumber];
}

int GetMidiNoteNumberFromFrequency(float frequency_hz)
{
/*
float midi[127];
int a = 440; // a is 440 hz...
for (int x = 0; x < 127; ++x)
{
   midi[x] = (a / 32) * (2 ^ ((x - 9) / 12));
}
*/
	int midinotenumber = 9+12*log(32*frequency_hz/440.0f)/log(2.0f);
	return midinotenumber;
}

bool IsC(int midinotenumber)
{
	if(midinotenumber==0 || 
	   midinotenumber==12 || 
	   midinotenumber==24 || 
	   midinotenumber==36 || 
	   midinotenumber==48 || 
	   midinotenumber==60 || 
	   midinotenumber==72 || 
	   midinotenumber==84 || 
	   midinotenumber==96 || 
	   midinotenumber==108 || 
	   midinotenumber==120 
	   )
	{
		return true;
	}
	return false;
}

bool IsCs(int midinotenumber)
{
	if(midinotenumber==1 || 
	   midinotenumber==13 || 
	   midinotenumber==25 || 
	   midinotenumber==37 || 
	   midinotenumber==49 || 
	   midinotenumber==61 || 
	   midinotenumber==73 || 
	   midinotenumber==85 || 
	   midinotenumber==97 || 
	   midinotenumber==109 || 
	   midinotenumber==121 
	   )
	{
		return true;
	}
	return false;
}

bool IsD(int midinotenumber)
{
	if(midinotenumber==2 || 
	   midinotenumber==14 || 
	   midinotenumber==26 || 
	   midinotenumber==38 || 
	   midinotenumber==50 || 
	   midinotenumber==62 || 
	   midinotenumber==74 || 
	   midinotenumber==86 || 
	   midinotenumber==98 || 
	   midinotenumber==110 || 
	   midinotenumber==122 
	   )
	{
		return true;
	}
	return false;
}

bool IsDs(int midinotenumber)
{
	if(midinotenumber==3 || 
	   midinotenumber==15 || 
	   midinotenumber==27 || 
	   midinotenumber==39 || 
	   midinotenumber==51 || 
	   midinotenumber==63 || 
	   midinotenumber==75 || 
	   midinotenumber==87 || 
	   midinotenumber==99 || 
	   midinotenumber==111 || 
	   midinotenumber==123 
	   )
	{
		return true;
	}
	return false;
}

bool IsE(int midinotenumber)
{
	if(midinotenumber==4 || 
	   midinotenumber==16 || 
	   midinotenumber==28 || 
	   midinotenumber==40 || 
	   midinotenumber==52 || 
	   midinotenumber==64 || 
	   midinotenumber==76 || 
	   midinotenumber==88 || 
	   midinotenumber==100 || 
	   midinotenumber==112 || 
	   midinotenumber==124 
	   )
	{
		return true;
	}
	return false;
}

bool IsF(int midinotenumber)
{
	if(midinotenumber==5 || 
	   midinotenumber==17 || 
	   midinotenumber==29 || 
	   midinotenumber==41 || 
	   midinotenumber==53 || 
	   midinotenumber==65 || 
	   midinotenumber==77 || 
	   midinotenumber==89 || 
	   midinotenumber==101 || 
	   midinotenumber==113 || 
	   midinotenumber==125 
	   )
	{
		return true;
	}
	return false;
}

bool IsFs(int midinotenumber)
{
	if(midinotenumber==6 || 
	   midinotenumber==18 || 
	   midinotenumber==30 || 
	   midinotenumber==42 || 
	   midinotenumber==54 || 
	   midinotenumber==66 || 
	   midinotenumber==78 || 
	   midinotenumber==90 || 
	   midinotenumber==102 || 
	   midinotenumber==114 || 
	   midinotenumber==126 
	   )
	{
		return true;
	}
	return false;
}

bool IsG(int midinotenumber)
{
	if(midinotenumber==7 || 
	   midinotenumber==19 || 
	   midinotenumber==31 || 
	   midinotenumber==43 || 
	   midinotenumber==55 || 
	   midinotenumber==67 || 
	   midinotenumber==79 || 
	   midinotenumber==91 || 
	   midinotenumber==103 || 
	   midinotenumber==115 || 
	   midinotenumber==127 
	   )
	{
		return true;
	}
	return false;
}

bool IsGs(int midinotenumber)
{
	if(midinotenumber==8 || 
	   midinotenumber==20 || 
	   midinotenumber==32 || 
	   midinotenumber==44 || 
	   midinotenumber==56 || 
	   midinotenumber==68 || 
	   midinotenumber==80 || 
	   midinotenumber==92 || 
	   midinotenumber==104 || 
	   midinotenumber==116 || 
	   midinotenumber==128 
	   )
	{
		return true;
	}
	return false;
}

bool IsA(int midinotenumber)
{
	if(midinotenumber==9 || 
	   midinotenumber==21 || 
	   midinotenumber==33 || 
	   midinotenumber==45 || 
	   midinotenumber==57 || 
	   midinotenumber==69 || 
	   midinotenumber==81 || 
	   midinotenumber==93 || 
	   midinotenumber==105 || 
	   midinotenumber==117 || 
	   midinotenumber==129 
	   )
	{
		return true;
	}
	return false;
}

bool IsAs(int midinotenumber)
{
	if(midinotenumber==10 || 
	   midinotenumber==22 || 
	   midinotenumber==34 || 
	   midinotenumber==46 || 
	   midinotenumber==58 || 
	   midinotenumber==70 || 
	   midinotenumber==82 || 
	   midinotenumber==94 || 
	   midinotenumber==106 || 
	   midinotenumber==118 || 
	   midinotenumber==130 
	   )
	{
		return true;
	}
	return false;
}

bool IsB(int midinotenumber)
{
	if(midinotenumber==11 || 
	   midinotenumber==23 || 
	   midinotenumber==35 || 
	   midinotenumber==47 || 
	   midinotenumber==59 || 
	   midinotenumber==71 || 
	   midinotenumber==83 || 
	   midinotenumber==95 || 
	   midinotenumber==107 || 
	   midinotenumber==119 || 
	   midinotenumber==131 
	   )
	{
		return true;
	}
	return false;
}



