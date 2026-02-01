# WavSet: Audio Buffer and Operations – Creating and Loading Audio

WavSet is the core class for representing in-memory audio data (buffers) and provides high-level operations to **load**, **generate**, and **duplicate** WAV samples. This section covers how to obtain and initialize audio content in a WavSet, from file I/O with libsndfile to programmatic waveform generation and efficient copying.

## 1. Loading WAV Files

WavSet uses libsndfile to read metadata or full sample data from disk. These methods are essential for ingesting external audio content.

### ReadWavFile

Loads the entire WAV file into memory, decoding samples as 32-bit floats.

- **Signature**

```cpp
  bool ReadWavFile(const char* filename);
```

- **Behavior**
- Opens `filename` via `SndfileHandle`.
- Reads `samplerate`, `channels`, and `frames`.
- Allocates `pSamples = malloc(numSamples * sizeof(float))`.
- Populates buffer with `file.read(pSamples, numSamples)`.
- **Use case**

Perfect for playback, mixing, or analysis when full sample data is needed.

```cpp
WavSet ws;
if (!ws.ReadWavFile("kick.wav")) {
  // Handle allocation or file errors
}
```

### ReadWavFileHeader

Reads only header information (sample rate, channel count, total frames) without loading sample data. Ideal for quick inspection or pre-allocation.

- **Signature**

```cpp
  bool ReadWavFileHeader(const char* filename);
```

- **Behavior**
- Opens `filename` via `SndfileHandle`.
- Extracts `samplerate`, `channels`, and `frames` into WavSet fields.
- Leaves `pSamples` at `NULL`.
- **Use case**

Validate file format or reserve memory before full load.

```cpp
WavSet ws;
if (ws.ReadWavFileHeader("long_track.wav")) {
  int sr = ws.SampleRate;
  int ch = ws.numChannels;
  // Decide if you need full load
}
```

---

## 2. Generating Audio Buffers 🎵

WavSet can synthesize basic periodic waveforms or silence. These functions allocate and populate `pSamples` for custom-built audio.

| Method | Description | Defaults |
| --- | --- | --- |
| **CreateSilence** | Allocate silent audio of given duration and format. | `samplerate=44100, numchannel=2` |
| **CreateSin** | Generate a sine wave at frequency and amplitude. | `hz=440.0f, amp=1.0f` |
| **CreateSquare** | Generate a square wave at frequency and amplitude. | `hz=440.0f, amp=1.0f` |
| **CreateSaw** | Generate a sawtooth wave at frequency and amplitude. | `hz=440.0f, amp=1.0f` |
| **CreateTri** | Generate a triangle wave at frequency and amplitude. | `hz=440.0f, amp=1.0f` |


All methods have signature overloads taking:

```cpp
bool CreateX(
  float duration,
  int samplerate = 44100,
  int numchannel = 2,
  float frequency_hz = 440.0f,   // for periodic waves
  float amplitude    = 1.0f      // for periodic waves
);
```

### CreateSilence

Allocates and zero-fills `pSamples`.

```cpp
bool WavSet::CreateSilence(
  float duration,
  int samplerate,
  int numchannel
) {
  SampleRate = samplerate;
  totalFrames = SampleRate * duration;
  numChannels = numchannel;
  numSamples = totalFrames * numChannels;
  numBytes = numSamples * sizeof(float);
  pSamples = (float*)malloc(numBytes);
  if (!pSamples) return false;
  memset(pSamples, 0, numBytes);
  return true;
}
```

### CreateSin

Fills buffer with `amplitude * sin(2π·frequency·t)` samples.

```cpp
for (int i = 0; i < numSamples; ++i)
  pSamples[i] = amplitude
    * sin( (float)i / SampleRate * M_PI * 2 * frequency_hz );
```

### CreateSquare

Uses a sine lookup to produce ±`amplitude` output.

```cpp
float temp = sin((float)i / SampleRate * M_PI * 2 * frequency_hz);
pSamples[i] = (temp > 0) ? amplitude : -amplitude;
```

### CreateSaw

Linearly ramps sample value from –`amplitude` to +`amplitude` each period.

### CreateTri

Alternates ramp direction at waveform peaks for a triangle shape.

---

## 3. Copying and Inserting Audio 🎚️

WavSet provides two families of **Copy** operations to duplicate or splice audio:

| Function | Allocation | Purpose |
| --- | --- | --- |
| **Copy(...)** | Yes | Clone entire buffer, one segment, or a time-range slice. |
| **CopyNoMalloc** | No | Insert a slice from source into an existing buffer. |


### Copy

Allocates a new buffer and copies:

1. **Entire WavSet** or a specific segment (`idSegment`).
2. **Time-range** by seconds or by frames.

**Signatures**

```cpp
bool Copy(WavSet* pWavSet, int idSegment = -1);
bool Copy(WavSet* pWavSet, float duration_s, float offset_s);
bool Copy(WavSet* pWavSet, int duration_frame, int offset_frame);
```

**Behavior**

- Allocates `pSamples = malloc(numBytes)`.
- Copies requested samples via `memcpy`.
- Returns `false` on allocation or bounds error.

```cpp
WavSet src, dst;
src.ReadWavFile("loop.wav");
// Copy middle 2 seconds starting at 1s
dst.Copy(&src, 2.0f, 1.0f);
```

### CopyNoMalloc

Writes into an already-allocated target buffer without new allocations.

**Signatures**

```cpp
bool CopyNoMalloc(
  WavSet* pWavSet,
  float duration_s,
  float src_offset_s,
  float dst_offset_s
);
bool CopyNoMalloc(
  WavSet* pWavSet,
  int duration_frame,
  int src_offset_frame,
  int dst_offset_frame
);
```

**Behavior**

- Asserts both source and destination buffers exist.
- Uses `memcpy` to move `duration_frame * numChannels * sizeof(float)` bytes.

```cpp
WavSet track, segment;
track.CreateSilence(10.0f);
segment.ReadWavFile("hit.wav");
// Overlay 'hit' at 3s into track, in-place
track.CopyNoMalloc(&segment, 0.5f, 0.0f, 3.0f);
```

---

**With these APIs**, WavSet lets you seamlessly load, generate, and manipulate audio buffers in C++. Whether you need to play back WAV files, synthesize tones, or stitch samples together, WavSet’s creation and loading operations form the foundation for higher-level transformations and playback.