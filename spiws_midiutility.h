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
#ifndef _MIDIUTILITY_H
#define _MIDIUTILITY_H

//midi note range from 0 to 127, from C -1 to G9
extern const char* notename[132]; //we use 132 to complete 9th octave
extern const float notefreq[132]; //we use 132 to complete 9th octave

//note remaping, begin
extern const char* notenameremap_c_minorpentatonic[132]; //we use 132 to complete 9th octave
int SPIWAVSETLIB_API GetNoteRemap_C_MinorPentatonic(int midinotenumber);
int SPIWAVSETLIB_API GetNoteRemap_C3(int midinotenumber);
int SPIWAVSETLIB_API GetNoteRemap(const char* pstring, int midinotenumber);
//note remaping, end

int  SPIWAVSETLIB_API GetMidiNoteNumberFromString(const char* pstring);
float SPIWAVSETLIB_API GetFrequencyFromMidiNoteNumber(int midinotenumber);
SPIWAVSETLIB_API const char* GetNoteNameFromMidiNoteNumber(int midinotenumber);
int SPIWAVSETLIB_API GetMidiNoteNumberFromFrequency(float frequency_hz);

bool SPIWAVSETLIB_API IsC(int midinotenumber);
bool SPIWAVSETLIB_API IsCs(int midinotenumber);
bool SPIWAVSETLIB_API IsD(int midinotenumber);
bool SPIWAVSETLIB_API IsDs(int midinotenumber);
bool SPIWAVSETLIB_API IsE(int midinotenumber);
bool SPIWAVSETLIB_API IsF(int midinotenumber);
bool SPIWAVSETLIB_API IsFs(int midinotenumber);
bool SPIWAVSETLIB_API IsG(int midinotenumber);
bool SPIWAVSETLIB_API IsGs(int midinotenumber);
bool SPIWAVSETLIB_API IsA(int midinotenumber);
bool SPIWAVSETLIB_API IsAs(int midinotenumber);
bool SPIWAVSETLIB_API IsB(int midinotenumber);

int SPIWAVSETLIB_API GetOctave(int midinotenumber);
SPIWAVSETLIB_API const char* GetNote(int midinotenumber);

//2020sept28, spi, begin
SPIWAVSETLIB_API string GetNoteNamesForOctave(int octavenumber=-2); //-2 for all octaves, otherwise an integer between -1 and 9
SPIWAVSETLIB_API string GetNoteNamesForNote(string note, int octavestart=-1, int octaveend=9); //possible input note string are "C", "C#", "CS", "Cs","D", "D#", "DS", "Ds", etc.
																								//i.e. for "C" as input, it will return "C-1,C0,C1,C2,C3,C4,C5,C6,C7,C8,C9"
bool SPIWAVSETLIB_API IsNoteName(string mystring);
bool SPIWAVSETLIB_API IsNoteNoOctave(string mystring);
int SPIWAVSETLIB_API GetNoteOffsetInFirstOctave(string mystring);
SPIWAVSETLIB_API string GetNoteNoOctave(int noteoffset_infirstoctave);
//2020sept28, spi, end

#endif //_MIDIUTILITY_H