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
#ifndef _PARTITIONSET_H
#define _PARTITIONSET_H

class SPIWAVSETLIB_API PartitionSet
{
public:
	int mfile_format;
	int mfile_ntrks;
	int mfile_division;
	int timesig_numerator;
	int timesig_denominator;
	int timesig_midiclockspermetronomeclick;
	int timesig_numberofnotated32ndnotesperquaternote;
	float tempo_microsecondsperquaternote;
	std::vector<class Partition*> partitionvector;
	std::vector<class MidiEvent*> tempmidieventvector;
	PartitionSet();
	~PartitionSet();
	bool Populate(const char* midifile);
	bool IsStandardMidiFile(const char* filename);
	bool IsStandardMidiFileTxt(const char* filename);
	bool HasChannelNumber(string midifileline);
	bool HasNonZeroTimeStamp(string midifileline);
	bool IsTimeSig(string midifileline);
	bool IsTempo(string midifileline);
	bool IsMetaTrkName(string midifileline);
	bool IsNoteOn(string midifileline);
	bool IsNoteOff(string midifileline);
	class MidiEvent* CreateMidiEventNoteOn(string midifileline);
	class MidiEvent* CreateMidiEventNoteOff(string midifileline);
	void StoreNoteOn(class MidiEvent* pMidiEventNoteOn);
	class MidiEvent* RetreiveNoteOn(class MidiEvent* pMidiEventNoteOff);
	bool CreateWavSetForPartition(class Partition* pPartition, class Instrument* pInstrument, float numberofsecondsinplayback);
	void Play(struct PaStreamParameters* pPaStreamParameters, class InstrumentSet* pInstrumentSet, float numberofsecondsinplayback);
	float GetTempoInBPM();
	float GetLengthInSeconds();
	float ConvertMidiClockTicksToSeconds(int midiclockticks);
	float GetLengthInSeconds(class Partition* pPartition);
	float GetLengthInSeconds(class MidiEventSet* pMidiEventSet);
	float GetStartTimeStampInSeconds(class MidiEventSet* pMidiEventSet);
	float GetEndTimeStampInSeconds(class MidiEventSet* pMidiEventSet);
	bool HasOnePartition();
};


#endif //_PARTITIONSET_H