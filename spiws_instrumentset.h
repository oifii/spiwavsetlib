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

////////////////////////////////////////////////////////////////
//nakedsoftware.org, spi@oifii.org or stephane.poirier@oifii.org
//
//
//2012june11, creation for spisplitpatternplay
//nakedsoftware.org, spi@oifii.org or stephane.poirier@oifii.org
////////////////////////////////////////////////////////////////
#ifndef _INSTRUMENTSET_H
#define _INSTRUMENTSET_H

class SPIWAVSETLIB_API InstrumentSet
{
public:
	std::vector<class Instrument*> instrumentvector;
	InstrumentSet();
	~InstrumentSet();
	void Populate(const char* wavfilesfolder, int iflag_subfolders=1);
	void Populate(std::vector<string>* wavfilenames);
	std::string GetFilenamePattern(const char* filename);
	bool IsNewFilenamePattern(const char* prevfilename, const char* newfilename);
	bool HasOneInstrument();
	int GetNumberOfInstrument();
	class Instrument* GetInstrumentFromID(int idinstrument);
	class Instrument* GetInstrumentRandomly();
	class Instrument* GetInstrumentFromMidiTrackName(class Partition* pPartition);
	void Play(struct PaStreamParameters* pPaStreamParameters,  float numberofsecondsinplayback, int numberofinstrumentsinplayback,  int iCONCATENATEATTACKSflag=1);
	bool OpenAllStreams(PaStreamParameters* pPaStreamInputParameters, PaStreamParameters* pPaStreamOutputParameters, PaStreamCallback* pPaStreamCallback);
	bool CloseAllStreams();
};

#endif //_INSTRUMENTSET_H
