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
//2012june01, creation with ReadWavFile(), SplitInSegments() and GetPointerToSegmentData() for spisplitshuffleplay
//2012june04, spi, added EraseSegment() for spisplitdisperseplay, tested OK
//2012june04, spi, added WriteWavFile() for spisplitdisperseplay
//2012june04, spi, added WavSet(WavSet* pWavSet) and Copy() for spisplitdisperseplay
//2012june04, spi, added WriteWavFile() for spisplitdisperseplay
//2012june06, spi, added FadeSegmentEdges() for spisplitdisperseplay
//2012june08, spi, added GetSegmentsLength() for spiplay
//2012june08, spi, added GetWavSetLength() for spiplay
//2012june10, spi, added ReadWavFileHeader() for spinavwavfolders_sspp 
//				in order to replace lib soundtouch by class wavset 
//				which will now depend on libsndfile
//2012june11, spi, added assert(numChannels==2); statements where ever needed
//				mono wav file not supported for now
//2012june11, spi, adding CreateSilence(), CreateSin() and CreateTri()
//				as well as Concatenate() and Mix()
//2012june12, spi, adding Copy(class WavSet* pWavSet, float duration_s, float offset_s)
//				and Copy(class WavSet* pWavSet, int duration_sample, int offset_sample)
//2012june12, spi, adding Play()
//
//
//nakedsoftware.org, spi@oifii.org or stephane.poirier@oifii.org
////////////////////////////////////////////////////////////////

#include "stdafx.h"

//#define SPIWAVSETLIB_API
#include "spiwavsetlib.h"

#include <stdlib.h>
#include <stdio.h>
#include "portaudio.h"

/*
#include "WavFile.h"
#include "SoundTouch.h"
using namespace soundtouch;
*/
#include	<sndfile.hh>

#include "spiws_WavSet.h"
#include "spiws_instrument.h"
#include <memory.h>
#include <assert.h>
#include <math.h>
#define M_PI       3.14159265358979323846

using namespace std;
#include <string>
#include <ShellAPI.h>

extern CHAR pCHAR[1024];
extern WCHAR pWCHAR[1024];

//int iSoxPathCharArray = ; 
//const char* pSoxPathCharArray[]={"",""};
//int iSpiPlayPathCharArray = ; 
//const char* pSpiPlayPathCharArray[]={"",""};

WavSet::WavSet()
{
	Init();
}

void WavSet::Init()
{
	SampleRate = 0;
	totalFrames = 0; 
	numChannels = 0;
	numSamples = 0;  
	numBytes = 0;
	pSamples = NULL;
	SetName("");

	numFramesPerSegment = -1;
	numSegments = -1;
	numSamplesPerSegment = -1;  
	numBytesPerSegment = -1;

	idSegmentSelected = -1;

	midinote = -1;
	patterncode = -1;

	fadein = false;
	fadeout = false;
	pPaStream = NULL;

	frameIndex=0;

}

WavSet::WavSet(WavSet* pWavSet, int idSegment)
{
	Copy(pWavSet,idSegment); //-1 for all segments
}

bool WavSet::Copy(WavSet* pWavSet, int idSegment) //-1 for all segments
{
	SampleRate = pWavSet->SampleRate;
	totalFrames = pWavSet->totalFrames; 
	numChannels = pWavSet->numChannels;
	numSamples = pWavSet->numSamples;  
	numBytes = pWavSet->numBytes;
	SetName("");

	numFramesPerSegment = pWavSet->numFramesPerSegment;
	numSegments = pWavSet->numSegments;
	numSamplesPerSegment = pWavSet->numSamplesPerSegment;  
	numBytesPerSegment = pWavSet->numBytesPerSegment;


	idSegmentSelected = pWavSet->idSegmentSelected;

	midinote = pWavSet->midinote;
	patterncode = pWavSet->patterncode;

	if(numBytes!=0)
	{
		pSamples = (float*) malloc( numBytes );
		if( pSamples == NULL )
		{
			swprintf(pWCHAR, L"Could not allocate load buffer.\n");StatusAddText(pWCHAR);
			return false;
		}
		if(idSegment==-1)
		{
			//copy all segments
			memcpy(pSamples, pWavSet->pSamples, numBytes);
		}
		else if(idSegment<numSegments)
		{
			//initialize buffer with zeros
			memset(pSamples, 0, numBytes);
			//copy one segment only
			float* pBaseFloatDest = GetPointerToSegmentData(idSegment);
			float* pBaseFloatSrc = pWavSet->GetPointerToSegmentData(idSegment);
			memcpy(pBaseFloatDest, pBaseFloatSrc, numBytesPerSegment);
		}
		else
		{
			swprintf(pWCHAR, L"Could copy segment id %d\n", idSegment);StatusAddText(pWCHAR);
			if(pSamples) free(pSamples);
			return false;
		}
	}
	return true;
}

bool WavSet::Copy(WavSet* pWavSet, float duration_s, float offset_s)
{
	assert(pWavSet!=NULL && pSamples==NULL);
	int duration_frame=duration_s*pWavSet->SampleRate/(pWavSet->numChannels);
	int offset_frame=offset_s*pWavSet->SampleRate/(pWavSet->numChannels);
	return Copy(pWavSet, duration_frame, offset_frame);
}

bool WavSet::Copy(WavSet* pWavSet, int duration_frame, int offset_frame)
{
	assert(pWavSet!=NULL && pSamples==NULL);
	assert((duration_frame+offset_frame)<=pWavSet->totalFrames);

	SampleRate = pWavSet->SampleRate;
	totalFrames = duration_frame; 
	numChannels = pWavSet->numChannels;
	numSamples = totalFrames*numChannels;  
	numBytes = numSamples*sizeof(float);
	SetName("");

	numFramesPerSegment = -1;
	numSegments = -1;
	numSamplesPerSegment = -1;  
	numBytesPerSegment = -1;

	idSegmentSelected = -1;

	midinote = pWavSet->midinote;
	patterncode = pWavSet->patterncode;

	if(numBytes!=0)
	{
		pSamples = (float*) malloc( numBytes );
		if( pSamples == NULL )
		{
			swprintf(pWCHAR, L"error in wavset::copy(), could not allocate buffer.\n");StatusAddText(pWCHAR);
			return false;
		}
		memcpy(pSamples, &(pWavSet->pSamples[offset_frame*pWavSet->numChannels]), numBytes);
	}
	else
	{
		assert(false);
		return false;
	}
	return true;
}

