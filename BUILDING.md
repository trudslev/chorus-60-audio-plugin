# Building CHORUS-60

CHORUS-60 builds on macOS (AU + VST3 + Standalone), Windows (VST3 + Standalone), and Linux
(VST3 + Standalone) — AU is Apple-only. JUCE 8.0.14 is fetched automatically via CMake
`FetchContent` on first configure (no local JUCE checkout needed) on any platform.

## macOS

### Requirements

- Xcode (full install, not just Command Line Tools) — `xcodebuild -version` must succeed.
- CMake 3.24+.
- [pluginval](https://github.com/Tracktion/pluginval) for VST3 validation: `brew install --cask pluginval`.

### Build

```sh
cmake -B build -G Xcode
cmake --build build --config Release
```

This builds AU, VST3, and a Standalone app, and installs the AU/VST3 bundles to:

```
~/Library/Audio/Plug-Ins/Components/CHORUS-60.component
~/Library/Audio/Plug-Ins/VST3/CHORUS-60.vst3
```

### Validate

```sh
auval -a | grep -i chorus                    # confirm AU registration + 4-char codes
auval -v aufx Ch60 Nfdy                      # full AU validation

/Applications/pluginval.app/Contents/MacOS/pluginval \
    --strictness-level 8 \
    --validate ~/Library/Audio/Plug-Ins/VST3/CHORUS-60.vst3
```

If Logic Pro doesn't pick up a freshly built AU: Preferences → Audio Units Manager → "Reset & Rescan Selection", or restart Logic.

### Run the unit/DSP tests

```sh
./build/Tests/Chorus60Tests_artefacts/Release/Chorus60Tests
```

## Windows

### Requirements

- Visual Studio 2022 or later with the "Desktop development with C++" workload.
- CMake 3.24+ (bundled with Visual Studio, or install separately).
- [pluginval](https://github.com/Tracktion/pluginval) for VST3 validation (Windows build available from the same releases page) — no `auval` equivalent, since AU doesn't exist on Windows.

### Build

```bat
cmake -B build -A x64
cmake --build build --config Release
```

This builds VST3 and a Standalone app. VST3 install location is JUCE's own platform default
(`%COMMONPROGRAMFILES%\VST3\CHORUS-60.vst3`, i.e. usually `C:\Program Files\Common Files\VST3\`) —
CHORUS-60 doesn't override `VST3_COPY_DIR` on Windows.

### Validate

```bat
pluginval.exe --strictness-level 8 --validate "%COMMONPROGRAMFILES%\VST3\CHORUS-60.vst3"
```

### Run the unit/DSP tests

```bat
build\Tests\Chorus60Tests_artefacts\Release\Chorus60Tests.exe
```

## Linux

### Requirements

- A C++20-capable compiler (GCC or Clang) and CMake 3.24+.
- JUCE's standard Linux build dependencies:
  ```sh
  sudo apt-get install -y \
      libasound2-dev libjack-jackd2-dev \
      libcurl4-openssl-dev \
      libfreetype6-dev libfontconfig1-dev \
      libx11-dev libxcomposite-dev libxcursor-dev libxext-dev libxinerama-dev libxrandr-dev libxrender-dev \
      libwebkit2gtk-4.1-dev \
      libglu1-mesa-dev mesa-common-dev
  ```
  (package names above are for Debian/Ubuntu — adjust for other distros).
- [pluginval](https://github.com/Tracktion/pluginval) for VST3 validation (Linux build available from the same releases page) — no `auval` equivalent, since AU doesn't exist on Linux.

### Build

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Unlike the Xcode/Visual Studio generators used on macOS/Windows, CMake's default Linux generators
(Makefiles/Ninja) are single-config, so `CMAKE_BUILD_TYPE=Release` must be set at configure time.

This builds VST3 and a Standalone app. VST3 install location is JUCE's own platform default
(`~/.vst3/CHORUS-60.vst3`) — CHORUS-60 doesn't override `VST3_COPY_DIR` on Linux.

### Validate

```sh
./pluginval --strictness-level 8 --validate ~/.vst3/CHORUS-60.vst3
```

### Run the unit/DSP tests

```sh
./build/Tests/Chorus60Tests_artefacts/Release/Chorus60Tests
```

## What the DSP test suite covers

Each modulation engine's bounded/finite excursion (never more than ~2.5ms even at full depth) and
its measurably asymmetric (non-sinusoidal) peak timing; the BBD delay line's impulse-response delay
accuracy, its two simultaneous taps staying independent, and its reconstruction filter's ~7kHz
rolloff; stereo decorrelation's genuine L/R difference (never touching the left channel); the
character stage's bounded drift range, near-transparent behavior at Saturation=0, and a measurably
louder noise floor as Noise increases; output mix/trim's dry/wet blend and gain math; APVTS
parameter defaults and session round-tripping; factory-program structural sanity; and a full-chain
CPU check at 48kHz/64-sample buffers with both engines active, against the real-time budget.

Tonal correctness of the DSP models and the eventual full factory-program bank is explicitly **not**
covered here — see the "DSP tuning" note below.

## Notes

- `PLUGIN_MANUFACTURER_CODE` (`Nfdy`, shared across the suite), `PLUGIN_CODE` (`Ch60`), `BUNDLE_ID`
  (`com.neonfoundry.chorus60`), and `COMPANY_NAME` (`Neon Foundry`) — finalize
  these before any real release, since they're effectively permanent once shipped or automated
  against.
- JUCE's free/personal tier splash screen is enabled (no paid license configured).
- **DSP tuning**: every DSP stage has real, functioning processing (no stubs), grounded in
  `design/BBD-TECHNICAL-NOTES.md`'s description of the real circuit, but the exact filter cutoffs,
  excursion/drift/noise-floor ranges, and saturation drive curve are a first, technically-reasoned
  pass, not a tuned one - same status both siblings' own DSP had before their by-ear pass. Build,
  load, listen, adjust.
- **Factory bank**: only 3 baseline programs (I, II, I+II) are implemented so far - the full curated
  16-name bank from `design/CHORUS60-GUI-SPEC.md` section 9 is an explicit follow-up, tracked in
  `prompts/PROMPTS.md`.
