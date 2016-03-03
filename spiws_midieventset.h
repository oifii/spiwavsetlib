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
//2014march17, header fix
//
//nakedsoftware.org, spi@oifii.org or stephane.poirier@oifii.org
////////////////////////////////////////////////////////////////
#ifndef _MIDIEVENTSET_H
#define _MIDIEVENTSET_H

class SPIWAVSETLIB_API MidiEventSet
{
public:
	std::vector<class MidiEvent*> midieventvector;
	MidiEventSet(class MidiEvent* pMidiEventNoteOn, class MidiEvent* pMidiEventNoteOff);
	~MidiEventSet();
	float GetLengthInMidiClockTicks();
	float GetStartTimeStampInMidiClockTicks();
	float GetEndTimeStampInMidiClockTicks();
	int GetNoteNumber();
};


#endif //_MIDIEVENTSET_H