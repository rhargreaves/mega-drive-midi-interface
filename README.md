# Sega Mega Drive MIDI Interface [![CircleCI](https://circleci.com/gh/rhargreaves/mega-drive-midi-interface.svg?style=svg)](https://circleci.com/gh/rhargreaves/mega-drive-midi-interface)

Exposes the YM2612 FM synth chip as a MIDI interface

## Goal

The main goal of this project is to allow the Mega Drive's FM synthesis chip & PSG to be controlled via MIDI over a serial link (such as the USB interface provided by the Mega EverDrive X7 flash cart) so that it can be sequenced by a modern DAW like Albeton Live.

See also [Stretch Goals](#stretch-goals).

## Getting Started

### Requirements

- Mega EverDrive X7 cart
- A MIDI-to-Serial virtual device (such as [Hairless MIDI<->Serial Bridge](http://projectgus.github.io/hairless-midiserial/))

## Build & Test

Unit tests are built with `cmocka` and are compiled to x86. Mega Drive interfaces are mocked out where appropriate.

Docker:

```sh
./docker-make all
```

Linux (requires `cmake` & [gendev](https://github.com/kubilus1/gendev)):

```sh
make all
```

## Stretch Goals

- Support for MIDI messages to be received via the controller ports (in serial mode) as an alternative to the EverDrive X7.
- Support for loading & playback of PCM samples.

## Contributions

Pull requests are welcome.
