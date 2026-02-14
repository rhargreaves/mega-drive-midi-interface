# Mega Drive MIDI Interface
[![Build & Release](https://github.com/rhargreaves/mega-drive-midi-interface/actions/workflows/build.yml/badge.svg)](https://github.com/rhargreaves/mega-drive-midi-interface/actions/workflows/build.yml) [![GitHub release (latest by date)](https://img.shields.io/github/v/release/rhargreaves/mega-drive-midi-interface?style=plastic)](https://github.com/rhargreaves/mega-drive-midi-interface/releases)

Control the Yamaha YM2612 and PSG of the Sega Mega Drive / Genesis via MIDI.

<p align="center">
    <img src="https://github.com/rhargreaves/mega-drive-midi-interface/raw/main/docs/blastem_chan.png" width="600" />
</p>

## Features

- On-screen per-channel activity indicators & log
- Built-in FM presets for General MIDI compatibility
- Built-in PSG envelopes based on the [EEF](https://github.com/rhargreaves/mega-drive-midi-interface/wiki/PSG-Envelopes) format
- Supports MIDI 1.0 CCs & events (e.g. panning, volume, pitch bending)
- Loading of [user-defined presets](https://github.com/rhargreaves/mega-drive-midi-interface/wiki/FM-Presets) via SysEx
- Memoisation of a MIDI channel's current FM parameters as a user-defined preset
- Auto-saving & loading of user-defined presets in SRAM (EverDrive X7 & PRO only)
- Polyphony via dynamic FM channel mapping
- Portamento (glide)
- Experimental DAC support
- Connectivity via:
  - [Mega EverDrive X7](https://krikzz.com/store/home/33-mega-everdrive-x7.html) USB port.
  - [Mega EverDrive PRO](https://krikzz.com/store/home/59-mega-everdrive-pro.html) USB port.
  - [Controller or ext/modem port via custom USB serial cable](https://github.com/rhargreaves/mega-drive-serial-port#hardware).
  - [MegaWiFi cartridge / BlastEm emulator](https://github.com/rhargreaves/mega-drive-midi-interface/wiki/BlastEm-&-MegaWiFi-Guide) (experimental)
  - Control of YM2612 registers via [GenMDM-style CCs](https://catskullelectronics.com/pages/genmdm-manual)
- [Supplementary CLI](https://github.com/rhargreaves/mdmi-cli) for helping load/dump presets from/to TFI/DMP/WOPN files

## Getting Started

Head over to the [Wiki](https://github.com/rhargreaves/mega-drive-midi-interface/wiki/Getting-Started) for more information and detailed reference pages.

## Build & Test

Unit & system tests are compiled and ran as x86 binaries using CMocka. Mega Drive interfaces are mocked.

There are three flavours of ROM:

- **EverDrive**: for genuine Mega EverDrives.
- **Generic**: for other generic flash carts.
- **MegaWiFi**: for use with the BlastEm emulator or MegaWiFi hardware.

### Docker:

Build EverDrive ROM (default):

```sh
./docker-make release
```

Build Generic ROM:

```sh
ROM_TYPE=GENERIC ./docker-make release
```

Build MegaWiFi ROM:

```sh
ROM_TYPE=MEGAWIFI ./docker-make release
```

### Linux without Docker

You'll need to configure SGDK in your environment accordingly and make changes to the Makefile to support building outside of Docker. I've never tried to build it outside of Docker :)

```sh
make
```

> [!WARNING]
> Building this project requires certain modifications to be applied to the SGDK. If you are not building using Docker you will need to manually apply patches from the [docker-sgdk](https://github.com/rhargreaves/docker-sgdk) repo (*.patch files in the root) otherwise the build or tests may fail.

## Acknowledgments

* This project uses a pixel font and colour scheme from [MDPlayer by kuma4649](https://github.com/kuma4649/MDPlayer) licensed under the GNU General Public License v3.0.
* The built-in FM presets are based on [Wohlstand's XG bank from libOPNMIDI](https://github.com/Wohlstand/libOPNMIDI/blob/master/fm_banks/xg.wopn)

## Contributions

Pull requests are welcome, as are donations!

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=4VY6LCUMYLD42&source=url)