bool WavSet::CopyNoMalloc(class WavSet* pWavSet, float duration_s, float src_offset_s, float dst_offset_s)
{
	int duration_frame=duration_s*pWavSet->SampleRate/(pWavSet->numChannels);
	int src_offset_frame=src_offset_s*pWavSet->SampleRate/(pWavSet->numChannels);
	int dst_offset_frame=dst_offset_s*pWavSet->SampleRate/(pWavSet->numChannels);
	return CopyNoMalloc(pWavSet, duration_frame, src_offset_frame, dst_offset_frame);
}

bool WavSet::CopyNoMalloc(class WavSet* pWavSet, int duration_frame, int src_offset_frame, int dst_offset_frame)
{
	assert(pWavSet!=NULL && pWavSet->pSamples!=NULL);
	assert((duration_frame+src_offset_frame)<=pWavSet->totalFrames);
	assert(pSamples!=NULL);
	assert((duration_frame+dst_offset_frame)<=totalFrames);
	assert(pWavSet->numChannels==numChannels);
	int numberofbytetocopy=duration_frame*numChannels*sizeof(float);
	memcpy(&(pSamples[dst_offset_frame*numChannels]), &(pWavSet->pSamples[src_offset_frame*numChannels]), numberofbytetocopy);
	return true;
}

WavSet::~WavSet()
{
	if(pSamples) free(pSamples);
}

bool WavSet::ReadWavFile(const char* filename)
{
	/////////////////////
	// open input file...
    /*
	WavInFile* pWavInFile = new WavInFile(filename);
	if(pWavInFile)
	*/
	SndfileHandle file;
	file = SndfileHandle(filename);
	if(1)
	{
		swprintf(pWCHAR, L"Begin reading file.\n"); fflush(stdout);StatusAddText(pWCHAR);
		SampleRate = file.samplerate(); //pWavInFile->getSampleRate();
		totalFrames = file.frames(); //pWavInFile->getNumSamples(); 
		numChannels = file.channels(); //pWavInFile->getNumChannels();
		numSamples = totalFrames * numChannels;  
		numBytes = numSamples * sizeof(float);
		assert(pSamples==NULL);
		pSamples = (float*) malloc( numBytes );
		if( pSamples == NULL )
		{
			/////////////////////////////////////////////////////////////
			//log currently used parameters into file (to ease debugging)
			/////////////////////////////////////////////////////////////
			if(1)
			{
				FILE* pFILE = fopen("debug.txt", "a");
				fprintf(pFILE, "ReadWavFile():\n");
				fprintf(pFILE, "totalFrames = %d\n", totalFrames);
				fprintf(pFILE, "numChannels = %d\n", numChannels);
				fprintf(pFILE, "numSamples = %d\n", numSamples);
				fprintf(pFILE, "numBytes = %d\n", numBytes);
				fprintf(pFILE, "Could not allocate load buffer, exiting ...\n");
				fclose(pFILE);
			}
			swprintf(pWCHAR, L"Could not allocate load buffer, exiting ...\n");StatusAddText(pWCHAR);
			return false;
		}
		//for(int i=0; i<numSamples; i++ ) pLoadedSamples[i] = 0;

		///////////////////////////////////////
		// read samples from the input file ...
		/*
		while (pWavInFile->eof() == 0)
		{
			// Read a chunk of samples from the input file
			int num = pWavInFile->read(pSamples, numSamples);
		}
		delete pWavInFile;
		*/
		file.read(pSamples, numSamples);

		swprintf(pWCHAR, L"Done!\n"); fflush(stdout);StatusAddText(pWCHAR);

		string name = GetName();
		if(name.compare("")==0)
		{
			SetName(filename);
		}
		return true;
	}
	return false;
}

bool WavSet::WriteWavFile(const char* filename)
{
	assert(filename);
    const int format=SF_FORMAT_WAV | SF_FORMAT_PCM_16;  
	//const int format=SF_FORMAT_WAV | SF_FORMAT_FLOAT;  
    //const int format=SF_FORMAT_WAV | SF_FORMAT_PCM_24;  
    //const int format=SF_FORMAT_WAV | SF_FORMAT_PCM_32;  
	SndfileHandle outfile(filename, SFM_WRITE, format, numChannels, SampleRate); 
	if(frameIndex==0)
	{
		outfile.write(pSamples, numSamples);  
	}
	else
	{
		assert(frameIndex<=totalFrames);
		outfile.write(pSamples, frameIndex*numChannels); 
	}
	string name = GetName();
	if(name.compare("")==0)
	{
		SetName(filename);
	}
	return true;
}

bool WavSet::AppendWavFile(const char* filename)
{
	assert(filename);
    const int format=SF_FORMAT_WAV | SF_FORMAT_PCM_16;  
	//const int format=SF_FORMAT_WAV | SF_FORMAT_FLOAT;  
    //const int format=SF_FORMAT_WAV | SF_FORMAT_PCM_24;  
    //const int format=SF_FORMAT_WAV | SF_FORMAT_PCM_32;  
	//SndfileHandle outfile(filename, SFM_WRITE, format, numChannels, SampleRate); 
	SndfileHandle outfile(filename, SFM_RDWR, format, numChannels, SampleRate); 
	outfile.seek(outfile.frames(), SEEK_SET);
	if(frameIndex==0)
	{
		outfile.write(pSamples, numSamples);  
	}
	else
	{
		assert(frameIndex<=totalFrames);
		outfile.write(pSamples, frameIndex*numChannels); 
	}
	string name = GetName();
	if(name.compare("")==0)
	{
		SetName(filename);
	}
	return true;
}

const char*  WavSet::GetName()
//string WavSet::GetName()
{
	//return (const char*) pCharName; //not OK
	//return "noname"; //OK
	//return strdup(pCharName);
	return wavsetname.c_str();
	//return wavsetname;
}

char tempname[WAVSET_CHARNAME_MAXLENGTH];
void WavSet::SetName(const char* name)
{
	//string namewithoutpath = name;
	//int found = namewithoutpath.rfind('\\');
	//strcpy_s(pCharName, WAVSET_CHARNAME_MAXLENGTH, name);
	//wavsetname = namewithoutpath;
	/*
	wavsetname = name;
	*/
	/*
	//let's force name to be uppercase so instrument can match wavset more rapidly
	strcpy_s(tempname, WAVSET_CHARNAME_MAXLENGTH-1, name); //was giving problem, must use strcpy_s()
	strupr(tempname);
	wavsetname = tempname;
	*/
	wavsetname = name;
}

