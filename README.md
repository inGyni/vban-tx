# VBAN Transmitter VST3 Audio Plugin

A VST3 audio plugin that transmits audio over the network using the VBAN (VB-Audio Network) protocol. This plugin allows you to stream audio from your DAW to remote VBAN receivers over UDP such as VB-Audio VoiceMeeter, VBAN Receptor, or other VBAN-compatible applications.

## Configuration

The plugin automatically creates a configuration file at:
- **Windows**: `%APPDATA%\VBANTX\config.json`

Default configuration:
```json
{
  "ip": "127.0.0.1",
  "port": 6980,
  "streamName": "AudioStream"
}
```

You can modify this file to change the target IP address, port, and stream name for your VBAN transmission.

## Building

### Prerequisites

#### Windows
- **CMake** 3.22 or later
- **Visual Studio 2019/2022** with C++ build tools
- **Git** for cloning the repository

#### Linux
- **CMake** 3.22 or later
- **GCC** 9 or later
- **Git** for cloning the repository

### Build Instructions

1. **Clone the repository with submodules:**
  ```bash
  git clone --recursive https://github.com/inGyni/vban-tx.git
  ```
  Then navigate to the project directory:
  ```
  cd vban-tx
  ```

  If you already cloned without submodules:
  ```bash
  git submodule update --init --recursive
  ```

2. **Generate build files:**
#### Windows
  ```bash
  cmake -S . -B build -G "Visual Studio 17 2022" -A x64
  ```
#### Linux
  ```bash
  cmake . -B build
  ```

3. **Build the plugin:**
#### Windows
  ```bash
  cmake --build build --config Release
  ```
#### Linux
  ```bash
  cmake --build build
```

### Build Output

The compiled VST3 plugin will be located in:
```
build/VBANTX_artefacts/..
```

## Installation

1. Copy the `VBAN TX.vst3` folder to your VST3 plugins directory:
   - **Windows**: `C:\Program Files\Common Files\VST3\`
    - **Linux**: `~/.vst3/`
   - Or your DAW's custom VST3 directory

2. Restart your DAW or rescan the VST3 directory and the plugin should appear in your plugin list

## Usage

1. Load the "VBAN TX" plugin on any audio track in your DAW
2. Configure the target IP address and port in the config file if needed (default sends to 127.0.0.1:6980)
3. Set up a VBAN receiver on the target machine
4. Play audio through the track - it will be transmitted via VBAN

## Dependencies

- [JUCE Framework](https://github.com/juce-framework/JUCE) - Audio plugin framework
- Standard C++ libraries for networking and threading