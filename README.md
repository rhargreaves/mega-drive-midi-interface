# Mega Drive MIDI Interface [![Build](https://github.com/rhargreaves/mega-drive-midi-interface/workflows/Build%20&%20Release/badge.svg?branch=master)](https://github.com/rhargreaves/mega-drive-midi-interface/actions) [![GitHub release (latest by date)](https://img.shields.io/github/v/release/rhargreaves/mega-drive-midi-interface?style=plastic)](https://github.com/rhargreaves/mega-drive-midi-interface/releases)

Control the Yamaha YM2612 and PSG of the Sega Mega Drive via MIDI.

## Features

- On-screen per-channel activity indicators & log
- Built-in FM presets for General MIDI compatibility
- Built-in PSG envelopes based on the [EEF](https://github.com/rhargreaves/mega-drive-midi-interface/wiki/PSG-Envelopes) format
- Supports MIDI 1.0 CCs & events (e.g. panning, volume, pitch bending)
- Polyphony via dynamic FM channel mapping
- Connectivity via:
  - [Mega EverDrive X7](https://krikzz.com/store/home/33-mega-everdrive-x7.html) USB port.
  - [Mega EverDrive PRO](https://krikzz.com/store/home/59-mega-everdrive-pro.html) USB port.
  - [Controller port via custom USB serial cable](https://github.com/rhargreaves/mega-drive-serial-port#hardware).
  - MegaWiFi / BlastEm (WIP, not yet functional)
- Control of YM2612 registers via [GenMDM-style CCs](https://catskullelectronics.com/public/genMDM.pdf)

<p align="center">
    <img src="https://github.com/rhargreaves/mega-drive-midi-interface/raw/master/docs/screenshot.jpg" width="600" />
</p>

## Getting Started

Head over to the [Wiki](https://github.com/rhargreaves/mega-drive-midi-interface/wiki/Getting-Started) for more information and detailed reference pages.

## Build & Test

Unit & system tests are compiled and ran as x86 binaries using CMocka. Mega Drive interfaces are mocked.

### Docker:

Build EverDrive / Generic Flash ROM:

```sh
./docker-make
```

Build MegaWiFi ROM:

```sh
./docker-make ROM_TYPE=MEGAWIFI
```

### Linux (with `cmake` & [gendev](https://github.com/kubilus1/gendev)):

```sh
make
```

See Docker for arguments which can be passed to `make`.

## Contributions

Pull requests are welcome, as are donations!

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=4VY6LCUMYLD42&source=url)
