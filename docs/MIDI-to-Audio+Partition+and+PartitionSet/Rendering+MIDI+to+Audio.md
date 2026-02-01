# MIDI-to-Audio: Partition and PartitionSet – Rendering MIDI to Audio

This section explains how the library transforms a parsed MIDI “partition” into a stereo audio buffer. The core routine, `PartitionSet::CreateWavSetForPartition`, walks through each MIDI note event and mixes instrument samples into a silent track-long buffer.

## Workflow Overview

The conversion follows a clear six-step pipeline. Each partition ends up as a single `WavSet` ready for playback or file export.

```flowchart
flowchart TD
  Start[Start CreateWavSetForPartition]
  Clear[Delete existing pWavSet] 
  Compute[Compute partition length in seconds]
  Clamp[Clamp to max playback seconds]
  Silence[Create silent WavSet]
  Loop[For each MidiEventSet]
  GetSample[Retrieve WavSet from Instrument]
  MixNote[Compute offset & mix note]
  Name[Set partition name]
  End[Return success]

  Start --> Clear --> Compute --> Clamp --> Silence --> Loop
  Loop --> GetSample --> MixNote --> Loop
  Loop --> Name --> End
```

## Core Function Signature

```cpp
bool PartitionSet::CreateWavSetForPartition(
    Partition* pPartition,
    Instrument* pInstrument,
    float numberOfSecondsInPlayback
);
```

- Declared in `spiws_partitionset.h`
- Implemented in `spiws_partitionset.cpp`

## Step-by-Step Breakdown

1. **Clear existing buffer**
2. If `pPartition->pWavSet` is non-null, delete it and reset the pointer .
3. **Compute and clamp duration**
4. Call `GetLengthInSeconds(pPartition)` to translate MIDI ticks to seconds.
5. If duration exceeds `numberOfSecondsInPlayback`, clamp it.
6. **Create silent track**
7. Allocate a new `WavSet` and invoke `CreateSilence(duration)` to build a stereo, 44.1 kHz silent buffer .
8. **Iterate through MIDI events**
9. For each `MidiEventSet` in `pPartition->midieventsetvector`, request the matching sample set from the instrument.
10. Use `Instrument::GetWavSetFromMidiNoteNumber(pMidiEventSet)` or fallback to random selection.
11. **Mix each note**
12. Compute `offsetInSeconds = GetStartTimeStampInSeconds(pMidiEventSet)` and `durationInSeconds = GetLengthInSeconds(pMidiEventSet)`.
13. If `offsetInSeconds` is within playback window, call

```cpp
     pPartition->pWavSet->Sum(
       0.25f, pWavSet, offsetInSeconds, durationInSeconds
     );
```

to add the note into the silent buffer .

1. **Assign partition name**
2. Combine `miditrackname` with the instrument’s base name via

```cpp
     pPartition->partitionname =
       pPartition->miditrackname
       + pInstrument->GetInstrumentNameWithoutPath();
```

- Returns `true` on success; `false` otherwise.

## Key Classes and Methods

| Component | Responsibility | Reference |
| --- | --- | --- |
| PartitionSet | Parses MIDI, holds `partitionvector`, orchestrates rendering | `spiws_partitionset.h` |
| Partition | Contains `midieventsetvector` and `pWavSet`; holds per-track metadata | `spiws_partition.h` |
| MidiEventSet | Pairs a Note-On and Note-Off event, provides tick-based length and timestamp | `spiws_midieventset.h` |
| Instrument | Holds a collection of `WavSet` samples; maps MIDI note numbers to audio buffers | `spiws_instrument.h` |
| WavSet::CreateSilence(float) | Allocates and zero-fills a stereo buffer at 44.1 kHz | `spiws_WavSet.h` |
| WavSet::Sum(float,…) | Mixes a source buffer into a destination at given offset and duration | `spiws_WavSet.cpp` |
| PartitionSet::GetLengthInSeconds | Converts MIDI clock ticks into real-time seconds | `spiws_partitionset.cpp` |
| PartitionSet::GetStartTimeStampInSeconds | Converts event start ticks to seconds | `spiws_partitionset.cpp` |


## Usage in Playback

`PartitionSet::Play` assigns instruments (random or by track name) and invokes `CreateWavSetForPartition` for each partition. The resulting `WavSet` can then be played via PortAudio or external utilities (SoX, spiplay).

```cpp
for (auto pPartition : partitionvector) {
  Instrument* pInstrument = instrumentSet->GetInstrumentRandomly();
  CreateWavSetForPartition(pPartition, pInstrument, maxPlaybackSec);
  pPartition->LaunchScheduledPlay(USING_SOX);
}
```

(Implementation excerpt )

## Best Practices and Notes

- Ensure your `InstrumentSet` is populated before rendering.
- The default mix amplitude (`0.25f`) can be adjusted per instrument or globally.
- Clamping duration prevents runaway memory use.
- Partition names assist in file naming and debugging. 📝

This conversion pipeline unifies MIDI sequencing with sample-based playback, producing self-contained `WavSet` objects that drive real-time audio or file rendering with minimal client code.