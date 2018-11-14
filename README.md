# Sega Mega Drive MIDI Interface [![CircleCI](https://circleci.com/gh/rhargreaves/mega-drive-midi-interface.svg?style=svg)](https://circleci.com/gh/rhargreaves/mega-drive-midi-interface)

Exposes the Mega Drive's YM2612 FM Synth and PSG chip as a MIDI interface

<p align="center">
    <img src="https://github.com/rhargreaves/mega-drive-midi-interface/raw/master/docs/screenshot.png" width="600" />
</p>

## Goal

The main goal of this project is to allow the Mega Drive's FM synthesis chip & PSG to be controlled via MIDI over a serial link (such as the USB interface provided by the Mega EverDrive X7 flash cart) so that it can be sequenced by a modern DAW like Albeton Live.

See also [stretch goals](#stretch-goals).

## Getting Started

### Requirements

- Mega Drive/Genesis Console (PAL or NTSC)
- [Mega EverDrive X7 cart](https://krikzz.com/store/home/33-mega-everdrive-v2.html)
- A MIDI-to-Serial virtual device (such as [Hairless MIDI<->Serial Bridge](http://projectgus.github.io/hairless-midiserial/))
- Optional: MIDI loop device (so software running on the same PC as the USB connection can use the MIDI interface). In macOS this is possible via the use of a ["IAC Device Bus" creatable from the Audio MIDI Setup utility](https://help.ableton.com/hc/en-us/articles/209774225-Using-virtual-MIDI-buses).

### Download

You can download pre-built ROMs from [releases](https://github.com/rhargreaves/mega-drive-midi-interface/releases). Alternatively you can download the source and compile the ROM yourself (see [Build & Test](#build--test))

## Progress

In summary, support for:

- Note On/Off events.
- A handful of CCs.

Check out the [development board](https://github.com/rhargreaves/mega-drive-midi-interface/projects/1) for a snapshot view of what's in progress.

## MIDI Channels

* 1 to 6 are assigned to the YM2612's respective FM channels.
* 7 to 9 are assigned to the PSG's respective tone channels.
* 10 is assigned to the PSG's noise channel.

## Control Change Messages

### MIDI

| CC  | Description    | Effect                  | Values |
|-----|----------------|-------------------------|--------|
| 7   | Channel Volume | FM: Op 4 Total Level    | 0 - 127: [Logarithmic](src/midi.c#L24) |
|     |                | PSG: Attenuation        | 0 - 127: [Logarithmic](src/midi.c#L45) |
| 10  | Panning        | Stereo                  | 0 - 31: Left<br>32 - 96: Centre<br>97 - 127: Right |
| 123 | All Notes Off  | Key Off                 | 0 |

### GenMDM Compatible

| CC  | Description    | Range  |
|-----|----------------|--------|
| 14  | FM Algorithm   | 8      |

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
