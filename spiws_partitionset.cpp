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

#include "stdafx.h"

//#define SPIWAVSETLIB_API
#include "spiwavsetlib.h"


#include <assert.h>
#include <string>
#include <fstream>
#include <vector>

#include <iostream>
#include <sstream>
using namespace std;

#include "portaudio.h"
#include "spiws_wavset.h"

#include "spiws_instrument.h"
#include "spiws_instrumentset.h"

#include "spiws_midievent.h"
#include "spiws_midieventset.h"
#include "spiws_partition.h"
#include "spiws_partitionset.h"

extern CHAR pCHAR[1024];
extern WCHAR pWCHAR[1024];

//#include <assert.h>

PartitionSet::PartitionSet()
{
	mfile_format = -1;
	mfile_ntrks = -1;
	mfile_division =-1;
	timesig_numerator = 4;
	timesig_denominator = 4; 
	timesig_midiclockspermetronomeclick = 96;
	timesig_numberofnotated32ndnotesperquaternote = 8;
	tempo_microsecondsperquaternote = 480000.0;
}
PartitionSet::~PartitionSet()
{
	vector<Partition*>::iterator it;
	for(it=partitionvector.begin(); it<partitionvector.end(); it++)
	{
		if(*it!=NULL) 
		{
			delete *it;
		}
		else
		{
			assert(false);
		}
	}
}

