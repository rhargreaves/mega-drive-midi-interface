# Sega Mega Drive MIDI Interface [![CircleCI](https://circleci.com/gh/rhargreaves/mega-drive-midi-interface.svg?style=svg)](https://circleci.com/gh/rhargreaves/mega-drive-midi-interface) [![GitHub release (latest by date)](https://img.shields.io/github/v/release/rhargreaves/mega-drive-midi-interface?style=plastic)](https://github.com/rhargreaves/mega-drive-midi-interface/releases)

Control the Yamaha YM2612 and PSG chips of the SEGA Mega Drive via MIDI.

## Features

- On-screen per-channel activity indicators & log
- Built-in FM presets for General MIDI compatibility
- Built-in PSG envelopes based on the [EEF](<https://github.com/rhargreaves/mega-drive-midi-interface/wiki/PSG-Envelopes>) format
- Supports MIDI 1.0 CCs & events (e.g. panning, volume, pitch bending)
- Polyphony via dynamic FM channel mapping
- Connectivity via:
  - [Mega EverDrive X7's](https://krikzz.com/store/home/33-mega-everdrive-x7.html) USB port
  - [Controller port via custom USB serial cable](https://github.com/rhargreaves/mega-drive-serial-port#hardware).
- Control of YM2612 registers via [GenMDM-style CCs](https://catskullelectronics.com/public/genMDM.pdf)

<p align="center">
    <img src="https://github.com/rhargreaves/mega-drive-midi-interface/raw/master/docs/screenshot.jpg" width="600" />
</p>

## Getting Started

Head over to the [Wiki](https://github.com/rhargreaves/mega-drive-midi-interface/wiki/Getting-Started) for more information and detailed reference pages.

## Build & Test

Unit & system tests are compiled and ran as x86 binaries using CMocka. Mega Drive interfaces are mocked.

### Docker:

```sh
./docker-make
```

### Linux (with `cmake` & [gendev](https://github.com/kubilus1/gendev)):

```sh
make
```

## Contributions

Pull requests are welcome, as are donations!

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=4VY6LCUMYLD42&source=url)