int WavSet::GetMidiNote()
{
	return midinote;
}

void WavSet::SetMidiNote(int midinotenumber)
{
	midinote = midinotenumber;
}

int WavSet::GetPatternCode()
{
	return patterncode;
}

void WavSet::SetPatternCode(int code)
{
	patterncode = code;
}

bool WavSet::CreateSilence(float duration, int samplerate, int numchannel)
{
	assert(pSamples==NULL);
	if(pSamples!=NULL)
	{
		free(pSamples);
		Init();
	}
	SampleRate = samplerate; 
	totalFrames = SampleRate*duration; //duration is in seconds
	numChannels = numchannel; 
	numSamples = totalFrames * numChannels;  
	numBytes = numSamples * sizeof(float);
	assert(pSamples==NULL);
	pSamples = (float*) malloc( numBytes );
	if(pSamples!=NULL)
	{
		memset(pSamples, 0, numBytes);
		return true;
	}
	return false;
}

//amplitude * sin( ( (2*pi*hertz) / samplerate) * time)
bool WavSet::CreateSin(float duration, int samplerate, int numchannel, float frequency_hz, float amplitude)
{
	assert(pSamples==NULL);
	if(pSamples!=NULL)
	{
		free(pSamples);
		Init();
	}
	SampleRate = samplerate; 
	totalFrames = SampleRate*duration; //duration is in seconds
	numChannels = numchannel; 
	numSamples = totalFrames * numChannels;  
	numBytes = numSamples * sizeof(float);
	assert(pSamples==NULL);
	pSamples = (float*) malloc( numBytes );
	//todo:
	if(numChannels==1)
	{
		for (int i=0; i<numSamples; i++) pSamples[i]=amplitude*sin(float(i)/SampleRate*M_PI*2*frequency_hz);  
		//for (int i=0; i<numSamples; i++) pSamples[i]=amplitude*sin(float(i)/SampleRate*M_PI*frequency_hz);  
	}
	else if (numChannels==2)
	{
		for (int i=0; i<numSamples; i=i+2) 
		{
			//pSamples[i]=amplitude*sin(float(i)/SampleRate*M_PI*2*frequency_hz); 
			//pSamples[i+1]=amplitude*sin(float(i+1)/SampleRate*M_PI*2*frequency_hz);  
			pSamples[i]=amplitude*sin(float(i)/SampleRate*M_PI*frequency_hz); 
			pSamples[i+1]=amplitude*sin(float(i+1)/SampleRate*M_PI*frequency_hz);  
		}
	}
	else
	{
		assert(false);
		return false;
	}
	return true;
}

bool WavSet::CreateSquare(float duration, int samplerate, int numchannel, float frequency_hz, float amplitude)
{
	assert(pSamples==NULL);
	if(pSamples!=NULL)
	{
		free(pSamples);
		Init();
	}
	SampleRate = samplerate; 
	totalFrames = SampleRate*duration; //duration is in seconds
	numChannels = numchannel; 
	numSamples = totalFrames * numChannels;  
	numBytes = numSamples * sizeof(float);
	assert(pSamples==NULL);
	pSamples = (float*) malloc( numBytes );
	//todo:
	if(numChannels==1)
	{
		for (int i=0; i<numSamples; i++) 
		{
			float temp = sin(float(i)/SampleRate*M_PI*2*frequency_hz);
			//float temp = sin(float(i)/SampleRate*M_PI*frequency_hz);
			if(temp>0) pSamples[i]=amplitude*1.0;
			  else pSamples[i]=amplitude*-1.0;
		}
	}
	else if (numChannels==2)
	{
		for (int i=0; i<numSamples; i=i+2) 
		{
			//float temp = sin(float(i)/SampleRate*M_PI*2*frequency_hz);
			float temp = sin(float(i)/SampleRate*M_PI*frequency_hz);
			if(temp>0) pSamples[i]=amplitude*1.0;
			  else pSamples[i]=amplitude*-1.0;
			//temp = sin(float(i+1)/SampleRate*M_PI*2*frequency_hz);  
			temp = sin(float(i+1)/SampleRate*M_PI*frequency_hz);  
			if(temp>0) pSamples[i+1]=amplitude*1.0;
			  else pSamples[i+1]=amplitude*-1.0;
		}
	}
	else
	{
		assert(false);
		return false;
	}
	return true;
}

bool WavSet::CreateSaw(float duration, int samplerate, int numchannel, float frequency_hz, float amplitude)
{
	assert(pSamples==NULL);
	if(pSamples!=NULL)
	{
		free(pSamples);
		Init();
	}
	SampleRate = samplerate; 
	totalFrames = SampleRate*duration; //duration is in seconds
	numChannels = numchannel; 
	numSamples = totalFrames * numChannels;  
	numBytes = numSamples * sizeof(float);
	assert(pSamples==NULL);
	pSamples = (float*) malloc( numBytes );
	//todo:
	assert(frequency_hz>0);
	//int samplesperwavelength = SampleRate / (frequency_hz/numChannels);
	int samplesperwavelength = SampleRate / frequency_hz;
	float amplitudestep = 2*amplitude/samplesperwavelength;
	float amplitudetemp = -amplitude;
	if(numChannels==1)
	{
		for (int i=0; i<numSamples; i++) 
		{
			pSamples[i]=amplitudetemp; 
			if(abs(amplitudetemp)>amplitude) amplitudetemp = -amplitude; //reset amplitudetemp whenever it hits max amplitude boundary
			amplitudetemp += amplitudestep;
		}
	}
	else if (numChannels==2)
	{
		for (int i=0; i<numSamples; i=i+2) 
		{
			pSamples[i]=amplitudetemp; 
			pSamples[i+1]=amplitudetemp;  
			if(abs(amplitudetemp)>amplitude) amplitudetemp = -amplitude; //reset amplitudetemp whenever it hits max amplitude boundary
			amplitudetemp += amplitudestep;
		}
	}
	else
	{
		assert(false);
		return false;
	}
	return true;
}