//one partitionset contains one or many partition(s)
//one partition contains one or many midieventset(s)
//one midieventset contains typically two midievent(s), i.e. note on and note off
bool PartitionSet::Populate(const char* midifile)
{
	string quote="\"";
	string inputfilename = midifile;
	string outputfilename = inputfilename + ".txt";

	//0) detect if midifile needs conversion into .mid.txt	
	if(IsStandardMidiFile(inputfilename.c_str()))
	{
		//1) convert to midifiletxt
		string systemcommand = "mf2txp.exe " + quote + inputfilename + quote + " " + quote + outputfilename + quote;
		sprintf(pCHAR, "%s\n", systemcommand);StatusAddText(pCHAR);
		system(systemcommand.c_str()); 
	}
	else
	{
		assert(IsStandardMidiFileTxt(inputfilename.c_str()));
	}

	//2) read midifiletxt lines
	vector<string> midifiletxtlines;
	ifstream ifs(outputfilename.c_str());
	string temp;
	while(getline(ifs,temp))
		midifiletxtlines.push_back(temp);

	/////////////////////////////
	//3) Populate
	/////////////////////////////
	Partition* pCurrentPartition = NULL;
	//3.1) first pass, read midi file and identify the number of instrument track 
	string miditrackheader = "MTrk";
	string miditrackfooter = "TrkEnd";
	int nline=0;
	int nmiditrack=0;
	int ninstrumenttrack=0;
	bool bIsInsideMidiTrack = false;
	bool bIsInsideInstrumentTrack = false;
	vector<string>::iterator iterator;
	for(iterator=midifiletxtlines.begin(); iterator<midifiletxtlines.end(); iterator++)
	{
		nline++; //line counter
		//process midi file header
		if(nline==1)
		{
			string midifileheader = *iterator; //i.e. MFile 1 11 96
			size_t found = midifileheader.find("MFile ");
			if (found!=string::npos)
			{
				if(found!=0)
				{
					return false;
				}
			}
			//Mfile <format> <ntrks> <division>
			string midifileheadernumbers=midifileheader.substr(6,midifileheader.size());
			sscanf(midifileheadernumbers.c_str(), "%d %d %d", &mfile_format, &mfile_ntrks, &mfile_division);
			continue;
		}
		//look for midi track's header and footer
		if(*iterator == miditrackheader)
		{
			//start of midi track detected
			bIsInsideMidiTrack = true;
			//create partition
			pCurrentPartition = new Partition; 
			continue;
		}
		if(*iterator == miditrackfooter && bIsInsideMidiTrack)
		{
			//end of midi track detected
			nmiditrack++; //midi track counter
			if(bIsInsideInstrumentTrack)
			{
				//end of instrument track detected
				ninstrumenttrack++;
				//attach partition to partitionset
				partitionvector.push_back(pCurrentPartition);
			}
			else
			{
				assert(pCurrentPartition);
				delete pCurrentPartition;
			}
			bIsInsideMidiTrack = false;
			bIsInsideInstrumentTrack = false;
			continue;
		}
		if(IsTimeSig(*iterator) && nmiditrack==0)
		{
			//global timesig detected
			size_t found = (*iterator).find('/');
			if (found!=string::npos)
			{
				timesig_numerator = atoi(((*iterator).substr(9,found)).c_str());
				string mystring = (*iterator).substr(found+1,(*iterator).size());
				sscanf(mystring.c_str(), "%d %d %d", &timesig_denominator, &timesig_midiclockspermetronomeclick, &timesig_numberofnotated32ndnotesperquaternote);
			}
			else
			{
				assert(false);
			}
			continue;
		}
		if(IsTempo(*iterator) && nmiditrack==0)
		{
			//global tempo detected
			size_t found = (*iterator).find("0 Tempo");
			if (found!=string::npos)
			{
				tempo_microsecondsperquaternote = atof(((*iterator).substr(7,(*iterator).size())).c_str());
			}
			else
			{
				assert(false);
			}
			continue;
		}
		if(IsMetaTrkName(*iterator))
		{
			assert(bIsInsideMidiTrack);
			string tag = "0 Meta TrkName ";
			pCurrentPartition->miditrackname = (*iterator).substr(tag.size()+1,(*iterator).size()-(tag.size()+1)-1); //kick out the encapsulating quotes
		}
		if(HasChannelNumber(*iterator) && HasNonZeroTimeStamp(*iterator))
		{
			//time stamped midi event detected
			bIsInsideInstrumentTrack = true;
			if(IsNoteOn(*iterator))
			{
				//create noteon midievent
				MidiEvent* pMidiEventNoteOn = new MidiEvent;
				if(pMidiEventNoteOn) pMidiEventNoteOn->CreateNoteOn(*iterator);
				//store midievent to temporary vector
				StoreNoteOn(pMidiEventNoteOn);
				//
				//todo
			}
			else if(IsNoteOff(*iterator))
			{
				//create noteoff midievent
				//todo
				MidiEvent* pMidiEventNoteOff = new MidiEvent;
				if(pMidiEventNoteOff) pMidiEventNoteOff->CreateNoteOff(*iterator);
				//retreive midievent (matching this noteoff midievent to its corresponding noteon midievent)
				MidiEvent* pMidiEventNoteOn = RetreiveNoteOn(pMidiEventNoteOff);
				if(pMidiEventNoteOn!=NULL)
				{
					//create midieventset (pair of midi event)
					MidiEventSet* pMidiEventSet = new MidiEventSet(pMidiEventNoteOn,pMidiEventNoteOff);
					//attach midieventset to current partition
					pCurrentPartition->midieventsetvector.push_back(pMidiEventSet);
				}
				else
				{
					//keep going safely, todo, pMidiEventNoteOn==NULL, no matching note on was found
					continue;
				}
			}
			else
			{
				//currently not supporting other midi event than noteon and note off
			}
		}		
	}

	return true;
}

//simple validation based on file extension only
bool PartitionSet::IsStandardMidiFile(const char* filename)
{
	string fn=filename;
	size_t found = fn.rfind(".mid");
	if (found!=string::npos)
	{
		if(found+4!=fn.length())
		{
			return false;
		}
	}
	return true;
}

bool PartitionSet::IsStandardMidiFileTxt(const char* filename)
{
	string fn=filename;
	size_t found = fn.rfind(".mid.txt");
	if (found!=string::npos)
	{
		if(found+8!=fn.length())
		{
			return false;
		}
	}
	return true;
}

bool PartitionSet::HasChannelNumber(string midifileline)
{
	size_t found = midifileline.find("ch=");
	if (found!=string::npos)
	{
		return true;
	}
	return false;
}

bool PartitionSet::HasNonZeroTimeStamp(string midifileline)
{
	string firstchar = midifileline.substr(0,1);
	if (firstchar != "0")
	{
		return true;
	}
	return false;
}

bool PartitionSet::IsTimeSig(string midifileline)
{
	size_t found = midifileline.find("0 TimeSig");
	if (found==0)
	{
		return true;
	}
	return false;
}

