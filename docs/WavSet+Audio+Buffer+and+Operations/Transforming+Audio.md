# WavSet: Audio Buffer and Operations – Transforming Audio

**WavSet** is the core C++ class for managing in-memory WAV audio buffers. Once you load or generate audio into a `WavSet`, you can perform a variety of **transformations**—splitting into segments, erasing content, fading edges, reversing, and simple resampling. These operations enable texture creation, slicing for granular playback, and buffer preparation for PortAudio or external players (sox, spiplay).

---

## Splitting Audio into Segments 🪡

You can subdivide a continuous buffer into equally-sized time segments. This sets up segment-level editing and playback.

### SplitInSegments

```cpp
bool SplitInSegments(double secondsPerSegment);
```

- **Purpose**: Divide the buffer into chunks of `secondsPerSegment` seconds.
- **Behavior**:
- Computes `numFramesPerSegment = secondsPerSegment * SampleRate`.
- Calculates `numSegments = totalFrames / numFramesPerSegment`.
- Updates segment counts and per-segment byte offsets.
- Warns (via debug log and `StatusAddText`) if `secondsPerSegment` is zero or exceeds total length, then treats entire buffer as one segment.
- **Returns**: `true` on successful segmentation.
- **Declaration**:
- **Implementation**:

### GetPointerToSegmentData

```cpp
float* GetPointerToSegmentData(int idSegment);
```

- **Purpose**: Obtain a raw pointer to the float sample data of a specific segment.
- **Parameters**:
- `idSegment`: Zero-based index of the segment (0 ≤ idSegment < numSegments).
- **Returns**:
- Pointer to the first sample of the segment.
- `NULL` (and triggers `assert`) if:
- `pSamples` is `NULL`,
- segmentation not initialized (`numSegments == -1`), or
- `idSegment` is out of range.
- **Declaration**:
- **Implementation**:

#### Example

```cpp
wavSet.SplitInSegments(0.5);           // 0.5-second slices
float* segmentData = wavSet.GetPointerToSegmentData(2);
```

---

## Erasing Audio Buffers 🗑️

Zero-out samples to silence either the **entire buffer** or a **single segment**.

| Method | Description | Declaration & Implementation |
| --- | --- | --- |
| **Erase()** | Zero out **all** channels for every frame in the buffer. | / |
| **EraseSegment(int idSegment)** | Zero out samples only within the specified segment. | / |


Both methods assume **stereo** (`numChannels == 2`) and iterate per frame:

```cpp
// Erase entire buffer
for(int i = 0; i < totalFrames; i++) {
    pSamples[2*i]   = 0.0f; // left
    pSamples[2*i+1] = 0.0f; // right
}
```

---

## Fading Segment Edges 🌅

Prevent clicks when concatenating or looping segments by applying linear fades at the start and end of each slice.

```cpp
bool FadeSegmentEdges(int idSegment);
```

- **Purpose**: Apply a **fade-in** at the beginning and **fade-out** at the end of a segment.
- **Algorithm**:
- Compute `numFramesPerEdge = numFramesPerSegment / 16`.
- For frames `i` in `[0 .. numFramesPerEdge)`, scale sample by `i/numFramesPerEdge` (fade-in).
- For the last `numFramesPerEdge` frames, scale by `(1 – (i/numFramesPerEdge))` (fade-out).
- **Failure**: Returns `false` if the segment is too short for the chosen edge length.
- **Declaration**:
- **Implementation**:

---

## Reversing Audio Playback ↩️

Flip sample order in place for creative effects like reverse reverb or backward loops.

```cpp
bool Reverse();
```

- **Purpose**: Reverse the audio buffer, handling **mono** (`numChannels == 1`) or **stereo** (`numChannels == 2`).
- **Algorithm**:
- Two-pointer swap from both ends moving toward the center, swapping channel pairs appropriately.
- **Returns**: `true` on success; `false` (with `assert`) if an unsupported channel count is encountered.
- **Declaration**:
- **Implementation**:

#### Example

```cpp
wavSet.Reverse(); // Now playback will run from end to start
```

---

## Resampling Helpers 🔄

Simple, non-interpolated resampling that adjusts **sample rate** and/or **channel count**. Note: these methods may introduce frequency shifts.

| Method | From → To | Behavior | Citation |
| --- | --- | --- | --- |
| **Resample44100monoTo44100stereo** | 44 100 Hz mono → 44 100 Hz stereo | Duplicates each mono sample into both left and right channels. |  |
| **Resample48000monoTo44100mono** | 48 000 Hz mono → 44 100 Hz mono | Sets `SampleRate = 44100` without changing sample data (no interpolation). |  |
| **Resample48000monoTo44100stereo** | 48 000 Hz mono → 44 100 Hz stereo | First calls `Resample48000monoTo44100mono()`, then duplicates to stereo as above. |  |
| **Resample48000stereoTo44100stereo** | 48 000 Hz stereo → 44 100 Hz stereo | Sets `SampleRate = 44100`, preserving channel interleaving (no interpolation). |  |


---

## Usage Patterns and Context

- **Integration**:
- `WavSet` is used by **Instrument** to build sample-based instruments (via `CreateSin`, `CreateSquare`, etc.).
- **PartitionSet** leverages `SplitInSegments`, fades, and segment access to schedule MIDI-mapped playback.
- **Playback**:
- Buffers can be streamed via PortAudio (`OpenStream`/`CloseStream`) or written to disk and launched with sox/spiplay.
- **Design**:
- Emphasizes **simplicity** over high-quality resampling; transformation methods are lightweight and real-time-friendly.
- Segmentation enables granular synthesis, pattern-based slicing, and audio textures.

---

This section empowers you to **slice**, **silence**, **smooth**, **reverse**, and **retune** your audio buffers within the `spiwavsetlib` framework, forming the foundation for pattern-driven sample playback and real-time audio transformations.