bool WavSet::CreateTri(float duration, int samplerate, int numchannel, float frequency_hz, float amplitude)
{
	assert(pSamples==NULL);
	if(pSamples!=NULL)
	{
		free(pSamples);
		Init();
	}
	SampleRate = samplerate; 
	totalFrames = SampleRate*duration; //duration is in seconds
	numChannels = numchannel; 
	numSamples = totalFrames * numChannels;  
	numBytes = numSamples * sizeof(float);
	assert(pSamples==NULL);
	pSamples = (float*) malloc( numBytes );
	//todo:
	assert(frequency_hz>0);
	//int samplesperwavelength = SampleRate / (frequency_hz/numChannels);
	//int samplesperwavelength = SampleRate / (frequency_hz);
	//int samplesperwavelength = SampleRate / (numChannels*frequency_hz);
	int samplesperwavelength = SampleRate / (2*frequency_hz);
	float amplitudestep = 2*amplitude/samplesperwavelength;
	float amplitudetemp = -amplitude;
	if(numChannels==1)
	{
		for (int i=0; i<numSamples; i++) 
		{
			pSamples[i]=amplitudetemp; 
			if(abs(amplitudetemp)>amplitude) amplitudestep = -amplitudestep; //negate amplitudestep whenever it hits amplitude boundary
			amplitudetemp += amplitudestep;
		}
	}
	else if (numChannels==2)
	{
		for (int i=0; i<numSamples; i=i+2) 
		{
			pSamples[i]=amplitudetemp; 
			pSamples[i+1]=amplitudetemp;  
			if(abs(amplitudetemp)>amplitude) amplitudestep = -amplitudestep; //negate amplitudestep whenever it hits amplitude boundary
			amplitudetemp += amplitudestep;
		}
	}
	else
	{
		assert(false);
		return false;
	}
	return true;
}

float WavSet::SpreadSample(int numberofsample, class WavSet* pWavSet, float distance_s, float duration_s, float distanceoffset_s, float amplitude)
{
	assert(pWavSet);
	float totallengthinseconds = 0.0f;
	float offset_s;
	for(int i=0; i<numberofsample; i++)
	{
		offset_s = distanceoffset_s + duration_s/2 + i*(distance_s/numberofsample);
		Sum(amplitude, pWavSet, offset_s, duration_s);
		totallengthinseconds = offset_s + duration_s;
	}
	return totallengthinseconds;
}

float WavSet::SpreadSample(const char* patternedsample, WavSet* pWavSet, float distance_s, float duration_s, float distanceoffset_s, float amplitude)
{
	assert(patternedsample);
	assert(pWavSet);
	string pattern = patternedsample; //i.e. "1000110011001100"
	string present;
	int numberofsample = pattern.size();
	float totallengthinseconds = 0.0f;
	float offset_s;
	for(int i=0; i<numberofsample; i++)
	{
		present = pattern.substr(i,1);
		if(present=="1")
		{
			offset_s = distanceoffset_s + duration_s/2 + i*(distance_s/numberofsample);
			Sum(amplitude, pWavSet, offset_s, duration_s);
			totallengthinseconds = offset_s + duration_s;
		}
		else if(present!="0")
		{
			//SpreadSample() can only read pattern made out of "0" and "1"
			//you may consider using SpreadSamples() instead
			assert(false);
		}
	}
	return totallengthinseconds;
}

//2012xxx00, pattern like "9000220099002200" have not been debugged yet
//2013oct15, also supports binary pattern like "0101001101011011"
float WavSet::SpreadSamples(const char* patternedsample, Instrument* pInstrument, float distance_s, float duration_s, float distanceoffset_s, float amplitude)
{
	//todo: copy paste spreadsample function in here below
	//      this function takes non-binary pattern like "9000220099002200" where each symbol can take a value between 0-9 (todo: and even 0-Z eventually)
	assert(patternedsample);
	assert(pInstrument);
	string pattern = patternedsample; //i.e. "9000220099002200"
	string patterncode;
	int numberofsample = pattern.size();
	int patternrange = 9;
	float totallengthinseconds = 0.0f;

	//WavSet* pWavSet = pInstrument->GetWavSetRandomly();
	//SpreadSample(patternedsample, pWavSet, distance_s, duration_s, distanceoffset_s);
	float offset_s;
	for(int i=0; i<numberofsample; i++)
	{
		patterncode = pattern.substr(i,1);
		if(patterncode!="0" && patterncode!="1")
		{
			offset_s = distanceoffset_s + duration_s/2 + i*(distance_s/numberofsample);
			//WavSet* pWavSet = pInstrument->GetWavSetFromPatternCode(patterncode.c_str(), patternrange);
			WavSet* pWavSet = pInstrument->GetWavSetFromPatternCode(patterncode.c_str());
			Sum(amplitude, pWavSet, offset_s, duration_s);
			totallengthinseconds = offset_s + duration_s;
		}
		else if(patterncode!="0")
		{
			offset_s = distanceoffset_s + duration_s/2 + i*(distance_s/numberofsample);
			//WavSet* pWavSet = pInstrument->GetWavSetFromPatternCode(patterncode.c_str(), patternrange);
			WavSet* pWavSet = pInstrument->GetWavSetRandomly();
			Sum(amplitude, pWavSet, offset_s, duration_s);
			totallengthinseconds = offset_s + duration_s;
		}
	}
	return totallengthinseconds;
}

//distance_s is the length of the loop in seconds
//distance_s < 0 will default into duration_s
//duration_s < 0 will default into supplied sample length
float WavSet::LoopSample(WavSet* pWavSet, float distance_s, float duration_s, float distanceoffset_s)
{
	assert(pWavSet);
	if(duration_s<0) duration_s = pWavSet->GetWavSetLength();
	if(distance_s<0) distance_s = pWavSet->GetWavSetLength();
	//2013oct12, poirier, begin
	int numberofsample = distance_s/duration_s; //number of repetition
	float fremainder_s = (distance_s/duration_s - numberofsample)*duration_s;
	//int numberofsample = ceil(distance_s/duration_s); //number of repetition
	//2013oct12, poirier, end
	float totallengthinseconds = 0.0f;
	float offset_s;
	int i;
	for(i=0; i<numberofsample; i++)
	{
		//offset_s = distanceoffset_s + duration_s/2 + i*(distance_s/numberofsample);
		offset_s = distanceoffset_s + i*(distance_s/numberofsample);
		Sum(1.0, pWavSet, offset_s, duration_s);
		totallengthinseconds = offset_s + duration_s;
	}
	//2013oct12, poirier, begin
	/*
	if(fremainder_s>0.0f)
	{
		i++;
		offset_s = distanceoffset_s + i*(distance_s/numberofsample);
		Sum(1.0, pWavSet, offset_s, fremainder_s);
		totallengthinseconds = offset_s + fremainder_s;
	}
	*/
	//2013oct12, poirier, end
	return totallengthinseconds;
}