bool PartitionSet::IsTempo(string midifileline)
{
	size_t found = midifileline.find("0 Tempo");
	if (found==0)
	{
		return true;
	}
	return false;
}

bool PartitionSet::IsMetaTrkName(string midifileline)
{
	size_t found = midifileline.find("0 Meta TrkName");
	if (found==0)
	{
		return true;
	}
	return false;
}

//returns true if Note On with non-zero velocity
bool PartitionSet::IsNoteOn(string midifileline)
{
	size_t found = midifileline.find("On ch=");
	if (found!=string::npos)
	{
		//ok, possible Note On with a non-zero velocity 
		found = midifileline.find(" v=0");
		if (found!=string::npos)
		{
			//NoteOn with zero velocity
			return false;
		}
		//ok, definitely Note On
		return true;
	}
	return false;
}

//returns true if Note Off or Note On with zero velocity
bool PartitionSet::IsNoteOff(string midifileline)
{
	size_t found = midifileline.find("Off ch=");
	if (found!=string::npos)
	{
		//ok definitely Note Off
		return true;
	}
	found = midifileline.find(" On"); 
	if (found!=string::npos)
	{
		found = midifileline.find(" v=0");
		if (found!=string::npos)
		{
			//ok, Note On with zero velocity - it has the effect of a Note Off
			return true;
		}
	}
	return false;
}

MidiEvent* PartitionSet::CreateMidiEventNoteOn(string midifileline)
{
	MidiEvent* pMidiEvent = new MidiEvent;
	if(pMidiEvent) pMidiEvent->CreateNoteOn(midifileline);
	return pMidiEvent;
}

MidiEvent* PartitionSet::CreateMidiEventNoteOff(string midifileline)
{
	MidiEvent* pMidiEvent = new MidiEvent;
	if(pMidiEvent) pMidiEvent->CreateNoteOff(midifileline);
	return pMidiEvent;
}

void PartitionSet::StoreNoteOn(MidiEvent* pMidiEventNoteOn)
{
	tempmidieventvector.push_back(pMidiEventNoteOn);
	return;
}

MidiEvent* PartitionSet::RetreiveNoteOn(MidiEvent* pMidiEventNoteOff)
{
	bool found=false;
	MidiEvent* pMidiEvent = NULL;
	vector<MidiEvent*>::iterator it;
	for(it=tempmidieventvector.begin(); it<tempmidieventvector.end(); it++)
	{
		if( ((*it)->iChannel == pMidiEventNoteOff->iChannel) &&
			((*it)->iValue1 == pMidiEventNoteOff->iValue1) )
			//match found
			found=true;
			pMidiEvent = *it;
			tempmidieventvector.erase(it);
			break;
	}
	return pMidiEvent;
}

//when the wavset audio buffer has to be created, i.e. prior to playing it
bool PartitionSet::CreateWavSetForPartition(Partition* pPartition, Instrument* pInstrument, float numberofsecondsinplayback)
{
	assert(pPartition!=NULL);
	assert(pInstrument!=NULL);
	if(pPartition->pWavSet!=NULL)
	{
		delete pPartition->pWavSet;
		pPartition->pWavSet = NULL;
	}

	//1) create silence wavset of the proper length
	float duration = GetLengthInSeconds(pPartition);
	if(duration>numberofsecondsinplayback) duration=numberofsecondsinplayback;
	pPartition->pWavSet = new WavSet();
	int ii=0;
	if(pPartition->pWavSet && pPartition->pWavSet->CreateSilence(duration)) //default samplerate=44100 and numchannel=2
	{
		ii++;
		/*
		if(ii==1)
		{
			//2) spread n samples
			WavSet* pTempWavSet = new WavSet;
			pTempWavSet->CreateSin(0.5);
			pPartition->pWavSet->SpreadSample(10, pTempWavSet,100); //default duration and distance
			delete pTempWavSet;
		}
		*/
		vector<MidiEventSet*>::iterator iter;
		for(iter=pPartition->midieventsetvector.begin(); iter<pPartition->midieventsetvector.end(); iter++)
		{
			if(*iter!=NULL) 
			{
				//2) get proper wavset from instrument 
				MidiEventSet* pMidiEventSet = *iter;
				WavSet* pWavSet = pInstrument->GetWavSetFromMidiNoteNumber(pMidiEventSet);
				//WavSet* pWavSet = pInstrument->GetWavSetRandomly();
				if(pWavSet!=NULL)
				{
					//3) sum each note onto former silence wavset
					float offsetinseconds = GetStartTimeStampInSeconds(pMidiEventSet);
					float durationinseconds = GetLengthInSeconds(pMidiEventSet);
					if(offsetinseconds>numberofsecondsinplayback) break;
					//pPartition->pWavSet->Sum(1.0f, pWavSet, offsetinseconds, durationinseconds);
					//pPartition->pWavSet->Sum(0.5f, pWavSet, offsetinseconds, durationinseconds);
					pPartition->pWavSet->Sum(0.25f, pWavSet, offsetinseconds, durationinseconds);
				}
			}
		}
		//4) set partition name based on both miditrackname and instrumentname
		pPartition->partitionname = pPartition->miditrackname + pInstrument->GetInstrumentNameWithoutPath();
		return true;
	}
	return false;
}

