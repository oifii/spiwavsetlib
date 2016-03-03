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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//nakedsoftware.org, spi@oifii.org or stephane.poirier@oifii.org
//
//
//2012june11, creation for spisplitpatternplay
//2012june21, many CreateFrom*() function added to ease instrument creation from string, filename or foldername
//            for the release of the application spimidisampler
//
//nakedsoftware.org, spi@oifii.org or stephane.poirier@oifii.org
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _INSTRUMENT_H
#define _INSTRUMENT_H

#include <vector>

//#define INSTRUMENT_MAXNUMBEROFWAVSET			20
#define INSTRUMENT_MAXNUMBEROFWAVSET			20
#define INSTRUMENT_MAXWAVSETLENGTH				5.0

#define INSTRUMENT_WAVSETINSEQUENCE				1
#define INSTRUMENT_WAVSETALLATONCE				2
#define INSTRUMENT_WAVSETRANDOM					3

#define INSTRUMENT_PATTERNCODETOALL				0
#define INSTRUMENT_PATTERNCODETOEACHMIDINOTE	1

#define INSTRUMENT_SYNTH_SINWAV			0
#define INSTRUMENT_SYNTH_SQUAREWAV		1
#define INSTRUMENT_SYNTH_SAWWAV			2
#define INSTRUMENT_SYNTH_TRIWAV			3

#define INSTRUMENT_TEMPFOLDER			0


class SPIWAVSETLIB_API Instrument
{
public:
	std::vector<class WavSet*> wavsetvector;
	std::string instrumentname;
	bool sortedonmidinote;
	bool patterncodeassigned;
	Instrument();
	~Instrument();
	bool SetInstrumentName(std::string filenamepattern);
	const char* GetInstrumentName();
	const char* GetInstrumentNameWithoutPath();
	void SetWavSetPatternCodes(int flag);
	const char* GetWavSetPatternCodes();
	const char* GetWavSetPatternNotes();
	int GetNumberOfWavSet();
	class WavSet* GetWavSetFromID(int idwavset);
	class WavSet* GetWavSetRandomly();
	class WavSet* GetWavSetFromFrequency(float frequency_hz);
	class WavSet* GetWavSetFromMidiNoteName(const char* midinotename);
	class WavSet* GetWavSetFromMidiNoteNumber(int midinotenumber);
	class WavSet* GetWavSetFromMidiNoteNumber(class MidiEventSet* pMidiEventSet);
	class WavSet* GetWavSetFromPatternCode(const char* patterncode);
	class WavSet* GetWavSetFromPatternCode(const char* patterncode, int patternrange);
	const char* GetWavFolderFromWavFoldersFilename(const char* wavfoldersfilename);
	bool IsValidWavFolder(const char* wavfoldername);
	int GetMidiNoteNumberFromMidiNoteName(const char* midinotename);
	int GetMidiNoteNumberFromWavFilename(const char* wavfilename);
	int GetMidiNoteNumberFromTxtFilename(const char* wavfilename);
	bool CreateWavSynth(int flag=INSTRUMENT_SYNTH_SINWAV);
	bool WriteWavFiles(int flag=INSTRUMENT_TEMPFOLDER);
	bool CreateFromWavFilenamesFile(const char* wavfilenamesfile, int maxnumberofwavset=INSTRUMENT_MAXNUMBEROFWAVSET);
	bool CreateFromWavFolder(const char* wavfolder,  int maxnumberofwavset=INSTRUMENT_MAXNUMBEROFWAVSET);
	bool CreateFromWavFoldersFilename(const char* wavfoldersfilename,  int maxnumberofwavset=INSTRUMENT_MAXNUMBEROFWAVSET);
	bool CreateFromWavFilenamesFilter(const char* wavfilenamesfilter=NULL,  int maxnumberofwavset=INSTRUMENT_MAXNUMBEROFWAVSET);
	bool CreateFromRootWavFoldersFilename(const char* rootwavfoldersfilename,  int maxnumberofwavset=INSTRUMENT_MAXNUMBEROFWAVSET);
	bool CreateFromName(const char* name,   int maxnumberofwavset=INSTRUMENT_MAXNUMBEROFWAVSET);
	void Play(struct PaStreamParameters* pPaStreamOutputParameters, int iflag=INSTRUMENT_WAVSETINSEQUENCE);
	bool SplitWavSetsInSegments(double fSecondsPerSegment);
	void DisplayMidiStats();
	bool OpenAllStreams(PaStreamParameters* pPaStreamInputParameters, PaStreamParameters* pPaStreamOutputParameters, PaStreamCallback* pPaStreamCallback);
	bool CloseAllStreams();
};


#endif //_INSTRUMENT_H