bool WavSet::Concatenate(class WavSet* pWavSet)
{
	if(pWavSet==NULL || pWavSet->pSamples==NULL) 
	{
		swprintf(pWCHAR, L"error in wavset::concatenate\n");StatusAddText(pWCHAR);
		assert(false);
		return false;
	}
	if(pSamples!=NULL)
	{
		assert(numChannels==pWavSet->numChannels);
		assert(SampleRate==pWavSet->SampleRate);
		pSamples = (float* )realloc(pSamples, numBytes+pWavSet->numBytes);
		memcpy(&(pSamples[numSamples]), pWavSet->pSamples, pWavSet->numBytes);
		totalFrames=totalFrames+pWavSet->totalFrames;
		numSamples=numSamples+pWavSet->numSamples;
		numBytes=numBytes+pWavSet->numBytes;
	}
	else
	{
		Copy(pWavSet);
	}
	return true;
}


bool WavSet::Sum(float amplitude, WavSet* pWavSet2, float offset_s, float duration_s)
{
	assert(pWavSet2);
	//assert(0.0f<=amplitude && amplitude<=1.0f);
	assert(0.0f<=amplitude && amplitude<=10.0f);
	assert(numChannels==pWavSet2->numChannels);
	assert(SampleRate==pWavSet2->SampleRate);

	float length_s = GetWavSetLength();
	float length2_s = pWavSet2->GetWavSetLength();
	if(length_s>offset_s)
	{
		//find overlapping area
		float offsetstart_s = offset_s;
		float offsetend_s = offsetstart_s + duration_s;
		if(length2_s<duration_s)
		{
			offsetend_s = offsetstart_s + length2_s;
		}
		if(offsetend_s>length_s)
		{
			offsetend_s = length_s;
		}
		/*
		//convert into frames
		int offsetstart_frames = offsetstart_s*SampleRate/numChannels;
		int offsetend_frames = offsetend_s*SampleRate/numChannels;
		//sum
		if(numChannels==1)
		{
			for (int i=offsetstart_frames; i<offsetend_frames; i++) 
			{
				pSamples[i]=pSamples[i]+amplitude*pWavSet2->pSamples[i-offsetstart_frames];  
			}
		}
		else if (numChannels==2)
		{
			for (int i=offsetstart_frames; i<offsetend_frames; i=i+2) 
			{
				pSamples[i]=pSamples[i]+amplitude*pWavSet2->pSamples[i-offsetstart_frames];  
				pSamples[i+1]=pSamples[i+1]+amplitude*pWavSet2->pSamples[i-offsetstart_frames];  
			}
		}
		else
		{
			assert(false);
			return false;
		}
		*/
		//convert into samples
		int offsetstart_samples = offsetstart_s*SampleRate*numChannels;
		int offsetend_samples = offsetend_s*SampleRate*numChannels;
		//sum
		if(numChannels==1)
		{
			for (int i=offsetstart_samples; i<offsetend_samples; i++) 
			{
				pSamples[i]=pSamples[i]+amplitude*pWavSet2->pSamples[i-offsetstart_samples];  
			}
		}
		else if (numChannels==2)
		{
			for (int i=offsetstart_samples; i<offsetend_samples; i=i+2) 
			{
				pSamples[i]=pSamples[i]+amplitude*pWavSet2->pSamples[i-offsetstart_samples];  
				pSamples[i+1]=pSamples[i+1]+amplitude*pWavSet2->pSamples[i-offsetstart_samples];  
			}
		}
		else
		{
			assert(false);
			return false;
		}

	}
	else
	{
		//no need to sum, return safely
	}
	return true;
}

bool WavSet::Mix(float amplitude1, WavSet* pWavSet1, float amplitude2, WavSet* pWavSet2)
{
	if(pWavSet1==NULL || pWavSet1->pSamples==NULL || pWavSet1->numBytes==0) 
	{
		swprintf(pWCHAR, L"error in wavset::mix, pWavSet1==NULL || pWavSet1->pSamples==NULL\n");StatusAddText(pWCHAR);
		assert(false);
		return false;
	}
	if(pWavSet2==NULL || pWavSet2->pSamples==NULL || pWavSet2->numBytes==0) 
	{
		swprintf(pWCHAR, L"error in wavset::mix, pWavSet2==NULL || pWavSet2->pSamples==NULL\n");StatusAddText(pWCHAR);
		assert(false);
		return false;
	}
	if(pSamples!=NULL)
	{
		swprintf(pWCHAR, L"error in wavset::mix, attempting to mix into previously allocated buffer\n");StatusAddText(pWCHAR);
		assert(false);
		return false;
	}
	assert(pWavSet1->numChannels==pWavSet2->numChannels);
	assert(pWavSet1->SampleRate==pWavSet2->SampleRate);
	SampleRate=pWavSet1->SampleRate;
	numChannels=pWavSet1->numChannels;
	int numsamplesdifference=0;
	if(pWavSet1->numBytes>=pWavSet2->numBytes)
	{
		totalFrames=pWavSet1->totalFrames;
		numSamples=pWavSet1->numSamples;
		numBytes=pWavSet1->numBytes;
		numsamplesdifference=pWavSet1->numSamples-pWavSet2->numSamples;
	}
	else
	{
		totalFrames=pWavSet2->totalFrames;
		numSamples=pWavSet2->numSamples;
		numBytes=pWavSet2->numBytes;
		numsamplesdifference=pWavSet2->numSamples-pWavSet1->numSamples;
	}
	pSamples = (float*)malloc(numBytes);if(pSamples==NULL) {swprintf(pWCHAR, L"error in wavset::mix, not enough memory.\n");StatusAddText(pWCHAR);return false;}
	if(numChannels==1)
	{
		for (int i=0; i<(numSamples-numsamplesdifference); i++) 
		{
			pSamples[i]=amplitude1*pWavSet1->pSamples[i]+amplitude2*pWavSet2->pSamples[i];  
		}
	}
	else if (numChannels==2)
	{
		for (int i=0; i<(numSamples-numsamplesdifference); i=i+2) 
		{
			pSamples[i]=amplitude1*pWavSet1->pSamples[i]+amplitude2*pWavSet2->pSamples[i];  
			pSamples[i+1]=amplitude1*pWavSet1->pSamples[i+1]+amplitude2*pWavSet2->pSamples[i];  
		}
	}
	else
	{
		assert(false);
		return false;
	}
	if(pWavSet1->numBytes>pWavSet2->numBytes)
	{
		if(numChannels==1)
		{
			for (int i=(numSamples-numsamplesdifference); i<numSamples; i++) 
			{
				pSamples[i]=amplitude1*pWavSet1->pSamples[i];  
			}
		}
		else if (numChannels==2)
		{
			for (int i=(numSamples-numsamplesdifference); i<numSamples; i=i+2) 
			{
				pSamples[i]=amplitude1*pWavSet1->pSamples[i];  
				pSamples[i+1]=amplitude1*pWavSet1->pSamples[i+1];  
			}
		}
	}
	else
	{
		if(numChannels==1)
		{
			for (int i=(numSamples-numsamplesdifference); i<numSamples; i++) 
			{
				pSamples[i]=amplitude2*pWavSet2->pSamples[i];  
			}
		}
		else if (numChannels==2)
		{
			for (int i=(numSamples-numsamplesdifference); i<numSamples; i=i+2) 
			{
				pSamples[i]=amplitude2*pWavSet2->pSamples[i];  
				pSamples[i+1]=amplitude2*pWavSet2->pSamples[i+1];  
			}
		}
	}
	return true;
}

