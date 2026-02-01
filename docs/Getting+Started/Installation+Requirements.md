# Getting Started – Installation Requirements

This section details the prerequisites and configuration steps needed to build and use **spiwavsetlib** on Windows. Follow each requirement closely to ensure full functionality of WAV set creation, transformation, and playback.

## 1. Prerequisites

- A **C++ build environment** on Windows (e.g., Visual Studio 2019 or later).
- Windows SDK with support for
- ShellExecuteA (`<ShellAPI.h>`)
- Sleep function (`<windows.h>`)
- A minimum of **C++11** standard compliance.

## 2. External Libraries

Install and configure the following native libraries:

| Library | Minimum Version | Purpose | Include |
| --- | --- | --- | --- |
| **libsndfile** 🗄️ | 1.0+ | WAV file read/write (`SndfileHandle`) | `<sndfile.hh>` |
| **PortAudio** 🎧 | 19+ | Audio streaming & playback | `"portaudio.h"` |


**Setup Steps**

1. Download Windows binaries from each project’s website.
2. Add their **include** and **lib** directories to your Visual Studio project.
3. Link against `libsndfile.lib` and `portaudio.lib`.

## 3. Command-Line Audio Tools

A set of external players drive certain playback modes. By default, **spiwavsetlib** expects these executables at hard-coded paths:

| Tool | Default Path | Mode |
| --- | --- | --- |
| **sox.exe** | `C:\app-bin\sox\sox.exe` | `USING_SOX` |
| **spiplay.exe** | `C:\app-bin\spiplay\spiplay.exe` | `USING_SPIPLAY` |
| **spispectrumplay_asio.exe** | `C:\app-bin\spispectrumplay_asio\spispectrumplay_asio.exe` | `USING_SPISPECTRUMPLAY` |
| **spiplaystream_asio.exe** | `C:\app-bin\spiplaystream_asio\spiplaystream_asio.exe` | `USING_SPIPLAYSTREAM` |


## 4. WAV Samples & Descriptor Files

> **Tip:** To run in background, these commands invoke Windows `start` or `ShellExecuteA`.

To build **Instrument** objects from recorded audio, you need:

- **WAV sample folders** containing `.wav` files.
- **Text descriptor files** (e.g., `wavfolder_<instrument>.txt`) listing sample folders.

The library selects and loads these via:

```cpp
std::ifstream ifs("wsic_txtfilenames.txt");
while (getline(ifs, line)) {
  // line == "wavfolder_trumpet.txt", etc.
}
```

Instrument creation functions then parse these lists to pick sample sets .

## 5. Windows-Specific APIs

The code leverages Windows-only calls for process launching and timing:

```cpp
#include <ShellAPI.h>
ShellExecuteA(
  NULL,               // parent window
  "open",             // operation
  "C:\\app-bin\\spiplay\\spiplay.exe",
  args,               // quoted filename + duration
  NULL,
  SW_SHOW             // show window
);
```

```cpp
#include <windows.h>
// Pause threads to sync playback
Sleep(static_cast<DWORD>(duration_s * 1000));
```

These calls assume **Windows-style paths** and will not compile on other platforms  .

## 6. Environment Setup Steps

1. **Install Visual Studio**
2. Select “Desktop development with C++” workload.
3. **Acquire libsndfile & PortAudio**
4. Unzip binaries; configure VS include/lib paths.
5. **Deploy Command-Line Tools**
6. Copy `sox.exe`, `spiplay.exe`, and ASIO variants into `C:\app-bin\…`.
7. **Verify Paths**
8. If you install tools in another location, update hard-coded paths in `spiws_WavSet.cpp`.
9. **Prepare WAV Resources**
10. Create descriptor files (one `.txt` per instrument) at your working directory.

## 7. Configuration Overrides

You can adjust compile-time flags in **spiws_WavSet.h**:

```cpp
#define USING_SOX               1
#define USING_SPIPLAY           2
#define USING_SPISPECTRUMPLAY   3
#define USING_SPIPLAYSTREAM     4
#define USING_SPIPLAYX          5
```

Modify these or the system-call paths directly if your setup differs .

```card
{
    "title": "Windows Only",
    "content": "Spiwavsetlib relies on ShellExecuteA and Sleep; it does not support non-Windows systems."
}
```

Ensure all dependencies and tools are correctly installed before proceeding to build and use **spiwavsetlib**.