void PartitionSet::Play(PaStreamParameters* pPaStreamParameters, InstrumentSet* pInstrumentSet, float numberofsecondsinplayback)
{
	/////////////////////////////////////////////
	//now, assign an instrument to each partition 
	/////////////////////////////////////////////
	swprintf(pWCHAR, L"number of instruments in this instrumentset = %d\n", pInstrumentSet->instrumentvector.size());StatusAddText(pWCHAR);
	//browse through each partition
	vector<Partition*>::iterator it;
	int i=0;
	for(it=partitionvector.begin(); it<partitionvector.end(); it++)
	{
		i++; //spi
		if(*it!=NULL) 
		{
			Partition* pPartition=*it;
			if(1)
			{
				//play each partition (simultaneously) (with instruments randomly assigned)
				swprintf(pWCHAR, L"\nnumber of midieventset in this partition = %d\n", pPartition->midieventsetvector.size());StatusAddText(pWCHAR);
				sprintf(pCHAR, "miditrackname for this partition = %s\n", pPartition->miditrackname);StatusAddText(pCHAR);
				swprintf(pWCHAR, L"length of this partition = %f\n", GetLengthInSeconds(pPartition));StatusAddText(pWCHAR);
				//1) create wavset
				Instrument* pInstrument = pInstrumentSet->GetInstrumentRandomly();
				CreateWavSetForPartition(pPartition, pInstrument, numberofsecondsinplayback);
				/*
				//tested OK, beeps well deployed in time
				if(i==1)
				{
					//2) spread n samples
					WavSet* pTempWavSet = new WavSet;
					pTempWavSet->CreateSin(0.5);
					pPartition->pWavSet->SpreadSample(10, pTempWavSet,100); //default duration and distance
					delete pTempWavSet;
					
				}
				*/
				if(1)
				{
					pPartition->LaunchScheduledPlay(USING_SOX);
				}
				else
				{
					pPartition->LaunchScheduledPlay(USING_SPIPLAY);
				}
			}
			else
			{
				//play each partition individually once with each instrument (for debugging purpose)
				swprintf(pWCHAR, L"\nnumber of midieventset in this partition = %d\n", pPartition->midieventsetvector.size());StatusAddText(pWCHAR);
				sprintf(pCHAR, "miditrackname for this partition = %s\n", pPartition->miditrackname);StatusAddText(pCHAR);
				swprintf(pWCHAR, L"length of this partition = %f\n", GetLengthInSeconds(pPartition));StatusAddText(pWCHAR);
				swprintf(pWCHAR, L"\nnumber of instrument in this instrumentset = %d\n", pInstrumentSet->instrumentvector.size());StatusAddText(pWCHAR);
				swprintf(pWCHAR, L"will now play each partition once with each instrument = \n");StatusAddText(pWCHAR);
				vector<Instrument*>::iterator iter;
				for(iter=pInstrumentSet->instrumentvector.begin(); iter<pInstrumentSet->instrumentvector.end(); iter++)
				{
					if(*iter!=NULL) 
					{
						//1) create wavset
						Instrument* pInstrument = *iter;
						CreateWavSetForPartition(pPartition, pInstrument, numberofsecondsinplayback);
						if(1)
						{
							//2) spread n samples
							WavSet* pTempWavSet = new WavSet;
							pTempWavSet->CreateSin(0.5);
							pPartition->pWavSet->SpreadSample(10, pTempWavSet,100); //default duration and distance
							delete pTempWavSet;
						}
						if(1)
						{
							////////////////////////////////////////////////////
							// play each partition wavset as is using port audio 
							////////////////////////////////////////////////////
							//pPartition->pWavSet->Play(pPaStreamParameters); //(&outputParameters);
							pPartition->Play(pPaStreamParameters);
						}
					}
					else
					{
						assert(false);
					}
				}
			}
		}
	}

}