bool WavSet::SplitInSegments(double fSecondsPerSegment)
{
	numFramesPerSegment = fSecondsPerSegment * SampleRate;
	if(numFramesPerSegment>totalFrames || fSecondsPerSegment==0.0) 
	{
		/////////////////////////////////////////////////////////////
		//log currently used parameters into file (to ease debugging)
		/////////////////////////////////////////////////////////////
		if(1)
		{
			FILE* pFILE = fopen("debug.txt", "a");
			fprintf(pFILE, "SplitInSegments():\n");
			fprintf(pFILE, "fSecondsPerSegment = %f\n", fSecondsPerSegment);
			fprintf(pFILE, "SampleRate = %d\n", SampleRate);
			fprintf(pFILE, "numFramesPerSegment = %d\n", numFramesPerSegment);
			fprintf(pFILE, "totalFrames = %d\n", totalFrames);
			fprintf(pFILE, "(numFramesPerSegment>totalFrames || fSecondsPerSegment==0.0)==1, warning ...\n");
			fclose(pFILE);
		}
		swprintf(pWCHAR, L"(numFramesPerSegment>totalFrames || fSecondsPerSegment==0.0)==1, warning ...\n");StatusAddText(pWCHAR);
		//return false;
		//assert(false);
		numFramesPerSegment=totalFrames;
	}
	numSegments = totalFrames/numFramesPerSegment;
	numSamplesPerSegment = numFramesPerSegment * numChannels;  
	numBytesPerSegment = numSamplesPerSegment * sizeof(float);
	return true;
}

float* WavSet::GetPointerToSegmentData(int idSegment)
{
	if(pSamples==NULL || numSegments==-1 || idSegment>(numSegments-1)) 
	{
		assert(false);
		return NULL; //error
	}
	return pSamples+idSegment*numSamplesPerSegment;
}

bool WavSet::Erase()
{
	assert(numChannels==2);
	for(int i=0; i<totalFrames; i++ )
    {
        *(pSamples+2*i) = 0;  /* left */
        *(pSamples+2*i+1) = 0;  /* right */
    }
    return 0;

	return true;
}

bool WavSet::EraseSegment(int idSegment)
{
	assert(numChannels==2);
	for(int i=0; i<numFramesPerSegment; i++ )
    {
        *(GetPointerToSegmentData(idSegment)+2*i) = 0;  /* left */
        *(GetPointerToSegmentData(idSegment)+2*i+1) = 0;  /* right */
    }
    return 0;

	return true;
}

bool WavSet::FadeSegmentEdges(int idSegment)
{
	//numFramesPerEdge is half of the total number of frames to fade
	//int numFramesPerEdge = numFramesPerSegment/4; //pretty much the largest value
	//int numFramesPerEdge = 100; //pretty much the smallest value
	int numFramesPerEdge = numFramesPerSegment/16; 

	//if(numFramesPerSegment<(2*100)) 
	if(numFramesPerSegment<(2*numFramesPerEdge)) 
	{
		/////////////////////////////////////////////////////////////
		//log currently used parameters into file (to ease debugging)
		/////////////////////////////////////////////////////////////
		if(1)
		{
			FILE* pFILE = fopen("debug.txt", "a");
			fprintf(pFILE, "FadeSegmentEdges():\n");
			fprintf(pFILE, "idSegment = %d\n", idSegment);
			fprintf(pFILE, "numFramesPerSegment = %d\n", numFramesPerSegment);
			fprintf(pFILE, "numFramesPerEdge = %d\n", numFramesPerEdge);
			fprintf(pFILE, "Not enough frames for FadeSegmentEdges().\n");
			fclose(pFILE);
		}
		swprintf(pWCHAR, L"Not enough frames for FadeSegmentEdges().\n");StatusAddText(pWCHAR);
		return false;
	}
	float a,aa;
	int ii;
	assert(numChannels==2);
	//for(int i=0; i<100; i++ )
	for(int i=0; i<numFramesPerEdge; i++ )
    {
		//starting edge
		//a = i/100.0f;
		a = i/(1.0f*numFramesPerEdge); //a has values between 0.0 and 1.0, fade in factor
        *(GetPointerToSegmentData(idSegment)+2*i) = a*(*(GetPointerToSegmentData(idSegment)+2*i));  /* left */
        *(GetPointerToSegmentData(idSegment)+2*i+1) = a*(*(GetPointerToSegmentData(idSegment)+2*i+1));  /* right */

		//ending edge
		aa = 1-a; //aa has values between 1.0 and 0.0, fade out factor
		//ii = numFramesPerSegment-100+i;
		ii = numFramesPerSegment-numFramesPerEdge+i;
        *(GetPointerToSegmentData(idSegment)+2*ii) = aa*(*(GetPointerToSegmentData(idSegment)+2*ii));  /* left */
        *(GetPointerToSegmentData(idSegment)+2*ii+1) = aa*(*(GetPointerToSegmentData(idSegment)+2*ii+1));  /* right */
    }
	return true;
}

