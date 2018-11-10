# Sega Mega Drive MIDI Interface [![CircleCI](https://circleci.com/gh/rhargreaves/mega-drive-midi-interface.svg?style=svg)](https://circleci.com/gh/rhargreaves/mega-drive-midi-interface)

Exposes the YM2612 FM synth chip as a MIDI interface

<p align="center">
    <img src="https://github.com/rhargreaves/mega-drive-midi-interface/raw/master/docs/screenshot.png" width="600" />
</p>

## Goal

The main goal of this project is to allow the Mega Drive's FM synthesis chip & PSG to be controlled via MIDI over a serial link (such as the USB interface provided by the Mega EverDrive X7 flash cart) so that it can be sequenced by a modern DAW like Albeton Live.

See also [stretch goals](#stretch-goals).

## Progress

In summary:

- Support for Note On/Off events on MIDI channels 1-6, using a fixed voice.

Check out the [development board](https://github.com/rhargreaves/mega-drive-midi-interface/projects/1) for a snapshot view of what's in progress.

## Getting Started

### Requirements

- Mega Drive/Genesis Console (PAL or NTSC)
- [Mega EverDrive X7 cart](https://krikzz.com/store/home/33-mega-everdrive-v2.html)
- A MIDI-to-Serial virtual device (such as [Hairless MIDI<->Serial Bridge](http://projectgus.github.io/hairless-midiserial/))
- Optional: MIDI loop device (so software running on the same PC as the USB connection can use the MIDI interface). In macOS this is possible via the use of a ["IAC Device Bus" creatable from the Audio MIDI Setup utility](https://help.ableton.com/hc/en-us/articles/209774225-Using-virtual-MIDI-buses).

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

- Support polyphony on a single MIDI channel (pool the YM2612's channels?)
- Support for MIDI messages to be received via the controller ports (in serial mode) as an alternative to the EverDrive X7.
- Support for loading & playback of PCM samples.

## Contributions

Pull requests are welcome.
