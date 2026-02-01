# Instrument: Building Playable Instruments

This section covers how the `Instrument` class locates and retrieves `WavSet` objects from its internal collection. These helpers enable mapping MIDI notes, random selection, and pattern-based sample triggering—key capabilities for algorithmic composition and dynamic playback.

## Finding and Using WavSets Within an Instrument

Each `Instrument` maintains a vector of `WavSet*` called `wavsetvector`. The following helpers let you pick the right sample:

| Function | Purpose | Parameters | Returns |
| --- | --- | --- | --- |
| **GetWavSetFromMidiNoteNumber** | Find a `WavSet` matching a given MIDI note number. | `int midinotenumber` | `WavSet*` |
| **GetWavSetFromMidiNoteNumber** (overload) | Same as above, but accepts a `MidiEventSet*` for convenience. | `MidiEventSet* pMidiEventSet` | `WavSet*` |
| **GetWavSetRandomly** 🎲 | Select a **random** `WavSet` from the instrument. |  | `WavSet*` |
| **GetWavSetFromPatternCode** 🔣 | Lookup a `WavSet` by its single-character **pattern code**. | `const char* patterncode` | `WavSet*` |


---

## GetWavSetFromMidiNoteNumber

Retrieves a sample whose `midinote` field equals the requested value.

If multiple matches exist, one is chosen at random; if none match, a fallback index is used.

```cpp
WavSet* Instrument::GetWavSetFromMidiNoteNumber(int midinotenumber) {
    WavSet* pWavSet = nullptr;
    if (midinotenumber < 0 || midinotenumber > 127) {
        assert(false);
        return nullptr;
    }
    // 1) Collect matches
    std::vector<WavSet*> temp;
    for (auto* ws : wavsetvector) {
        if (midinotenumber == ws->midinote)
            temp.push_back(ws);
    }
    // 2) Choose one
    if (!temp.empty()) {
        if (temp.size() == 1)
            pWavSet = temp[0];
        else {
            int r = 1 + int(temp.size() * rand()/(RAND_MAX+1.0)) - 1;
            pWavSet = temp[r];
        }
    }
    // 3) Fallback if no match
    else {
        int idx = wavsetvector.size() * midinotenumber / 128;
        pWavSet = wavsetvector[idx];
    }
    return pWavSet;
}
```

Reference:

### Overload: Using a MIDI Event

Automatically extracts the note number from a `MidiEventSet` and delegates:

```cpp
WavSet* Instrument::GetWavSetFromMidiNoteNumber(MidiEventSet* pMidiEventSet) {
    assert(pMidiEventSet);
    return GetWavSetFromMidiNoteNumber(pMidiEventSet->GetNoteNumber());
}
```

Reference:

---

## GetWavSetRandomly 🎲

Useful for **stochastic** playback and introducing variation. Picks any `WavSet` in the collection with uniform probability.

```cpp
WavSet* Instrument::GetWavSetRandomly() {
    int n = wavsetvector.size();
    int idx = int(n * rand()/(RAND_MAX + 1.0));
    return wavsetvector[idx];
}
```

Reference:

---

## GetWavSetFromPatternCode 🔣

Selects a `WavSet` whose `patterncode` field matches the given character.

Supports single-symbol patterns; if multiple sets share the same code, one is chosen at random.

```cpp
WavSet* Instrument::GetWavSetFromPatternCode(const char* patterncode) {
    assert(patterncode && strlen(patterncode) == 1);
    std::vector<WavSet*> temp;
    for (auto* ws : wavsetvector) {
        if (*patterncode == ws->GetPatternCode())
            temp.push_back(ws);
    }
    if (temp.empty())
        assert(false);
    if (temp.size() == 1)
        return temp[0];
    int r = 1 + int(temp.size() * rand()/(RAND_MAX+1.0)) - 1;
    return temp[r];
}
```

Reference:

---

## Usage Examples

### Rendering MIDI Partitions

In `PartitionSet::CreateWavSetForPartition`, each `MidiEventSet` is mapped to a sample:

```cpp
for (auto* evt : pPartition->midieventsetvector) {
    if (!evt) continue;
    WavSet* ws = pInstrument->GetWavSetFromMidiNoteNumber(evt);
    if (ws) {
        float start = GetStartTimeStampInSeconds(evt);
        float len   = GetLengthInSeconds(evt);
        pPartition->pWavSet->Sum(0.25f, ws, start, len);
    }
}
```

Reference:

### Algorithmic Patterns with SpreadSamples

Using non-binary patterns to distribute samples over time:

```cpp
float total = 0.0f;
std::string pat = patterncode; // e.g. "923019"
for (int i = 0; i < pat.size(); ++i) {
    std::string code = pat.substr(i,1);
    float offset = baseOffset + i*(distance/number);
    WavSet* ws;
    if (code != "0" && code != "1")
        ws = pInstrument->GetWavSetFromPatternCode(code.c_str());
    else
        ws = pInstrument->GetWavSetRandomly();
    Sum(amplitude, ws, offset, duration);
    total = offset + duration;
}
return total;
```

Reference:

---

💡 **Tip**

Use **GetWavSetRandomly** to inject variation when pattern codes default to “on/off” logic. A mix of deterministic (`patterncode`) and stochastic (`random`) selection enables rich, evolving textures.

---

These helpers abstract away manual indexing and randomization logic, letting you focus on high-level composition rules and MIDI mapping.