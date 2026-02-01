# WavSet: Audio Buffer and Operations – Saving Audio

This section dives into how **WavSet** persists in-memory audio buffers to disk. It covers two core methods:

- **WriteWavFile** – export a buffer as a new WAV file
- **AppendWavFile** – open an existing WAV and append samples at its end

These methods leverage **libsndfile** (via `SndfileHandle`) and form the foundation for higher-level playback routines like `Partition::LaunchScheduledPlay` and `InstrumentSet::Play`, which generate WAV files on-the-fly for external players (e.g., sox, spiplay).

---

## 📁 Persisting Audio Buffers to Disk

To save or extend WAV files, **WavSet** provides:

| Method | Purpose |
| --- | --- |
| **WriteWavFile(const char* filename)** | Export *this* buffer to a new WAV file using libsndfile. 💾 |
| **AppendWavFile(const char* filename)** | Open an existing WAV in read–write mode and append current samples to its end. ➕💾 |


---

## 💾 WriteWavFile

Export the **entire** or **partial** audio buffer to a fresh WAV file.

**Signature**

```cpp
bool WavSet::WriteWavFile(const char* filename);
```

**How it works**

1. Asserts that `filename` is valid.
2. Chooses WAV+PCM16 format:

```cpp
   const int format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
```

1. Opens a write-only handle:

```cpp
   SndfileHandle outfile(filename, SFM_WRITE, format, numChannels, SampleRate);
```

1. Writes samples:
2. If `frameIndex == 0`, writes *all* `numSamples`.
3. Otherwise, writes only `frameIndex * numChannels` frames.
4. Sets the internal name to the file path if unnamed.
5. Returns `true` on success.

**Example**

```cpp
WavSet ws;
// ... fill ws.pSamples ...
if (!ws.WriteWavFile("output.wav")) {
    std::cerr << "Failed to save WAV.\n";
}
```

---

## ➕💾 AppendWavFile

Append current samples to an **existing** WAV file.

**Signature**

```cpp
bool WavSet::AppendWavFile(const char* filename);
```

**How it works**

1. Validates `filename`.
2. Uses read-write mode:

```cpp
   const int format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
   SndfileHandle outfile(filename, SFM_RDWR, format, numChannels, SampleRate);
```

1. Seeks to end of existing data:

```cpp
   outfile.seek(outfile.frames(), SEEK_SET);
```

1. Writes current samples (full buffer or up to `frameIndex`).
2. Updates the internal name if unset.
3. Returns `true` on success.

**Example**

```cpp
WavSet ws;
// ... mix or generate new samples in ws ...
if (!ws.AppendWavFile("existing.wav")) {
    std::cerr << "Failed to append to WAV.\n";
}
```

---

## ⚙️ Dependencies & Integration

- **libsndfile**:

Header `<sndfile.hh>` provides `SndfileHandle`.

- **PortAudio**: Although not used here, these methods underpin playback pipelines.
- **External Players**:
- **sox** (`USING_SOX`)
- **spiplay** (`USING_SPIPLAY`)

**Partition::LaunchScheduledPlay** demonstrates how a partition’s `WavSet` is exported then executed by an external player:

```cpp
void Partition::LaunchScheduledPlay(int playerflag) {
    std::string filename = "temp_partition_" + partitionname + ".wav";
    pWavSet->WriteWavFile(filename.c_str());
    float duration = pWavSet->GetWavSetLength();
    // build and invoke sox or spiplay command...
}
```

Similarly, **InstrumentSet::Play** can concatenate multiple **WavSet** instances, write them with `WriteWavFile`, and launch playback across instruments.

---

## 🎯 Why These Methods Matter

- **Persistence**: Converts transient in-memory audio data into standard WAV files.
- **Interoperability**: Generated files seamlessly feed external tools (sox, spiplay) without custom streaming code.
- **Modularity**: Higher-level constructs (instruments, partitions) need only call these methods to handle all disk I/O.

By centralizing WAV export and append logic in **WavSet**, the library maintains a clear separation between audio processing and playback orchestration.