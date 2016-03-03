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

#include "spiws_partition.h"

#include <windows.h> //for Sleep()

extern CHAR pCHAR[1024];
extern WCHAR pWCHAR[1024];

InstrumentSet::InstrumentSet()
{
}
InstrumentSet::~InstrumentSet()
{
	vector<Instrument*>::iterator it;
	for(it=instrumentvector.begin(); it<instrumentvector.end(); it++)
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

void InstrumentSet::Populate(const char* wavfilesfolder,  int iflag_subfolders)
{
	/////////////////////////////////////////////////////
	//execute cmd line to get all folder's .wav filenames
	/////////////////////////////////////////////////////
	string quote = "\"";
	string pathfilter;
	string path=wavfilesfolder;
	pathfilter = path + "\\*.wav";
	string systemcommand;
	if(iflag_subfolders!=0)
	{
		systemcommand = "DIR " + quote + pathfilter + quote + "/S /B /O:N > wsip_filenames.txt"; //wsip tag standing for wav set (library) instrumentset (class) populate (function)
	}
	else
	{
		systemcommand = "DIR " + quote + pathfilter + quote + "/B /O:N > wsip_filenames.txt"; //wsip tag standing for wav set (library) instrumentset (class) populate (function)
	}

#ifdef _DEBUG
	sprintf(pCHAR, "%s\n", systemcommand);StatusAddText(pCHAR);
#endif //_DEBUG
	system(systemcommand.c_str());

	//////////////////////////////////////////////////////////////
	//load .wav filenames from wavfilenamesfile into string vector
	//////////////////////////////////////////////////////////////
	vector<string> filenames;
	vector<string>::iterator it;
	ifstream ifs("wsip_filenames.txt");
	string temp;
	while(getline(ifs,temp))
	{
		if(iflag_subfolders!=0)
		{
			filenames.push_back(temp); // the /S option added here above
		}
		else
		{
			filenames.push_back(path + "\\" + temp);
		}
	}
	Populate(&filenames);
}

//one instrumentset contains one or many instrument(s)
//one instrument contains one or many wavset(s)
//one wavset contains only one wavfile's content
void InstrumentSet::Populate(vector<string>* pwavfilenames)
{
	vector<string>::iterator it;
	int i=0;
	string prevfilename = "";
	Instrument* previnstrument = NULL;
	for(it=pwavfilenames->begin(); it<pwavfilenames->end(); it++)
	{
		//////////////////////////
		//read all valid WAV files 
		//////////////////////////
		//assert(false); //todo
		
		WavSet* pWavSet = new WavSet;
		pWavSet->ReadWavFile((*it).c_str()); //*it is a .wav filename
		/*
		if(pWavSet->numChannels!=2)
		{
			printf("exluding %s because sample is mono\n",(*it).c_str());
			delete pWavSet;
			continue;
		}
		*/
		if( (pWavSet->numChannels==2 && pWavSet->SampleRate!=44100) 
			|| (pWavSet->numChannels==1 && pWavSet->SampleRate!=22050) )
		{
			sprintf(pCHAR, "exluding %s because sample is %d Hz\n",(*it).c_str(),pWavSet->SampleRate);StatusAddText(pCHAR);
			delete pWavSet;
			continue;
		}
		if(i==0 || IsNewFilenamePattern(prevfilename.c_str(),(*it).c_str())==true)//(prevfilename!=((*it).c_str())))
		{
			Instrument* pInstrument=new Instrument;
			if(pInstrument)
			{
				pInstrument->SetInstrumentName(GetFilenamePattern((*it).c_str()));
				instrumentvector.push_back(pInstrument);
				pInstrument->wavsetvector.push_back(pWavSet);
				i++;
			}
			else
			{
				assert(false);
			}
			prevfilename = (*it).c_str();
			previnstrument = pInstrument;
		}
		else
		{
			if(previnstrument)
			{
				previnstrument->wavsetvector.push_back(pWavSet);
				i++;
			}
			else
			{
				assert(false);
			}
			prevfilename = (*it).c_str();
		}
	}

}

string InstrumentSet::GetFilenamePattern(const char* filename)
{
	string fn(filename);
	//0) remove path from input string
	size_t found = fn.rfind('\\');
	if (found!=string::npos)
	{
		fn = fn.substr(found+1,fn.length());
	}
	//0) remove extension from input string
	found = fn.rfind('.');
	if (found!=string::npos)
	{
		fn = fn.substr(0,found);
	}
	string fnpattern="";
	//1) find last underscore (or space) in input string
	found = fn.rfind('_');
	if (found!=string::npos)
	{
		fnpattern = fn.substr(0,found);
	}
	else
	{
		found = fn.rfind(' ');
		if (found!=string::npos)
		{
			fnpattern = fn.substr(0,found);
		}
		else
		{
			fnpattern = fn;
		}
	}
	return fnpattern;
}

//only compare left of last underscore
bool InstrumentSet::IsNewFilenamePattern(const char* prevfilename, const char* newfilename)
{
	/*
	//assert(false); //todo: implement
	string prevfn(prevfilename);
	string newfn(newfilename);
	//0) remove path from both input string
	size_t found = prevfn.rfind('\\');
	if (found!=string::npos)
	{
		prevfn = prevfn.substr(found+1,prevfn.length());
	}
	found = newfn.rfind('\\');
	if (found!=string::npos)
	{
		newfn = newfn.substr(found+1,newfn.length());
	}
	//0) remove extension from both input string
	found = prevfn.rfind('.');
	if (found!=string::npos)
	{
		prevfn = prevfn.substr(0,found);
	}
	found = newfn.rfind('.');
	if (found!=string::npos)
	{
		newfn = newfn.substr(0,found);
	}

	string prevfnpattern="";
	string newfnpattern="";
	//1) find last underscore (or space) in both input string
	found = prevfn.rfind('_');
	if (found!=string::npos)
	{
		prevfnpattern = prevfn.substr(0,found);
	}
	else
	{
		found = prevfn.rfind(' ');
		if (found!=string::npos)
		{
			prevfnpattern = prevfn.substr(0,found);
		}
		else
		{
			prevfnpattern = prevfn;
		}
	}
	found = newfn.rfind('_');
	if (found!=string::npos)
	{
		newfnpattern = newfn.substr(0,found);
	}
	else
	{
		found = newfn.rfind(' ');
		if (found!=string::npos)
		{
			newfnpattern = newfn.substr(0,found);
		}
		else
		{
			newfnpattern = newfn;
		}
	}
	*/

	string prevfnpattern(GetFilenamePattern(prevfilename));
	string newfnpattern(GetFilenamePattern(newfilename));
	//2) compare only left of last underscore
	if (newfnpattern.compare(prevfnpattern) == 0)
	{
		//if prevfilename same pattern as newfilename
		return false;
	}
	//if prevfilename has different pattern than newfilename
	return true;
}



//#define CONCATENATEATTACKS	0
//#define CONCATENATEATTACKS	1

void InstrumentSet::Play(PaStreamParameters* pPaStreamParameters, float numberofsecondsinplayback, int numberofinstrumentsinplayback, int iCONCATENATEATTACKSflag)
{
	/////////////////////////////////////////////////////////////
	//now, launch each instrument, each playing a default pattern 
	/////////////////////////////////////////////////////////////
	swprintf(pWCHAR, L"number of instruments in this instrumentset = %d\n", instrumentvector.size());StatusAddText(pWCHAR);
	//browse through each instrument
	vector<Instrument*>::iterator it;
	int i=0;
	for(it=instrumentvector.begin(); it<instrumentvector.end(); it++)
	{
		if(*it!=NULL) 
		{
			Instrument* pInstrument=*it;
			swprintf(pWCHAR, L"number of wavset in this instrument = %d\n", pInstrument->wavsetvector.size());StatusAddText(pWCHAR);
			WavSet* pWavSet3 = new WavSet; //used with CONCATENATEATTACKS
			//browse through each wavset
			vector<WavSet*>::iterator iterator;
			for(iterator=pInstrument->wavsetvector.begin(); iterator<pInstrument->wavsetvector.end(); iterator++)
			{
				if(*iterator!=NULL) 
				{
					WavSet* pWavSet=*iterator;
					//if(0) 
					if(!iCONCATENATEATTACKSflag) 
					{
						/////////////////////////////////////
						// play wavset as is using port audio 
						/////////////////////////////////////
						pWavSet->Play(pPaStreamParameters); //(&outputParameters);
					}

					if(iCONCATENATEATTACKSflag)
					{
						/////////////////////////////////////////////
						//concatenate attacks only (for a later play)
						/////////////////////////////////////////////
						WavSet* pWavSet2 = new WavSet;
						pWavSet2->Copy(pWavSet, 0.2f, 0.0f); //0.2 sec
						pWavSet3->Concatenate(pWavSet2);
						delete pWavSet2;
					}
				}
				else
				{
					assert(false);
				}
			}
			if(iCONCATENATEATTACKSflag)
			{
				//no more than numberofinstrumentsinplayback instruments simultaneously
				if(i!=0 && i%numberofinstrumentsinplayback==0)
				{
					//wait for the numberofinstrumentsinplayback tracks to be finished playing
					Sleep((int)numberofsecondsinplayback*1000);
				}

				if(0)
				{
					//////////////////////////////////////
					// play wavset3 as is using port audio 
					//////////////////////////////////////
					pWavSet3->Play(pPaStreamParameters);//(&outputParameters);
				}

				if(0)
				{
					///////////////////////////////
					// play wavset3 as is using sox 
					///////////////////////////////
					i++;
					char charBuffer2[WAVSET_CHARNAME_MAXLENGTH];
					char charBuffer3[WAVSET_CHARNAME_MAXLENGTH];
					sprintf_s(charBuffer2, WAVSET_CHARNAME_MAXLENGTH-1, "%d", i);
					string filename="instrument";
					filename = filename + charBuffer2 + ".wav";
					pWavSet3->WriteWavFile(filename.c_str());
					char charBuffer5[10]={"/b"}; //release runs start with /b option which stands for background
					char charBuffer6[10]={"-q"}; //release runs sox.exe with -q option which stands for quiet
					#ifdef _DEBUG
							sprintf_s(charBuffer5,10-1,""); //debug runs start without /b option which stands for background
							sprintf_s(charBuffer6,10-1,""); //debug runs sox.exe without -q option which stands for quiet
					#endif //_DEBUG
					int numRepeat = (int)numberofsecondsinplayback/pWavSet3->GetWavSetLength();
					//sprintf_s(charBuffer3,WAVSET_CHARNAME_MAXLENGTH-1,"start %s C:\\sox-14-3-2\\sox %s \"%s\" -d repeat %d",charBuffer5,charBuffer6,filename.c_str(),numRepeat);
					sprintf_s(charBuffer3,WAVSET_CHARNAME_MAXLENGTH-1,"start %s c:\\app-bin\\sox\\sox.exe %s \"%s\" -d repeat %d",charBuffer5,charBuffer6,filename.c_str(),numRepeat);
					system(charBuffer3);
				}
				if(1)
				{
					///////////////////////////////////
					// play wavset3 as is using spiplay 
					///////////////////////////////////
					i++;
					char charBuffer2[WAVSET_CHARNAME_MAXLENGTH];
					char charBuffer3[WAVSET_CHARNAME_MAXLENGTH];
					sprintf_s(charBuffer2, WAVSET_CHARNAME_MAXLENGTH-1, "%d", i);
					string filename="wsip_instrument";
					filename = filename + charBuffer2 + ".wav";
					pWavSet3->WriteWavFile(filename.c_str());
					char charBuffer5[10]={"/b"}; //release runs start with /b option which stands for background
					#ifdef _DEBUG
							sprintf_s(charBuffer5,10-1,""); //debug runs start without /b option which stands for background
					#endif //_DEBUG
					//sprintf_s(charBuffer3,WAVSET_CHARNAME_MAXLENGTH-1,"start %s C:\\spoirier\\oifii-org\\httpdocs\\ns-org\\nsd\\ar\\cp\\audio_spi\\spiplay\\Release\\spiplay.exe \"%s\" %f",charBuffer5,filename.c_str(),numberofsecondsinplayback);
					sprintf_s(charBuffer3,WAVSET_CHARNAME_MAXLENGTH-1,"start %s c:\\app-bin\\spiplay\\spiplay.exe \"%s\" %f",charBuffer5,filename.c_str(),numberofsecondsinplayback);
					system(charBuffer3);
				}
			}
			delete pWavSet3;
		}
		else
		{
			assert(false);
		}
	}

}

bool InstrumentSet::HasOneInstrument()
{
	if(instrumentvector.empty()) return false;
	return true;
}

int InstrumentSet::GetNumberOfInstrument()
{
	return instrumentvector.size();
}

Instrument* InstrumentSet::GetInstrumentFromID(int idinstrument)
{
	assert(idinstrument>-1);
	assert(idinstrument<instrumentvector.size());
	Instrument* pInstrument = NULL;
	pInstrument = instrumentvector.at(idinstrument);
	return pInstrument;
}

Instrument* InstrumentSet::GetInstrumentRandomly()
{
	Instrument* pInstrument = NULL;
	int random_integer;
	int lowest=1, highest=instrumentvector.size();
	int range=(highest-lowest)+1;
	random_integer = lowest+int(range*rand()/(RAND_MAX + 1.0));

	int idinstrument = random_integer-1;
	pInstrument = instrumentvector.at(idinstrument);
	return pInstrument;
}

Instrument* InstrumentSet::GetInstrumentFromMidiTrackName(Partition* pPartition)
{
	assert(pPartition);
	Instrument* pInstrument = NULL;
	//todo: spi
	pPartition->miditrackname;
	//note, do some mapping using miditrackname
	int idinstrument = instrumentvector.size()-1; //always pick the last instrument for now
	pInstrument = instrumentvector.at(idinstrument);
	return pInstrument;
}


bool InstrumentSet::OpenAllStreams(PaStreamParameters* pPaStreamInputParameters, PaStreamParameters* pPaStreamOutputParameters, PaStreamCallback* pPaStreamCallback)
{
	vector<Instrument*>::iterator it;
	for(it=instrumentvector.begin(); it<instrumentvector.end(); it++)
	{
		(*it)->OpenAllStreams(pPaStreamInputParameters, pPaStreamOutputParameters, pPaStreamCallback);
	}
	return true;
}

bool InstrumentSet::CloseAllStreams()
{
	vector<Instrument*>::iterator it;
	for(it=instrumentvector.begin(); it<instrumentvector.end(); it++)
	{
		(*it)->CloseAllStreams();
	}
	return true;
}