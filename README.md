# Sega Mega Drive MIDI Interface [![CircleCI](https://circleci.com/gh/rhargreaves/mega-drive-midi-interface.svg?style=svg)](https://circleci.com/gh/rhargreaves/mega-drive-midi-interface) [![GitHub release (latest by date)](https://img.shields.io/github/v/release/rhargreaves/mega-drive-midi-interface?style=plastic)](https://github.com/rhargreaves/mega-drive-midi-interface/releases)

Control the Yamaha YM2612 and PSG chips of the SEGA Mega Drive via MIDI.

## Features

- Built-in FM presets for General MIDI compatibility
- Built-in PSG envelopes based on the [EEF](<https://github.com/rhargreaves/mega-drive-midi-interface/wiki/PSG-Envelopes>) format
- Supports MIDI 1.0 CCs & events (e.g. panning, volume, pitch bending)
- Polythonic support via FM channel pooling
- Connectivity via:
  - Mega Everdrive's X7 USB port
  - [Custom-built USB cable connected to a controller port](https://github.com/rhargreaves/mega-drive-serial-port#hardware).
- Fine-grained control of YM2612 registers via [GenMDM-style CCs](https://catskullelectronics.com/public/genMDM.pdf)
- Optionally outputs FM parameter values to the screen for a specific MIDI channel (see CC 83)

<p align="center">
    <img src="https://github.com/rhargreaves/mega-drive-midi-interface/raw/master/docs/screenshot.jpg" width="600" />
</p>

## Getting Started

Check out the [Wiki](https://github.com/rhargreaves/mega-drive-midi-interface/wiki/Getting-Started) to get started.

## Build & Test

Unit & system tests are compiled and ran as x86 binaries using CMocka. Mega Drive interfaces are mocked.

Docker:

```sh
./docker-make
```

Linux (requires `cmake` & [gendev](https://github.com/kubilus1/gendev)):

```sh
make
```

## Stretch Goals

- Support for loading & playback of PCM samples.
- Support for loading & saving of FM/PSG presets via SD card or SysEx.

## Contributions

Pull requests are welcome, as are donations!

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=4VY6LCUMYLD42&source=url)
