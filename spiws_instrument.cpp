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
#include <vector>
#include <algorithm>
#include <fstream>

#include <iostream>
#include <sstream>
#include <string>

#include "portaudio.h"
#include "spiws_wavset.h"
#include "spiws_midieventset.h"
#include "spiws_instrument.h"
#include "spiws_midiutility.h"

using namespace std;

static std::string outstring;

extern CHAR pCHAR[1024];
extern WCHAR pWCHAR[1024];


struct wavset_less {
    bool operator ()(WavSet const& a, WavSet const& b) const 
	{
        if (a.midinote < b.midinote) return true;
        if (a.midinote > b.midinote) return false;
        return false;
    }
};

Instrument::Instrument()
{
	instrumentname = "noinstname";
	sortedonmidinote = false;
	patterncodeassigned = false;
}

Instrument::~Instrument()
{
	vector<WavSet*>::iterator it;
	for(it=wavsetvector.begin(); it<wavsetvector.end(); it++)
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

bool Instrument::SetInstrumentName(string filenamepattern)
{
	instrumentname = filenamepattern;
	return true;
}

const char* Instrument::GetInstrumentName()
{
	return instrumentname.c_str();
}

const char* Instrument::GetInstrumentNameWithoutPath()
{
	outstring = instrumentname;
	int found = outstring.rfind('\\');
	if (found!=string::npos)
	{
		outstring = outstring.substr(found+1);
	}
	return outstring.c_str();
}

int Instrument::GetNumberOfWavSet()
{
	return wavsetvector.size();
}

WavSet* Instrument::GetWavSetFromID(int idwavset)
{
	assert(idwavset>-1);
	assert(idwavset<wavsetvector.size());
	WavSet* pWavSet = NULL;
	pWavSet = wavsetvector.at(idwavset);
	return pWavSet;
}

WavSet* Instrument::GetWavSetRandomly()
{
	WavSet* pWavSet = NULL;
	int random_integer;
	int lowest=1, highest=wavsetvector.size();
	int range=(highest-lowest)+1;
	random_integer = lowest+int(range*rand()/(RAND_MAX + 1.0));

	int idwavset = random_integer-1;
	pWavSet = wavsetvector.at(idwavset);
	return pWavSet;
}


WavSet* Instrument::GetWavSetFromFrequency(float frequency_hz)
{
	WavSet* pWavSet = NULL;
	int midinotenumber = GetMidiNoteNumberFromFrequency(frequency_hz);
	pWavSet = GetWavSetFromMidiNoteNumber(midinotenumber);
	return pWavSet;
}

WavSet* Instrument::GetWavSetFromMidiNoteName(const char* midinotename)
{
	WavSet* pWavSet = NULL;
	int midinotenumber = GetMidiNoteNumberFromMidiNoteName(midinotename); 
	pWavSet = GetWavSetFromMidiNoteNumber(midinotenumber);
	return pWavSet;
}

WavSet* Instrument::GetWavSetFromMidiNoteNumber(int midinotenumber)
{
	WavSet* pWavSet = NULL;
	//assert(midinotenumber>=0 && midinotenumber<128);
	if(midinotenumber<0 || midinotenumber>127)
	{
		assert(false);
		return pWavSet;
	}

	/*
	int idwavset = wavsetvector.size()*midinotenumber/128;
	pWavSet = wavsetvector.at(idwavset);
	*/

	//2) find all matching wavset
	vector<WavSet*> tempwavsetvector;
	vector<WavSet*>::iterator it;
	for(it=wavsetvector.begin(); it<wavsetvector.end(); it++)
	{
		if(midinotenumber==(*it)->midinote)
		{
			tempwavsetvector.push_back(*it);
		}
	}
	//3) pick one (at random if more than one matching)
	if(!tempwavsetvector.empty())
	{
		if(tempwavsetvector.size()==1)
		{
			pWavSet = tempwavsetvector.at(0);
		}
		else
		{
			#ifdef _DEBUG
			if(1)
			{
				swprintf(pWCHAR, L"warning, %d wavsets matching midi note number %d\n", tempwavsetvector.size(), midinotenumber);StatusAddText(pWCHAR);
			}
			#endif //_DEBUG
			int random_integer;
			int lowest=1, highest=tempwavsetvector.size();
			int range=(highest-lowest)+1;
			random_integer = lowest+int(range*rand()/(RAND_MAX + 1.0));

			int idwavset = random_integer-1;
			pWavSet = tempwavsetvector.at(idwavset);
		}
	}
	else
	{
		#ifdef _DEBUG
		if(1)
		{
			swprintf(pWCHAR, L"warning, no wavsets matching midi note number %d\n", midinotenumber);StatusAddText(pWCHAR);
		}
		#endif //_DEBUG
		int idwavset = wavsetvector.size()*midinotenumber/128;
		pWavSet = wavsetvector.at(idwavset);
	}
	/*
	//1) find note name
	string name = notename[midinotenumber];
	//2) find all matching wavset
	vector<WavSet*> tempwavsetvector;
	vector<WavSet*>::iterator it;
	for(it=wavsetvector.begin(); it<wavsetvector.end(); it++)
	{
		string wavsetname = (*it)->GetName();
		if(wavsetname.rfind(name)!=string::npos)
		{
			tempwavsetvector.push_back(*it);
		}
	}
	//3) pick one (at random if more than one matching)
	if(!tempwavsetvector.empty())
	{
		if(tempwavsetvector.size()==1)
		{
			pWavSet = tempwavsetvector.at(0);
		}
		else
		{
			int random_integer;
			int lowest=1, highest=tempwavsetvector.size();
			int range=(highest-lowest)+1;
			random_integer = lowest+int(range*rand()/(RAND_MAX + 1.0));

			int idwavset = random_integer-1;
			pWavSet = tempwavsetvector.at(idwavset);
		}
	}
	*/
	return pWavSet;
}

WavSet* Instrument::GetWavSetFromMidiNoteNumber(MidiEventSet* pMidiEventSet)
{
	assert(pMidiEventSet);
	WavSet* pWavSet = NULL;
	int notenumber = pMidiEventSet->GetNoteNumber();
	return GetWavSetFromMidiNoteNumber(notenumber);
}

class WavSet* Instrument::GetWavSetFromPatternCode(const char* patterncode)
{
	assert(patterncode);
	assert(strlen(patterncode)==1);
	WavSet* pWavSet = NULL;

	//2) find all matching wavset
	vector<WavSet*> tempwavsetvector;
	vector<WavSet*>::iterator it;
	for(it=wavsetvector.begin(); it<wavsetvector.end(); it++)
	{
		if(*patterncode==(*it)->GetPatternCode())
		{
			tempwavsetvector.push_back(*it);
		}
	}
	//3) pick one (at random if more than one matching)
	if(!tempwavsetvector.empty())
	{
		if(tempwavsetvector.size()==1)
		{
			pWavSet = tempwavsetvector.at(0);
		}
		else
		{
			int random_integer;
			int lowest=1, highest=tempwavsetvector.size();
			int range=(highest-lowest)+1;
			random_integer = lowest+int(range*rand()/(RAND_MAX + 1.0));

			int idwavset = random_integer-1;
			pWavSet = tempwavsetvector.at(idwavset);
		}
	}
	else
	{
		//no match for this patterncode
		assert(false);
	}
	return pWavSet;
}

class WavSet* Instrument::GetWavSetFromPatternCode(const char* patterncode, int patternrange)
{
	assert(patterncode);
	WavSet* pWavSet = NULL;
	int instrumentrange = wavsetvector.size();

	int patterncodenumber = atoi(patterncode);//todo: GetPatternCodeNumber(patterncode);
	if(patterncodenumber<0 || patterncodenumber>9) 
	{
		//todo:
		assert(false);
		return NULL;
	}
	int idwavset = instrumentrange*(patterncodenumber-1)/patternrange;
	pWavSet = wavsetvector.at(idwavset);
	return pWavSet;
}

bool Instrument::IsValidWavFolder(const char* wavfoldername)
{
	assert(wavfoldername);
	//todo, verify that wavfoldername contains wav files
	//      that wav files are valid
	return true;
}

const char* Instrument::GetWavFolderFromWavFoldersFilename(const char* wavfoldersfilename)
{
	assert(wavfoldersfilename);
	//1) load in all folders
	vector<string> foldernames;
	vector<string>::iterator it;
	ifstream ifs(wavfoldersfilename);
	string temp;
	while(getline(ifs,temp))
	{
		if(IsValidWavFolder(temp.c_str()))
		{
			foldernames.push_back(temp);
		}
	}
	if(foldernames.empty()) 
	{
		//can't find file of file is empty
		assert(false);
		return "";
	}

	//2) pick a folder at random
	int random_integer;
	int lowest=1, highest=foldernames.size();
	int range=(highest-lowest)+1;
	random_integer = lowest+int(range*rand()/(RAND_MAX + 1.0));


	outstring = foldernames.at(random_integer-1);
	return outstring.c_str();
}

int Instrument::GetMidiNoteNumberFromMidiNoteName(const char* midinotename)
{
	return GetMidiNoteNumberFromWavFilename(midinotename);
}

int  Instrument::GetMidiNoteNumberFromWavFilename(const char* wavfilename)
{
	return GetMidiNoteNumberFromString(wavfilename);
}


/*
int  Instrument::GetMidiNoteNumberFromWavFilename(const char* wavfilename)
{
	int notemidi = -1;
	bool octaveconfirmed = false;
	int octave = -1;
	int note = -1;
	bool sharp = false;
	string notename = "";
	string filename = wavfilename;
	//0) remove path
	int pos = filename.rfind("\\");
	if(pos!=string::npos)
	{
		filename = filename.substr(pos+1,filename.size());
	}
	//1) find octave, sharp and notename
	string::iterator it;
	//for(it=filename.end()-1; it>filename.begin(); it--)
	//for(it=filename.end()-1; it>=filename.begin(); it--)
	for(it=filename.end()-1; it>filename.begin(); it--)
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
	}
	return notemidi;
}
*/

int Instrument::GetMidiNoteNumberFromTxtFilename(const char* wavfilename)
{
	//if spipitchdetection .txt file has been generated
	char txtfilename[WAVSET_CHARNAME_MAXLENGTH] = {""};
	strcpy_s(txtfilename, WAVSET_CHARNAME_MAXLENGTH-1, wavfilename);
	char* pChar = strrchr(txtfilename,'.');
	strcpy(&(pChar[1]), "txt");
	char notename[10];
	float notefreq;
	int notemidi = -1;
	FILE* pFILE = fopen(txtfilename, "r");
	if(pFILE)
	{
		fscanf(pFILE, "%f %s %d", &notefreq, &notename, &notemidi);
		fclose(pFILE);
	}
	return notemidi;
}

bool Instrument::CreateWavSynth(int flag)
{
	//INSTRUMENT_MAXWAVSETLENGTH
	if(flag==INSTRUMENT_SYNTH_SINWAV)
	{
		SetInstrumentName("INSTRUMENT_SYNTH_SINWAV");
		//1) create 128 wavset
		for(int i=0; i<128; i++)
		{
			WavSet* pWavSet = new WavSet;
			bool result = pWavSet->CreateSin(INSTRUMENT_MAXWAVSETLENGTH, 44100, 2, GetFrequencyFromMidiNoteNumber(i));
			if(result) 
			{
					pWavSet->midinote = i;
					wavsetvector.push_back(pWavSet);
			}
			else 
			{
				swprintf(pWCHAR, L"error in Instrument::CreateWavSynth(), not enough memory?\n");StatusAddText(pWCHAR);
				swprintf(pWCHAR, L"failed at id=%d\n", i);StatusAddText(pWCHAR);
				return false;
			}
		}
	}
	else if(flag==INSTRUMENT_SYNTH_SQUAREWAV)
	{
		SetInstrumentName("INSTRUMENT_SYNTH_SQUAREWAV");
		//1) create 128 wavset
		for(int i=0; i<128; i++)
		{
			WavSet* pWavSet = new WavSet;
			bool result = pWavSet->CreateSquare(INSTRUMENT_MAXWAVSETLENGTH, 44100, 2, GetFrequencyFromMidiNoteNumber(i));
			if(result) 
			{
					pWavSet->midinote = i;
					wavsetvector.push_back(pWavSet);
			}
			else 
			{
				swprintf(pWCHAR, L"error in Instrument::CreateWavSynth(), not enough memory?\n");StatusAddText(pWCHAR);
				swprintf(pWCHAR, L"failed at id=%d\n", i);StatusAddText(pWCHAR);
				return false;
			}
		}
	}
	else if(flag==INSTRUMENT_SYNTH_SAWWAV)
	{
		SetInstrumentName("INSTRUMENT_SYNTH_SAWWAV");
		//1) create 128 wavset
		for(int i=0; i<128; i++)
		{
			WavSet* pWavSet = new WavSet;
			bool result = pWavSet->CreateSaw(INSTRUMENT_MAXWAVSETLENGTH, 44100, 2, GetFrequencyFromMidiNoteNumber(i));
			if(result) 
			{
					pWavSet->midinote = i;
					wavsetvector.push_back(pWavSet);
			}
			else 
			{
				swprintf(pWCHAR, L"error in Instrument::CreateWavSynth(), not enough memory?\n");StatusAddText(pWCHAR);
				swprintf(pWCHAR, L"failed at id=%d\n", i);StatusAddText(pWCHAR);
				return false;
			}
		}
	}
	else if(flag==INSTRUMENT_SYNTH_TRIWAV)
	{
		SetInstrumentName("INSTRUMENT_SYNTH_TRIWAV");
		//1) create 128 wavset
		for(int i=0; i<128; i++)
		{
			WavSet* pWavSet = new WavSet;
			bool result = pWavSet->CreateTri(INSTRUMENT_MAXWAVSETLENGTH, 44100, 2, GetFrequencyFromMidiNoteNumber(i));
			if(result) 
			{
					pWavSet->midinote = i;
					wavsetvector.push_back(pWavSet);
			}
			else 
			{
				swprintf(pWCHAR, L"error in Instrument::CreateWavSynth(), not enough memory?\n");StatusAddText(pWCHAR);
				swprintf(pWCHAR, L"failed at id=%d\n", i);StatusAddText(pWCHAR);
				return false;
			}
		}
	}
	else
	{
		assert(false);
	}
	return true;
}

bool Instrument::WriteWavFiles(int flag)
{
	if(flag==INSTRUMENT_TEMPFOLDER)
	{
		char pBuffer[5] = {""};
		//string path = "d:\\temp\\";
		string path = "c:\\temp\\";
		path += GetInstrumentName();
		//create dir if required
		FILE* pFILE = fopen(path.c_str(), "r");
		if(pFILE==NULL)
		{
			//create dir
			string command = "mkdir " + path;
			system(command.c_str());
		}

		vector<WavSet*>::iterator it;
		int i=0;
		for(it=wavsetvector.begin(); it<wavsetvector.end(); it++)
		{
			int midinotenumber = (*it)->midinote;
			if(midinotenumber>-1 && midinotenumber<128)
			{
				string filename = path + "\\" + notename[midinotenumber] + ".WAV";
				(*it)->WriteWavFile(filename.c_str());
			}
			else
			{
				sprintf_s(pBuffer, 5-1, "%d", i);
				string filename = path + "\\" + pBuffer + ".WAV";
				(*it)->WriteWavFile(filename.c_str());
			}
			i++;
		}
	}
	else
	{
		assert(false);
	}
	return true;
}

bool Instrument::CreateFromWavFilenamesFile(const char* wavfilenamesfile, int maxnumberofwavset)
{
	//1) load in all wav files from wavfolder
	vector<string> wavfilenames;
	ifstream ifs(wavfilenamesfile); //ifstream ifs("wsic_filenames.txt");
	//ifstream ifs(wavfilenamesfile, ifstream::trunc); //ifstream ifs("wsic_filenames.txt");
	string temp;
	while(getline(ifs,temp))
	{
		wavfilenames.push_back(temp); //wavfilenames.push_back(path + "\\" + temp);
	}

	SetInstrumentName(wavfilenamesfile);//SetInstrumentName(wavfolder);
	bool dosort=false;
	//2) browse through these wav files, create and attach a wavset for each valid wav files
	vector<string>::iterator it;
	for(it=wavfilenames.begin(); it<wavfilenames.end(); it++)
	{
		//2.1) create wavset
		WavSet* pWavSet = new WavSet;
		pWavSet->ReadWavFile((*it).c_str()); //*it is a .wav filename
		//2.2) is supported?
		/*
		if(pWavSet->numChannels==1 && pWavSet->SampleRate==44100)
		{
			pWavSet->Resample44100monoTo44100stereo();
		}
		if( (pWavSet->numChannels==2 && pWavSet->SampleRate!=44100) 
			|| (pWavSet->numChannels==1 && pWavSet->SampleRate!=22050) )
		{
			printf("exluding %s because sample is %d Hz\n",(*it).c_str(),pWavSet->SampleRate);
			delete pWavSet;
			continue;
		}
		*/
		if(pWavSet->numChannels==1 && pWavSet->SampleRate==44100)
		{
			pWavSet->Resample44100monoTo44100stereo();
		}
		else if(pWavSet->numChannels==1 && pWavSet->SampleRate==48000)
		{
			pWavSet->Resample48000monoTo44100stereo();
		}
		else if(pWavSet->numChannels==2 && pWavSet->SampleRate==48000)
		{
			pWavSet->Resample48000stereoTo44100stereo();
		}
		if( (pWavSet->numChannels==2 && pWavSet->SampleRate!=44100) 
			|| (pWavSet->numChannels==1 && pWavSet->SampleRate!=22050) )
		{
			sprintf(pCHAR, "exluding %s because sample is %d Hz\n",(*it).c_str(),pWavSet->SampleRate);StatusAddText(pCHAR);
			delete pWavSet;
			continue;
		}
		//2.3) attach wavset
		if(wavsetvector.size()<= maxnumberofwavset)
		{
			wavsetvector.push_back(pWavSet);
			//2.4) set midi note from wav filename
			int notemidi=-1;
			notemidi = GetMidiNoteNumberFromWavFilename((*it).c_str());
			if(notemidi==-1)
			{
				//2.5) or, if not possible, try to set midi note from spipitchdetection .txt file
				notemidi = GetMidiNoteNumberFromTxtFilename((*it).c_str());
			}
			//2014july06, spi, begin
			//if(notemidi!=-1)
			if(notemidi>-1 && notemidi<128)
			//2014july06, spi, end
			{
				dosort = true;
				pWavSet->SetMidiNote(notemidi);
			}
		}
		else
		{
			break;
		}
	}
	if(wavsetvector.empty()) return false;
	if(dosort)
	{
		sort(wavsetvector.begin(), wavsetvector.end(), wavset_less());
		sortedonmidinote = true;
		#ifdef _DEBUG
		//for debugging, to ensure sort algorithm worked ok
		if(0)
		{
			vector<WavSet*>::iterator iterator;
			int i;
			for(iterator=wavsetvector.begin(); iterator<wavsetvector.end(); iterator++)
			{
				i = (*iterator)->GetMidiNote();
			}
		}
		#endif //_DEBUG
		SetWavSetPatternCodes(INSTRUMENT_PATTERNCODETOEACHMIDINOTE);
	}
	else
	{
		SetWavSetPatternCodes(INSTRUMENT_PATTERNCODETOALL);
	}
	return true;
}

string temp;
const char* Instrument::GetWavSetPatternCodes()
{
	temp="";
	vector<WavSet*>::iterator iterator;
	for(iterator=wavsetvector.begin(); iterator<wavsetvector.end(); iterator++)
	{
		temp+=(*iterator)->GetPatternCode();
	}
	return temp.c_str();
}

const char* Instrument::GetWavSetPatternNotes()
{
	temp="";
	vector<WavSet*>::iterator iterator;
	for(iterator=wavsetvector.begin(); iterator<wavsetvector.end(); iterator++)
	{
		int midinote = (*iterator)->GetMidiNote();
		if(midinote>-1 && midinote<128)
		{
			temp= temp + notename[midinote] + ", ";
		}
	}
	return temp.c_str();
}

void Instrument::SetWavSetPatternCodes(int flag)
{
	assert(flag==INSTRUMENT_PATTERNCODETOALL || flag==INSTRUMENT_PATTERNCODETOEACHMIDINOTE);
	vector<WavSet*>::iterator iterator;
	int prevmidinote =-1;
	int thismidinote =-1;
	int i=0;
	for(iterator=wavsetvector.begin(); iterator<wavsetvector.end(); iterator++)
	{
		if(flag==INSTRUMENT_PATTERNCODETOEACHMIDINOTE)
		{
			thismidinote = (*iterator)->GetMidiNote();
			if(prevmidinote!=-1 && thismidinote!=prevmidinote)
			{
				i++;
			}
		}
		if(i<10)
		{
			//ascii 48 is 0, ascii 57 is 9
			(*iterator)->SetPatternCode(i+48);
		}
		else if(i>9 && i<36)
		{
			//ascii 65 is A, ascii 90 is Z
			(*iterator)->SetPatternCode(i+55);
		}
		else if(i>35 && i<(35+128))
		{
			//from ascii 128 to ascii 255
			(*iterator)->SetPatternCode(i+128-36);
		}
		else
		{
			assert(false);
		}
		if(flag==INSTRUMENT_PATTERNCODETOALL)
		{
			i++;
		}
		else if(flag==INSTRUMENT_PATTERNCODETOEACHMIDINOTE)
		{
			prevmidinote = thismidinote;
		}
	}
}

bool Instrument::CreateFromWavFolder(const char* wavfolder, int maxnumberofwavset)
{
	assert(wavfolder);
	//0) execute cmd line to get all folder's .wav filenames
	string quote = "\"";
	string pathfilter;
	string path=wavfolder;
	pathfilter = path + "\\*.wav";
	string systemcommand;
	//systemcommand = "DIR " + quote + pathfilter + quote + "/B /O:N > wsic_filenames.txt"; //wsip tag standing for wav set (library) instrumentset (class) populate (function)
	systemcommand = "DIR " + quote + pathfilter + quote + "/B /S /O:N > wsic_filenames.txt"; // /S for adding path into "wsic_filenames.txt"
#ifdef _DEBUG
	sprintf(pCHAR, "%s\n", systemcommand);StatusAddText(pCHAR);
#endif //_DEBUG
	system(systemcommand.c_str());

	return CreateFromWavFilenamesFile("wsic_filenames.txt", maxnumberofwavset);
}

bool Instrument::CreateFromWavFilenamesFilter(const char* wavfilenamesfilter,  int maxnumberofwavset)
{
	//1) retreive all "wavfolder_*.txt" file
	//0) execute cmd line to get all folder's .wav filenames
	string quote = "\"";
	string pathfilter;
	//string path=wavfolder;
	//pathfilter = path + "\\*.wav";
	pathfilter = "wavfolder_*.txt";
	string systemcommand;
	systemcommand = "DIR " + quote + pathfilter + quote + "/B /O:N > wsic_txtfilenames.txt"; //wsip tag standing for wav set (library) instrumentset (class) populate (function)
#ifdef _DEBUG
	sprintf(pCHAR, "%s\n", systemcommand);StatusAddText(pCHAR);
#endif //_DEBUG
	system(systemcommand.c_str());

	//2) load in all "wavfolder_*.txt" file
	vector<string> txtfilenames;
	ifstream ifs("wsic_txtfilenames.txt");
	string temp;
	while(getline(ifs,temp))
	{
		//txtfilenames.push_back(path + "\\" + temp);
		txtfilenames.push_back(temp);
	}

	if(wavfilenamesfilter==NULL || strcmp(wavfilenamesfilter, "")==0)
	{
		//3.1) get a random wavfilenamesfile
		if(txtfilenames.empty()) 
		{
			//there is probably no "wavfolder_*.txt" in application directory
			//application directory is different depending if running debug or release version
			//typically, for developing debug and testing release, all "wavfolder_*.txt" files should be copied to both directory
			assert(false);
			return false;
		}
		//3.2) pick a folder at random
		int random_integer;
		int lowest=1, highest=txtfilenames.size();
		int range=(highest-lowest)+1;
		random_integer = lowest+int(range*rand()/(RAND_MAX + 1.0));

		outstring = txtfilenames.at(random_integer-1);
	}
	else
	{
		//4) use wavfilenamesfilter to get a wavfilenamesfile
		//assert(false); //todo
		//i.e. if pattern = "trombonesolo", it would specify a unique instrument
		//     if pattern = "trombone", it would find a few instrument and pick one at random
		string nopath = wavfilenamesfilter;
		int pos = nopath.rfind('\\');
		if(pos!=string::npos)
		{
			nopath = nopath.substr(pos+1,nopath.size());
		}
		//4.1) first, look for an exact match 
		vector<string>::iterator it;
		bool found = false;
		for(it=txtfilenames.begin(); it<txtfilenames.end(); it++)
		{
			if((*it).compare(nopath)==0) //if((*it).compare(wavfilenamesfilter)==0)
			{
				found=true;
				outstring = *it;
				break;
			}
		}
		if(found==false)
		{
			//4.2) second, look for substr matches
			vector<string> matchesvector;
			bool found = false;
			for(it=txtfilenames.begin(); it<txtfilenames.end(); it++)
			{
				if((*it).find(nopath)!=string::npos) //if((*it).compare(wavfilenamesfilter)==0)
				{
					found=true;
					matchesvector.push_back(*it);
				}
			}
			if(found==true)
			{
				//4.3) if a unique match is found
				if(matchesvector.size()==1)
				{
					outstring = matchesvector.at(0);
				}
				else
				{
					//4.4) if more than one match is found, pick one at random
					int random_integer;
					int lowest=1, highest=matchesvector.size();
					int range=(highest-lowest)+1;
					random_integer = lowest+int(range*rand()/(RAND_MAX + 1.0));

					outstring = matchesvector.at(random_integer-1);

				}
			}
			else
			{
				assert(false); //todo
				//1) find substr, build a vector of matching "wavfolder_*.txt" files
				//2) if more than one match found, pick one at random
				outstring = "wavfolder_basstuba.txt"; //for now, continue safely
			}
		}
	}
	return CreateFromWavFilenamesFile(outstring.c_str(), maxnumberofwavset);
}

bool Instrument::CreateFromWavFoldersFilename(const char* wavfoldersfilename, int maxnumberofwavset)
{
	assert(wavfoldersfilename);
	//1) pick a folder at random
	string wavfolder = GetWavFolderFromWavFoldersFilename(wavfoldersfilename);  

	//2) call CreateFromWavFolder()
	return CreateFromWavFolder(wavfolder.c_str(), maxnumberofwavset);
}

bool Instrument::CreateFromRootWavFoldersFilename(const char* rootwavfoldersfilename, int maxnumberofwavset)
{
	assert(rootwavfoldersfilename);
	//pick a folder at random
	string rootwavfolder = GetWavFolderFromWavFoldersFilename(rootwavfoldersfilename); //same function should work also for rootfolders

	//0) execute cmd line to get all root folder's sub folders
	string quote = "\"";
	string pathfilter;
	string path=rootwavfolder;
	pathfilter = path + "\\*."; //we want folders only
	string systemcommand;
	systemcommand = "DIR " + quote + pathfilter + quote + "/S /B /O:N > wavfolders_otherinstruments.txt"; // /S for recursive
#ifdef _DEBUG
	sprintf(pCHAR, "%s\n", systemcommand);StatusAddText(pCHAR);
#endif //_DEBUG
	system(systemcommand.c_str());

	return CreateFromWavFoldersFilename("wavfolders_otherinstruments.txt", maxnumberofwavset); 
}

bool Instrument::CreateFromName(const char* name, int maxnumberofwavset)
{
	assert(name);
	string namestring = name;
	if(namestring.compare("piano")==0)
	{
		return CreateFromWavFoldersFilename("wavfolders_piano.txt", maxnumberofwavset);
	}
	else if(namestring.compare("guitar")==0)
	{
		return CreateFromWavFoldersFilename("wavfolders_guitar.txt", maxnumberofwavset);
	}
	else if(namestring.compare("bass")==0)
	{
		return CreateFromWavFoldersFilename("wavfolders_bass.txt", maxnumberofwavset);
	}
	else if(namestring.compare("drumkit")==0)
	{
		return CreateFromWavFoldersFilename("wavfolders_drumkit.txt", maxnumberofwavset);
	}
	else if(namestring.compare("horns")==0)
	{
		return CreateFromWavFoldersFilename("wavfolders_horns.txt", maxnumberofwavset);
	}
	else if(namestring.compare("organ")==0)
	{
		return CreateFromWavFoldersFilename("wavfolders_organ.txt", maxnumberofwavset);
	}
	else if(namestring.compare("violin")==0)
	{
		return CreateFromWavFoldersFilename("wavfolders_violin.txt", maxnumberofwavset);
	}
	else if(namestring.compare("africa")==0)
	{
		return CreateFromWavFoldersFilename("wavfolders_otherinstruments_world_africa.txt", maxnumberofwavset);
	}
	else if(namestring.compare("otherinstruments")==0)
	{
		return CreateFromRootWavFoldersFilename("wavfolders_otherinstrumentsrootfolders.txt", maxnumberofwavset);
	}
	assert(false); //name must be in the list, otherwise use "otherinstruments"
	return false;
}

void Instrument::Play(PaStreamParameters* pPaStreamOutputParameters, int iflag)
{
	vector<WavSet*>::iterator it;
	if(iflag==INSTRUMENT_WAVSETINSEQUENCE)
	{
		for(it=wavsetvector.begin(); it<wavsetvector.end(); it++)
		{
			(*it)->Play(pPaStreamOutputParameters, INSTRUMENT_MAXWAVSETLENGTH);
		}
	}
	else if(iflag==INSTRUMENT_WAVSETALLATONCE)
	{
		//1) create audio buffer
		WavSet* pWavSet = new WavSet;
		if(pWavSet)
		{
			if(pWavSet->CreateSilence(INSTRUMENT_MAXWAVSETLENGTH))
			{
				string name = "";
				for(it=wavsetvector.begin(); it<wavsetvector.end(); it++)
				{
					name += (*it)->GetName();
					float amplitude = 0.25;
					int n = wavsetvector.size();
					if(n<11) amplitude=0.5;
					else if(n<21) amplitude=0.25;
					else if(n<51) amplitude=0.15;
					else amplitude = 0.1;
					pWavSet->Sum(amplitude, (*it), 0.0, INSTRUMENT_MAXWAVSETLENGTH);
				}
				//2) play it
				pWavSet->SetName(name.c_str());
				pWavSet->Play(pPaStreamOutputParameters);
				
			}
			//3) delete it
			delete pWavSet;
		}
	}
	else if(iflag==INSTRUMENT_WAVSETRANDOM)
	{
		//todo:
		assert(false);
	}
	else
	{
		assert(false);
	}
	return ;
}

bool Instrument::SplitWavSetsInSegments(double fSecondsPerSegment)
{
	vector<WavSet*>::iterator it;
	for(it=wavsetvector.begin(); it<wavsetvector.end(); it++)
	{
		(*it)->SplitInSegments(fSecondsPerSegment);
	}
	return true;
}

void Instrument::DisplayMidiStats()
{
	swprintf(pWCHAR, L"\n");StatusAddText(pWCHAR);
	vector<WavSet*>::iterator it;
	int i=0;
	for(it=wavsetvector.begin(); it<wavsetvector.end(); it++)
	{
		swprintf(pWCHAR, L"(%d,%d) ", i, (*it)->midinote);StatusAddText(pWCHAR);
		i++;
	}
	swprintf(pWCHAR, L"\n");StatusAddText(pWCHAR);
}

bool Instrument::OpenAllStreams(PaStreamParameters* pPaStreamInputParameters, PaStreamParameters* pPaStreamOutputParameters, PaStreamCallback* pPaStreamCallback)
{
	vector<WavSet*>::iterator it;
	for(it=wavsetvector.begin(); it<wavsetvector.end(); it++)
	{
		(*it)->OpenStream(pPaStreamInputParameters, pPaStreamOutputParameters, pPaStreamCallback);
	}
	return true;
}

bool Instrument::CloseAllStreams()
{
	vector<WavSet*>::iterator it;
	for(it=wavsetvector.begin(); it<wavsetvector.end(); it++)
	{
		(*it)->CloseStream();
	}
	return true;
}