float WavSet::GetSegmentsLength()
{
	return (numFramesPerSegment*numSegments/(1.0*SampleRate));
}

float WavSet::GetWavSetLength()
{
	if(totalFrames==0) return 0.0f;
	assert(SampleRate!=0);
	return (totalFrames/(1.0*SampleRate));
}

//2012june10, spi, added ReadWavFileHeader() for spinavwavfolders_sspp 
//				in order to replace lib soundtouch by class wavset 
//				which will now depend on libsndfile
bool WavSet::ReadWavFileHeader(const char* filename)
{
	assert(pSamples==NULL);
	assert(numBytes==0);
	SndfileHandle file;
	file = SndfileHandle(filename);
	if(1)
	{
		SampleRate = file.samplerate(); //pWavInFile->getSampleRate();
		totalFrames = file.frames(); //pWavInFile->getNumSamples(); 
		numChannels = file.channels(); //pWavInFile->getNumChannels();
		numSamples = totalFrames * numChannels;  
		//do not allocate in here, use ReadWavFile() to allocate and read the whole file
		numBytes = 0; //numSamples * sizeof(float);
		
	}
	return true;
}

bool WavSet::Play(PaStreamParameters* pPaStreamOutputParameters, float maxduration_s)
{
	bool result = false;
	WavSet* pWavSet = new WavSet();
	if(pWavSet)
	{
		result = pWavSet->Copy(this, maxduration_s, 0.0f);
		if(result)
		{
			result = pWavSet->Play(pPaStreamOutputParameters);
		}
		delete pWavSet;
	}
	return result;
}

bool WavSet::Play(PaStreamParameters* pPaStreamOutputParameters)
{
	assert(pPaStreamOutputParameters!=NULL);
    PaStream* pPaStream=NULL;
    PaError err;
	/////////////////////////////////////
	// play wavset as is using port audio 
	/////////////////////////////////////
	sprintf(pCHAR, "Begin playback of %s...\n", GetName()); fflush(stdout);StatusAddText(pCHAR);
	err = Pa_OpenStream(
				&pPaStream, //&stream
				NULL, // no input
				pPaStreamOutputParameters, //&outputParameters,
				SampleRate, //pWavSet->SampleRate,
				WAVSET_PLAY_BUFFERSIZE/numChannels, //pWavSet->numChannels,
				paClipOff,      // we won't output out of range samples so don't bother clipping them 
				NULL, // no callback, use blocking API 
				NULL ); // no callback, so no callback userData 
	if( err != paNoError ) goto error;

	if(pPaStream) //if(stream)
	{
		err = Pa_StartStream(pPaStream); //stream
		if( err != paNoError ) goto error;
		swprintf(pWCHAR, L"Waiting for playback to finish.\n"); fflush(stdout);StatusAddText(pWCHAR);

		err = Pa_WriteStream(pPaStream, pSamples, totalFrames); //stream, pWavSet->pSamples, pWavSet->totalFrames
		if( err != paNoError ) goto error;

		err = Pa_CloseStream(pPaStream); //stream
		if( err != paNoError ) goto error;
		swprintf(pWCHAR, L"Done.\n"); fflush(stdout);StatusAddText(pWCHAR);
	}

	return true;
error:
    Pa_Terminate();
    swprintf(pWCHAR, L"wavset::play(), an error occured while using the portaudio stream\n" );StatusAddText(pWCHAR);
    swprintf(pWCHAR, L"Error number: %d\n", err );StatusAddText(pWCHAR);
    sprintf(pCHAR, "Error message: %s\n", Pa_GetErrorText( err ) );StatusAddText(pCHAR);
    return false;
}

