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
//2012june13, creation for spisplitpatternplay
//
//
//nakedsoftware.org, spi@oifii.org or stephane.poirier@oifii.org
////////////////////////////////////////////////////////////////
#ifndef _PARTITION_H
#define _PARTITION_H

#include <vector>

#define USING_SOX		1
#define USING_SPIPLAY	2

class SPIWAVSETLIB_API Partition
{
public:
	std::string miditrackname;
	std::string partitionname;
	std::vector<class MidiEventSet*> midieventsetvector;
	class WavSet* pWavSet;
	Partition();
	~Partition();
	float GetLengthInMidiClockTicks();
	bool CreateWavSet(class Instrument* pInstrument);
	//void Play(struct PaStreamParameters* pPaStreamParameters, class Instrument* pInstrument, float numberofsecondsinplayback);
	void Play(struct PaStreamParameters* pPaStreamParameters);
	void LaunchScheduledPlay(int usingflag);
};

#endif //_PARTITION_H