# Sega Mega Drive MIDI Interface [![CircleCI](https://circleci.com/gh/rhargreaves/mega-drive-midi-interface.svg?style=svg)](https://circleci.com/gh/rhargreaves/mega-drive-midi-interface)

Exposes the Mega Drive's YM2612 FM Synth and PSG chip as a MIDI interface

<p align="center">
    <img src="https://github.com/rhargreaves/mega-drive-midi-interface/raw/master/docs/screenshot.png" width="600" />
</p>

## Goal

The main goal of this project is to allow the Mega Drive's FM synthesis chip & PSG to be controlled via MIDI over a serial link (such as the USB interface provided by the Mega EverDrive X7 flash cart) so that it can be sequenced by a modern DAW like Albeton Live.

The project will also support [Gen/MDM-style](https://catskullelectronics.com/public/genMDM.pdf) CC messages for compatibility with existing Gen/MDM-based applications.

Check out the [development board](https://github.com/rhargreaves/mega-drive-midi-interface/projects/1) for a snapshot view of what's in progress. See also [stretch goals](#stretch-goals).

## Getting Started

### Requirements

- Mega Drive/Genesis Console (PAL or NTSC)
- [Mega EverDrive X7 cart](https://krikzz.com/store/home/33-mega-everdrive-v2.html)
- A MIDI-to-Serial virtual device (such as [Hairless MIDI<->Serial Bridge](http://projectgus.github.io/hairless-midiserial/))
- Optional: MIDI loop device (so software running on the same PC as the USB connection can use the MIDI interface). In macOS this is possible via the use of a ["IAC Device Bus" creatable from the Audio MIDI Setup utility](https://help.ableton.com/hc/en-us/articles/209774225-Using-virtual-MIDI-buses).

### Download

You can download pre-built ROMs from [releases](https://github.com/rhargreaves/mega-drive-midi-interface/releases). Alternatively you can download the source and compile the ROM yourself (see [Build & Test](#build--test))

## Channel Mapping

| Channels | Assignment           |
| -------- | -------------------- |
| 1 - 6    | YM2612 FM Synthesis  |
| 7 - 9    | PSG Square Wave Tone |
| 10       | PSG Noise            |

## Control Change Messages

### MIDI Specification

| CC  | Description    | Effect                | Values                                             |
| --- | -------------- | --------------------- | -------------------------------------------------- |
| 7   | Channel Volume | FM: Op 4 Total Level  | 0 - 127: [Logarithmic](src/midi.c#L24)             |
|     |                | PSG: Attenuation      | 0 - 127: [Logarithmic](src/midi.c#L45)             |
| 10  | Panning        | Stereo                | 0 - 31: Left<br>32 - 96: Centre<br>97 - 127: Right |
| 123 | All Notes Off  | FM: Key Off           | 0                                                  |
|     |                | PSG: Max. Attenuation | 0                                                  |

### Gen/MDM Compatibility

Range determines how the possible 128 MIDI values are divided to give the respective YM2612 register value, using the formula:

_MIDI-Value / (128 / Range) = Register-Value_

For example: A MIDI value of 32, with CC range of 8 translates into to a YM2612 register value of 2.

| CC    | Description           | Range |
| ----- | --------------------- | ----- |
| 14    | FM Algorithm          | 8     |
| 15    | FM Feedback           | 8     |
| 16-19 | FM Total Level OP1-4  | 128   |
| 20-23 | FM Multiple OP1-4     | 16    |
| 24-27 | FM Detune OP1-4       | 8     |
| 28-31 | FM Rate Scaling OP1-4 | 4     |

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