//warning: wavset name better be corresponding to a filename already written to disk
bool WavSet::Play(int playerflag, float numberofsecondsinplayback)
{
	assert(playerflag==USING_SOX || playerflag==USING_SPIPLAY || playerflag==USING_SPISPECTRUMPLAY || 
		   playerflag==USING_SPIPLAYSTREAM || playerflag==USING_SPIPLAYX);
	char charBuffer2[WAVSET_CHARNAME_MAXLENGTH];
	char charBuffer3[WAVSET_CHARNAME_MAXLENGTH];
	char charBuffer5[10]={"/b"}; //release runs start with /b option which stands for background
	char charBuffer6[10]={"-q"}; //release runs sox.exe with -q option which stands for quiet
	int nShowCmd = 0;
	#ifdef _DEBUG
			sprintf_s(charBuffer5,10-1,""); //debug runs start without /b option which stands for background
			sprintf_s(charBuffer6,10-1,""); //debug runs sox.exe without -q option which stands for quiet
			nShowCmd = 1;
	#endif //_DEBUG
	string filename = wavsetname; //wavset name better be corresponding to a filename already written to disk
	if(playerflag==USING_SOX)
	{
		////////////////////////////////////////
		// play partition wavset as is using sox 
		////////////////////////////////////////
		//int numRepeat = numberofsecondsinplayback/pWavSet->GetWavSetLength();
		int numRepeat = 1;
		//sprintf_s(charBuffer3,WAVSET_CHARNAME_MAXLENGTH-1,"start %s c:\\app-bin\\sox\\sox.exe %s \"%s\" -d trim 0 %f",charBuffer5,charBuffer6,filename.c_str(),numberofsecondsinplayback);
		//system(charBuffer3);
		sprintf_s(charBuffer3,WAVSET_CHARNAME_MAXLENGTH-1,"%s \"%s\" -d trim 0 %f", charBuffer6, filename.c_str(), numberofsecondsinplayback);
		ShellExecuteA(NULL, "open", "c:\\app-bin\\sox\\sox.exe", charBuffer3, NULL, nShowCmd);
	}
	else if(playerflag==USING_SPIPLAY)
	{
		////////////////////////////////////////////
		// play partition wavset as is using spiplay 
		////////////////////////////////////////////
		//sprintf_s(charBuffer3,WAVSET_CHARNAME_MAXLENGTH-1,"start %s c:\\app-bin\\spiplay\\spiplay.exe \"%s\" %f",charBuffer5,filename.c_str(),numberofsecondsinplayback);
		//system(charBuffer3);
		sprintf_s(charBuffer3,WAVSET_CHARNAME_MAXLENGTH-1,"\"%s\" %f", filename.c_str(), numberofsecondsinplayback);
		ShellExecuteA(NULL, "open", "c:\\app-bin\\spiplay\\spiplay.exe", charBuffer3, NULL, nShowCmd);
	}
	else if(playerflag==USING_SPISPECTRUMPLAY)
	{
		////////////////////////////////////////////////////
		// play partition wavset as is using spispectrumplay 
		////////////////////////////////////////////////////
		//sprintf_s(charBuffer3,WAVSET_CHARNAME_MAXLENGTH-1,"start %s c:\\app-bin\\spispectrumplay_asio\\spispectrumplay_asio.exe \"%s\" %f 200 200 0 \"E-MU ASIO\" 6 7",charBuffer5,filename.c_str(),numberofsecondsinplayback);
		//system(charBuffer3);
		sprintf_s(charBuffer3,WAVSET_CHARNAME_MAXLENGTH-1,"\"%s\" %f 200 200 0 \"E-MU ASIO\" 6 7", filename.c_str(), numberofsecondsinplayback);
		ShellExecuteA(NULL, "open", "c:\\app-bin\\spispectrumplay_asio\\spispectrumplay_asio.exe", charBuffer3, NULL, nShowCmd);
	}
	else if(playerflag==USING_SPIPLAYSTREAM)
	{
		//////////////////////////////////////////////////
		// play partition wavset as is using spiplaystream 
		//////////////////////////////////////////////////
		//sprintf_s(charBuffer3,WAVSET_CHARNAME_MAXLENGTH-1,"start %s c:\\app-bin\\spiplaystream_asio\\spiplaystream_asio.exe \"%s\" %f \"E-MU ASIO\" 6 7",charBuffer5,filename.c_str(),numberofsecondsinplayback);
		//system(charBuffer3);
		sprintf_s(charBuffer3,WAVSET_CHARNAME_MAXLENGTH-1,"\"%s\" %f \"E-MU ASIO\" 6 7", filename.c_str(), numberofsecondsinplayback);
		ShellExecuteA(NULL, "open", "c:\\app-bin\\spiplaystream_asio\\spiplaystream_asio.exe", charBuffer3, NULL, nShowCmd);
	}
	else if(playerflag==USING_SPIPLAYX)
	{
		/////////////////////////////////////////////
		// play partition wavset as is using spiplayx 
		/////////////////////////////////////////////
		//sprintf_s(charBuffer3,WAVSET_CHARNAME_MAXLENGTH-1,"start %s c:\\app-bin\\spiplay\\spiplay.exe \"%s\" %f",charBuffer5,filename.c_str(),numberofsecondsinplayback);
		//system(charBuffer3);
		sprintf_s(charBuffer3,WAVSET_CHARNAME_MAXLENGTH-1,"\"%s\" %f", filename.c_str(), numberofsecondsinplayback);
		ShellExecuteA(NULL, "open", "c:\\app-bin\\spiplayx\\spiplayx.exe", charBuffer3, NULL, nShowCmd);
	}
	else
	{
		assert(false);
		return false;
	}

	return true;
}

//without really resampling, , todo check if generating glitch
bool WavSet::Resample44100monoTo44100stereo()
{
	if(numChannels==1 && SampleRate==44100)
	{
		//todo: test this assumption here below

		//spi note: this simple modif may work
		//half of the sample will become the left channel
		//while the other half will become the right channel
		//the wavset sample better be "channel interleaf",
		//this ensures both channel will be nearly the same
		numChannels=2;
		assert(totalFrames==numSamples); //was mono
		if(numSamples%2==0)
		{
			//numSamples better be even
			totalFrames = totalFrames/2;
		}
		else
		{
			//drop one sample
			numSamples = numSamples -1;
			totalFrames = numSamples/2;
		}
		return true;
	}
	return false;
}

//without really resampling, todo check if generating glitch
bool WavSet::Resample48000monoTo44100mono()
{
	if(numChannels==1 && SampleRate==48000)
	{
		//1) 48000 mono to 44100 mono
		assert(numSamples==totalFrames);
		SampleRate = 44100; //it will shift frequencies, but hey!
		return true;
	}
	return false;
}

bool WavSet::Resample48000monoTo44100stereo()
{
	bool result = false;
	if(numChannels==1 && SampleRate==48000)
	{
		//1) 48000 mono to 44100 mono
		result = Resample48000monoTo44100mono();
		if(result==false) 
		{
			return false;
		}
		//2) 44100 mono to 44100 stereo
		return Resample44100monoTo44100stereo();
	}
	return false;
}

//without really resampling, todo check if generating glitch
bool WavSet::Resample48000stereoTo44100stereo()
{
	if(numChannels==2 && SampleRate==48000)
	{
		//1) 48000 mono to 44100 mono
		SampleRate = 44100; //it will shift frequencies, but hey!
		return true;
	}
	return false;
}

bool WavSet::OpenStream(PaStreamParameters* pPaStreamInputParameters, PaStreamParameters* pPaStreamOutputParameters, PaStreamCallback* pPaStreamCallback)
{
	assert(pPaStream==NULL);	
	//1) open stream
	PaError myPaError = Pa_OpenStream(
						&pPaStream,
						pPaStreamInputParameters, //NULL, // no input
						pPaStreamOutputParameters, //&global_PaStreamParametersOUTPUT,
						SampleRate, //pWavSet->SampleRate,
						numSamplesPerSegment/numChannels, //pWavSet->numSamplesPerSegment/pWavSet->numChannels, //FRAMES_PER_BUFFER,
						paClipOff,      // we won't output out of range samples so don't bother clipping them 
						pPaStreamCallback, //patestCallback, // no callback, use blocking API 
						this); //pWavSet ); // no callback, so no callback userData
	if(myPaError==paNoError)return true;
	assert(false);
	return false;
}

bool WavSet::CloseStream()
{
	if(pPaStream) 
	{
		PaError myPaError = Pa_CloseStream(pPaStream);
		if(myPaError==paNoError)return true;
		assert(false);
		return false;
	}
	else 
	{
		assert(false);
		return false;
	}
}
