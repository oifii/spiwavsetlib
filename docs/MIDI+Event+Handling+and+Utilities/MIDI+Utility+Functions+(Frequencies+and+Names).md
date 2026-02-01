## MIDI Event Handling and Utilities

This module provides a set of **utility functions** for converting between MIDI note numbers, frequencies, and human-readable note names. By isolating musical-theory logic here, the rest of the library can focus on audio buffer management and playback without embedding pitch calculations or string parsing.

### MIDI Utility Functions (Frequencies and Names)

The `**spiws_midiutility**` module defines constants and functions to:

- Translate a MIDI note number (0–127) to its fundamental frequency in Hz.
- Map between note-name strings (e.g. “C4”, “A#3”) and MIDI note numbers.
- Query characteristics of notes (octave, pitch class, remappings, etc.).

These utilities are declared in `spiws_midiutility.h`  and implemented in `spiws_midiutility.cpp` and related `.cpp` files .

#### 🎵 Frequency Lookup

**Purpose:** Generate the fundamental frequency for a given MIDI note, used extensively in synthesizer creation (e.g. `Instrument::CreateWavSynth`).

| Function | Signature | Description |
| --- | --- | --- |
| **GetFrequencyFromMidiNoteNumber** | `float GetFrequencyFromMidiNoteNumber(int midinotenumber);` | Returns frequency in Hz using A4=440 Hz standard tuning. |
| **GetMidiNoteNumberFromFrequency** | `int   GetMidiNoteNumberFromFrequency(float frequency_hz);` | Inverts frequency → MIDI note (rounded). |


```cpp
#include "spiws_midiutility.h"

// A4 is MIDI note 69, should yield 440.0 Hz
float freq = GetFrequencyFromMidiNoteNumber(69);
// e.g., freq == 440.0f

// Approximate MIDI note for 261.63 Hz (middle C)
int midi = GetMidiNoteNumberFromFrequency(261.63f);
// e.g., midi == 60
```

Implementation uses the formula

```plaintext
frequency_hz = (440.0f / 32.0f) * pow(2.0f, (midinotenumber - 9.0f) / 12.0f);
```

asserting `0 ≤ midinotenumber < 128` .

#### 📛 Note Name Lookup

**Purpose:** Convert between MIDI note numbers and human-readable names, or parse note-name strings to MIDI numbers.

| Function | Signature | Description |
| --- | --- | --- |
| **GetNoteNameFromMidiNoteNumber** | `const char* GetNoteNameFromMidiNoteNumber(int midinotenumber);` | Returns name from internal array, e.g. “C4”, “G#-1” . |
| **GetMidiNoteNumberFromString** | `int       GetMidiNoteNumberFromString(const char* pstring);` | Parses strings like “C#3” → 49; handles sharps (“#”, “s”) and octaves. |
| **IsNoteName** | `bool      IsNoteName(string mystring);` | Validates full names (with octave). |
| **IsNoteNoOctave** | `bool      IsNoteNoOctave(string mystring);` | Validates note names without octave. |


```cpp
// Full name lookup
const char* name = GetNoteNameFromMidiNoteNumber(60);
// name == "C4"

// String-to-MIDI lookup
int midi1 = GetMidiNoteNumberFromString("A#3"); // returns 46
int midi2 = GetMidiNoteNumberFromString("CS5"); // alternative sharp notation

// Validations
bool ok1 = IsNoteName("G2");   // true
bool ok2 = IsNoteName("H4");   // false
bool ok3 = IsNoteNoOctave("Fs"); // true
```

Names are stored in a 132-element array to cover octave –1 through 9 . Parsing scans for a letter (A–G), optional sharp, and octave digit, then computes

```plaintext
midinote = baseOffset + 12 * (octave + 1)
```

with assertions for 0–127 .

#### 🔍 Note Characteristic Queries

Beyond basic conversion, several **helper predicates** and remapping functions enable scale enforcement or pitch-class queries:

- **Pitch-class checks** (boolean tests for note identity):
- `IsC`, `IsCs`, `IsD`, `IsDs`, …, `IsB`
- Example: `IsA(69) == true`, `IsAs(69) == false`.
- **Octave extraction**:
- `int GetOctave(int midinotenumber);`

Returns octave number (e.g. `60/12 – 1 == 4`).

- **Remapping**:
- `GetNoteRemap_C_MinorPentatonic(int midinotenumber);`
- `GetNoteRemap_C3(int midinotenumber);`
- `GetNoteRemap(const char* pstring, int midinotenumber);`

Facilitate scale-based pitch adjustments in playback.

```cpp
// Check if note is a sharp
if (IsFs(66)) {
  // ...
}

// Remap a note into C minor pentatonic
int remapped = GetNoteRemap_C_MinorPentatonic(65);
```

These utilities free the **audio engine** from hard-coded pitch logic, enabling flexible instrument creation, scale constraints, or dynamic note selection by name or number.

---

**Card example highlighting best practice:**

```card
{
    "title": "Decouple Logic",
    "content": "Keep musical theory separate from audio buffer code for maintainability."
}
```