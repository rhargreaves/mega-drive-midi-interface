# Sega Mega Drive MIDI Interface [![CircleCI](https://circleci.com/gh/rhargreaves/mega-drive-midi-interface.svg?style=svg)](https://circleci.com/gh/rhargreaves/mega-drive-midi-interface) [![GitHub release (latest by date)](https://img.shields.io/github/v/release/rhargreaves/mega-drive-midi-interface?style=plastic)](https://github.com/rhargreaves/mega-drive-midi-interface/releases)

Exposes the Mega Drive's YM2612 FM Synth and PSG as a MIDI interface

<p align="center">
    <img src="https://github.com/rhargreaves/mega-drive-midi-interface/raw/master/docs/screenshot.png" width="600" />
</p>

## Goal

The main goal of this project is to allow the Mega Drive's FM synthesis chip & PSG to be controlled via MIDI over a serial link (such as the USB interface provided by the Mega EverDrive X7 flash cart) so that it can be sequenced by a modern DAW like Albeton Live.

The project will also support [GenMDM-style](https://catskullelectronics.com/public/genMDM.pdf) CC messages for compatibility with existing GenMDM-based applications.

Check out the [development board](https://github.com/rhargreaves/mega-drive-midi-interface/projects/1) for a snapshot view of what's in progress. See also [stretch goals](#stretch-goals).

## Getting Started

### Hardware Requirements

- SEGA Mega Drive/Genesis Console
- [Mega EverDrive X7 cart](https://krikzz.com/store/home/33-mega-everdrive-v2.html)

### Software Requirements

- MIDI to serial port virtual device, e.g.

  - [Hairless MIDI<->Serial Bridge](http://projectgus.github.io/hairless-midiserial/)

- Optional: MIDI loop device (so software running on the same PC as the USB connection can use the MIDI interface). In macOS this is possible via the use of a ["IAC Device Bus" creatable from the Audio MIDI Setup utility](https://help.ableton.com/hc/en-us/articles/209774225-Using-virtual-MIDI-buses).

### Download

You can download pre-built ROMs from [releases](https://github.com/rhargreaves/mega-drive-midi-interface/releases).

## MIDI Channel Mapping

| Channels | Assignment                  |
| -------- | --------------------------- |
| 1 - 6    | YM2612 FM Channels 1 - 6    |
| 7 - 9    | PSG Square Wave Tones 1 - 3 |
| 10       | PSG Noise Channel           |
| 11 - 13  | As 7 - 9                    |
| 14 - 16  | As 7 - 9                    |

You can also [re-configure the MIDI mappings](#system-exclusive) via SysEx.

## Supported Events

- Note On/Off
- Pitch Bend
- Program Change (Select FM Preset)
- Universal SysEx Messages

## Control Change Messages

### MIDI Specification

| CC  | Description    | Effect                     | Values                                             |
| --- | -------------- | -------------------------- | -------------------------------------------------- |
| 7   | Channel Volume | FM: Output Op. Total Level | 0 - 127: [Logarithmic](src/midi.c#L24)             |
|     |                | PSG: Attenuation           | 0 - 127: [Logarithmic](src/midi.c#L45)             |
| 10  | Panning        | Stereo                     | 0 - 31: Left<br>32 - 96: Centre<br>97 - 127: Right |
| 123 | All Notes Off  | FM: Key Off                | 0                                                  |
|     |                | PSG: Max. Attenuation      | 0                                                  |

### GenMDM Compatibility

Range determines how the possible 128 MIDI values are divided to give the respective YM2612 register value, using the formula:

_midiValue / (128 / range) = registerValue_

For example: A MIDI value of 32, with CC range of 8 translates into to a YM2612 register value of 2.

#### Global FM Parameters

| CC  | Description          | Range |
| --- | -------------------- | ----- |
| 74  | Global LFO Enable    | 2     |
| 1   | Global LFO Frequency | 8     |
| 80  | Polyphonic Mode\*    | 2     |

#### FM Channels

| CC  | Description                      | Range |
| --- | -------------------------------- | ----- |
| 14  | Algorithm                        | 8     |
| 15  | Feedback                         | 8     |
| 75  | Frequency Modulation Level (FMS) | 8     |
| 76  | Amplitude Modulation Level (AMS) | 4     |

#### FM Channel Operators

Each CC relates to one of the four FM channel operators for the parameter type

| CC    | Description               | Range |
| ----- | ------------------------- | ----- |
| 16-19 | Total Level (TL)          | 128   |
| 20-23 | Multiple (MUL)            | 16    |
| 24-27 | Detune (DT1)              | 8     |
| 39-42 | Rate Scaling (RS)         | 4     |
| 43-46 | Attack Rate (AR)          | 32    |
| 47-50 | First Decay Rate (D1R)    | 32    |
| 51-54 | Second Decay Rate (D2R)   | 16    |
| 55-58 | Secondary Amplitude (D1L) | 16    |
| 59-62 | Release Rate (RR)         | 16    |
| 70-73 | Amplitude Modulation (AM) | 2     |
| 90-93 | SSG-EG                    | 16    |

## Polyphonic Mode (CC 80)

When polyphonic mode is enabled, all note on/off events are routed to a pool of
FM channels, ignoring the specific MIDI channel the event is sent to. This allows for
polyphony within a single MIDI channel.

In addition, any FM parameter change made will be sent to all FM channels.

If all FM channels are busy, the note on event is dropped.

## FM Presets

Sending a MIDI program change (0xC) message will select a pre-defined FM preset.
The full list of presets available are defined in
[`presets.h`](https://github.com/rhargreaves/mega-drive-midi-interface/blob/master/src/presets.h). They are based on [Wohlstand's XG bank from libOPNMIDI](https://github.com/Wohlstand/libOPNMIDI/blob/master/fm_banks/xg.wopn). The interface defaults to instrument 0 (Grand Piano) on start-up.

## System Real-Time Messages

| Status | Description           | Effect                 |
| ------ | --------------------- | ---------------------- |
| 0xF2   | Song Position Pointer | Set Beat Counter       |
| 0xF8   | Timing Clock          | Increment Beat Counter |
| 0xFA   | Start                 | Reset Beat Counter     |
| 0xFB   | Continue              | None                   |
| 0xFC   | Stop                  | None                   |

## Algorithm Operator Arrangement (CC 14)

Output operators are coloured blue. Operator 1 can also feedback into itself (see CC 15). Operators are numbered in the same order as the register placement (e.g. 0x30, 0x34, 0x38, 0x3C).

| Algorithm | Operator Arrangement                |
| --------- | ----------------------------------- |
| 0         | <img src="docs/algorithms/0.svg" /> |
| 1         | <img src="docs/algorithms/1.svg" /> |
| 2         | <img src="docs/algorithms/2.svg" /> |
| 3         | <img src="docs/algorithms/3.svg" /> |
| 4         | <img src="docs/algorithms/4.svg" /> |
| 5         | <img src="docs/algorithms/5.svg" /> |
| 6         | <img src="docs/algorithms/6.svg" /> |
| 7         | <img src="docs/algorithms/7.svg" /> |

_Note: Documentation on the YM2612 frequently muddles the second and third operators of algorithms 0 to 4 and 6. This is likely due to the original SEGA documentation incorrectly labelling the operator register addresses in sequential order, rather than 1st, 3rd, 2nd, 4th. That said, it seems to be generally common to keep the sequential order of the register addresses and simply correct the layout diagrams. This project favours this approach._

## System Exclusive

The interface supports the following SysEx messages:

| Name               | Sequence            | Description                                                                                                                    |
| ------------------ | ------------------- | ------------------------------------------------------------------------------------------------------------------------------ |
| General MIDI Reset | `7E 7F 09 01`       | Force all notes off on all channels                                                                                            |
| Remap MIDI Channel | `00 22 77 00 xx yy` | Remap MIDI channel _xx_ to device channel _yy_<br/>_xx_ = MIDI channel (0-15)<br/>_yy_ = FM (0-5), PSG (6-9), unassigned (127) |

All other messages are ignored.

## Build & Test

Unit & system tests are compiled and ran as x86 binaries using CMocka. Mega Drive interfaces are mocked.

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

Pull requests are welcome, as are donations!

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=4VY6LCUMYLD42&source=url)
