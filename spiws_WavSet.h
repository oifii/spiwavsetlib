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
//2012june12, spi, added Play()
//
//2012june15, spi, added support for reading midi file smf format 1
//				wavset library has been modified all over and function
//				Sum() to combine WavSet without any memory reallocation
//
//2012june16, spi, added support for spreading and looping samples in time
//				with family of functions SpreadSample() and LoopSample()
//
//
//nakedsoftware.org, spi@oifii.org or stephane.poirier@oifii.org
////////////////////////////////////////////////////////////////
#ifndef _WAVSET_H
#define _WAVSET_H

#define USING_SOX				1
#define USING_SPIPLAY			2
#define USING_SPISPECTRUMPLAY	3
#define USING_SPIPLAYSTREAM		4
#define USING_SPIPLAYX			5

#define WAVSET_PLAY_BUFFERSIZE		2048
#define WAVSET_CHARNAME_MAXLENGTH	2048

class SPIWAVSETLIB_API WavSet
{
public:
	int SampleRate;
	int totalFrames; 
	int numChannels;
	int numSamples;  
	int numBytes;
	float* pSamples;
	//char pCharName[WAVSET_CHARNAME_MAXLENGTH];
	std::string wavsetname;

	int numFramesPerSegment;
	int numSegments;
	int numSamplesPerSegment;  
	int numBytesPerSegment;

	int idSegmentSelected;
	
	int midinote; //0-127
	int patterncode; //ascii 0-255

	bool fadein;
	bool fadeout;
	PaStream* pPaStream; 

	int frameIndex;

	WavSet();
	void Init();
	WavSet(int samplerate, int numchannels, int totalframes, float* psamples);
	WavSet(class WavSet* pWavSet, int idSegment=-1);
	bool Copy(class WavSet* pWavSet, int idSegment=-1);
	bool Copy(class WavSet* pWavSet, float duration_s, float offset_s);
	bool Copy(class WavSet* pWavSet, int duration_frame, int offset_frame);
	bool CopyNoMalloc(class WavSet* pWavSet, float duration_s, float src_offset_s, float dst_offset_s);
	bool CopyNoMalloc(class WavSet* pWavSet, int duration_frame, int src_offset_frame, int dst_offset_frame);
	~WavSet();
	//virtual ~WavSet();
	bool ReadWavFile(const char* filename);
	bool WriteWavFile(const char* filename);
	bool AppendWavFile(const char* filename);
	const char* GetName();
	//std::string GetName();
	void SetName(const char* name);
	int GetMidiNote();
	void SetMidiNote(int midinotenumber);
	int GetPatternCode();
	void SetPatternCode(int code);
	bool SplitInSegments(double fSecondsPerSegment);
	float* GetPointerToSegmentData(int idSegment);
	bool Erase();
	bool EraseSegment(int idSegment);
	bool FadeSegmentEdges(int idSegment);
	float GetSegmentsLength();
	float GetWavSetLength();
	bool ReadWavFileHeader(const char* filename);
	bool CreateSilence(float duration, int samplerate=44100, int numchannel=2);
	bool CreateSin(float duration, int samplerate=44100, int numchannel=2, float frequency_hz=440.0f, float amplitude=1.0f);
	bool CreateSquare(float duration, int samplerate=44100, int numchannel=2, float frequency_hz=440.0f, float amplitude=1.0f);
	bool CreateSaw(float duration, int samplerate=44100, int numchannel=2, float frequency_hz=440.0f, float amplitude=1.0f);
	bool CreateTri(float duration, int samplerate=44100, int numchannel=2, float frequency_hz=440.0f, float amplitude=1.0f);
	float SpreadSample(int numberofsample, class WavSet* pWavSet, float distance_s=10.0f, float duration_s=1.0f, float distanceoffset_s=0.0f, float amplitude=1.0f);
	float SpreadSample(const char* patternedsample, class WavSet* pWavSet, float distance_s=10.0f, float duration_s=1.0f, float distanceoffset_s=0.0f, float amplitude=1.0f);
	float SpreadSamples(const char* patternedsample, class Instrument* pWavSet, float distance_s=10.0f, float duration_s=1.0f, float distanceoffset_s=0.0f, float amplitude=1.0f);
	float LoopSample(class WavSet* pWavSet, float distance_s=-1.0f, float duration_s=-1.0f, float distanceoffset_s=0.0f);
	bool Concatenate(class WavSet* pWavSet);
	bool Mix(float amplitude1, class WavSet* pWavSet1, float amplitude2, class WavSet* pWavSet2);
	bool Sum(float amplitude, class WavSet* pWavSet, float offset_s, float duration_s);
	bool Play(struct PaStreamParameters* pPaStreamOutputParameters, float maxduration_s);
	bool Play(struct PaStreamParameters* pPaStreamOutputParameters);
	bool Play(int usingflag, float numberofsecondsinplayback=-1.0);
	bool Resample44100monoTo44100stereo();
	bool Resample48000monoTo44100mono();
	bool Resample48000monoTo44100stereo();
	bool Resample48000stereoTo44100stereo();
	bool OpenStream(PaStreamParameters* pPaStreamInputParameters, PaStreamParameters* pPaStreamOutputParameters, PaStreamCallback* pPaStreamCallback);
	bool CloseStream();

	bool GetLeftChannel(class WavSet* pWavSet);
	bool GetRightChannel(class WavSet* pWavSet);
	bool SetLeftAndRightChannels(class WavSet* pLeftWavSet, class WavSet* pRightWavSet);
};


#endif //_WAVSET_H