float PartitionSet::GetTempoInBPM()
{
	//BPM = beat per minute = quater note per minute
	float bpm = 60000000.0f/tempo_microsecondsperquaternote;
	return bpm;
}

float PartitionSet::GetLengthInSeconds()
{
	vector<Partition*>::iterator it;
	float maxseconds = -1.0f;
	float tempseconds = 0.0f;
	for (it=partitionvector.begin(); it<partitionvector.end(); it++)
	{
		GetLengthInSeconds(*it);
		if(tempseconds>maxseconds) maxseconds=tempseconds;
	}
	return maxseconds;
}

float PartitionSet::ConvertMidiClockTicksToSeconds(int midiclockticks)
{
	
	float seconds = 0.0f;
	float ppq = timesig_midiclockspermetronomeclick; //ticks per quater note = pulse per quater note = ppq = ppqn
	float ticksperminute = GetTempoInBPM()*ppq;
	float tickspermilisecond = ticksperminute/60000.0f;
	float tempmilisecond = midiclockticks/tickspermilisecond;
	seconds = tempmilisecond/1000;
	
	/*
	float seconds = 0.0f;
	float ppq = timesig_midiclockspermetronomeclick*timesig_numberofnotated32ndnotesperquaternote; //ticks per quater note = pulse per quater note = ppq = ppqn
	float ticksperminute = GetTempoInBPM()*ppq;
	float tickspermilisecond = ticksperminute/60000.0f;
	float tempmilisecond = midiclockticks/tickspermilisecond;
	seconds = tempmilisecond/1000;
	*/
	return seconds;
}

float PartitionSet::GetLengthInSeconds(Partition* pPartition)
{
	float tempseconds = 0.0f;
	if(pPartition==NULL)
	{
		assert(false);
	}
	else
	{
		tempseconds = ConvertMidiClockTicksToSeconds(pPartition->GetLengthInMidiClockTicks());
	}
	return tempseconds;
}

float PartitionSet::GetLengthInSeconds(MidiEventSet* pMidiEventSet)
{
	float tempseconds = 0.0f;
	if(pMidiEventSet==NULL)
	{
		assert(false);
	}
	else
	{
		tempseconds = ConvertMidiClockTicksToSeconds(pMidiEventSet->GetLengthInMidiClockTicks());
		//2012june14, spi, begin
		//for debugging
		if(tempseconds< 0.5) 
		{
			tempseconds=0.5;
		}
		//2012june14, spi, end
	}
	return tempseconds;
}

float PartitionSet::GetStartTimeStampInSeconds(class MidiEventSet* pMidiEventSet)
{
	float tempseconds = 0.0f;
	if(pMidiEventSet==NULL)
	{
		assert(false);
	}
	else
	{
		tempseconds = ConvertMidiClockTicksToSeconds(pMidiEventSet->GetStartTimeStampInMidiClockTicks());
	}
	return tempseconds;
}

float PartitionSet::GetEndTimeStampInSeconds(class MidiEventSet* pMidiEventSet)
{
	float tempseconds = 0.0f;
	if(pMidiEventSet==NULL)
	{
		assert(false);
	}
	else
	{
		tempseconds = ConvertMidiClockTicksToSeconds(pMidiEventSet->GetEndTimeStampInMidiClockTicks());
	}
	return tempseconds;
}

bool PartitionSet::HasOnePartition()
{
	if(partitionvector.empty()) return false;
	return true;
}
