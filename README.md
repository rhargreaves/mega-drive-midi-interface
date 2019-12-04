# Sega Mega Drive MIDI Interface [![CircleCI](https://circleci.com/gh/rhargreaves/mega-drive-midi-interface.svg?style=svg)](https://circleci.com/gh/rhargreaves/mega-drive-midi-interface) [![GitHub release (latest by date)](https://img.shields.io/github/v/release/rhargreaves/mega-drive-midi-interface?style=plastic)](https://github.com/rhargreaves/mega-drive-midi-interface/releases)

Control the Yamaha YM2612 and PSG chips of the SEGA Mega Drive via MIDI.

<p align="center">
    <img src="https://github.com/rhargreaves/mega-drive-midi-interface/raw/master/docs/screenshot.png" width="600" />
</p>

## Features

- Built-in FM presets for General MIDI compatibility
- Supports MIDI 1.0 CCs & events (e.g. panning, volume, pitch bending)
- Polythonic support via FM channel pooling
- Connectivity via:
  - Mega Everdrive's X7 USB port
  - [Custom-built USB cable connected to a controller port](https://github.com/rhargreaves/mega-drive-serial-port#hardware).
- Fine-grained control of YM2612 registers via [GenMDM-style CCs](https://catskullelectronics.com/public/genMDM.pdf)

## Getting Started

### Hardware Requirements

- SEGA Mega Drive/Genesis Console (Model 1 or 2)

Either:

- [Mega EverDrive X7 cart](https://krikzz.com/store/home/33-mega-everdrive-v2.html)
- USB cable for connecting your PC to the EverDrive X7.

Or:

- Generic flash cart for loading the ROM
- USB serial cable for connectivity into Controller Port 2, [based on these specifications](https://github.com/rhargreaves/mega-drive-serial-port#hardware)

**Due to speed limitations on the controller port serial interface, EverDrive X7 USB connectivity is the recommended option, but connectivity via the controller port might be fine for simple use cases.**

### Software Requirements

- MIDI to serial port virtual device, e.g. [Hairless MIDI<->Serial Bridge](http://projectgus.github.io/hairless-midiserial/)
- Optional: MIDI loop device (so software running on the same PC as the USB connection can use the MIDI interface). In macOS this is possible via the use of a ["IAC Device Bus" creatable from the Audio MIDI Setup utility](https://help.ableton.com/hc/en-us/articles/209774225-Using-virtual-MIDI-buses).

### Download

You can download pre-built ROMs from [releases](https://github.com/rhargreaves/mega-drive-midi-interface/releases).

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

## FM Presets

Sending a MIDI program change (0xC) message will select a pre-defined FM preset.
The full list of presets available are defined in
[`presets.h`](https://github.com/rhargreaves/mega-drive-midi-interface/blob/master/src/presets.h). They are based on [Wohlstand's XG bank from libOPNMIDI](https://github.com/Wohlstand/libOPNMIDI/blob/master/fm_banks/xg.wopn). The interface defaults all FM channels to instrument 0 (Grand Piano) on start-up.

If MIDI channel 10 is mapped to an FM channel, the interface will make use of a separate bank of percussion instruments. An an example, to map MIDI channel 10 to FM channel 6, use the SysEx sequence `00 22 77 00 09 05`. Note that by default MIDI channel 10 is set to the PSG noise channel.

## Polyphonic Mode

When polyphonic mode is enabled (CC 80), all note on/off events are routed to a pool of
FM channels, ignoring the specific MIDI channel the event is sent to. This allows for
polyphony within a single MIDI channel. In addition, any FM parameter change made will be sent to all FM channels. If all FM channels are busy, the note on event is dropped.

## Dynamic Mapping Mode

When dynamic mapping mode is enabled (SysEx `00 22 77 03 01`), MIDI channel note-on/off events are dynamically routed to free FM and PSG channels. That is, MIDI channels no-longer map directly onto device channels but are virtualised and note-on/off events and MIDI program data is set on the next available channel. This mode is best suited for playback of General MIDI files and makes full use of available YM2612/PSG capacity.

This mode is a work-in-progress.

The following rules are used to determine which device channel receives the MIDI event:

1. FM channels 1 - 6 and PSG channels 1 - 3 (device channels) are included in the pool of available channels.
   The first available channel is assigned the first MIDI note on event. Future events are sent to this channel.
2. On subsequent events, if the note is already playing on that channel, the next available device channel is used.
3. The following MIDI parameters are automatically set on any mapped device channels:
   - Program
   - Volume
   - Pan
4.

## MIDI Message Reference

### Channel Mappings

By default, MIDI channels are assigned in a one-to-one arrangement to FM or PSG channels as follows:

| Channels | Assignment                  |
| -------- | --------------------------- |
| 1 - 6    | YM2612 FM Channels 1 - 6    |
| 7 - 9    | PSG Square Wave Tones 1 - 3 |
| 10       | PSG Noise Channel           |
| 11 - 13  | As 7 - 9                    |
| 14 - 16  | As 7 - 9                    |

You can also [re-configure the MIDI mappings](#system-exclusive) via SysEx

### Events

- Note On/Off
- Pitch Bend
- Program Change (FM only: selects preset)
- Universal SysEx Messages

### Common MIDI CCs

These are supported across FM and PSG channels:

| CC  | Description    | Effect                     | Values                                             |
| --- | -------------- | -------------------------- | -------------------------------------------------- |
| 7   | Channel Volume | FM: Output Op. Total Level | 0 - 127: [Logarithmic](src/midi.c#L24)             |
|     |                | PSG: Attenuation           | 0 - 127: [Logarithmic](src/midi.c#L45)             |
| 10  | Panning        | Stereo                     | 0 - 31: Left<br>32 - 96: Centre<br>97 - 127: Right |
| 123 | All Notes Off  | FM: Key Off                | 0                                                  |
|     |                | PSG: Max. Attenuation      | 0                                                  |

### FM Parameters

These only apply to channels mapped to FM channels:

#### Global

| CC  | Description       | Range\* |
| --- | ----------------- | ------- |
| 74  | LFO Enable        | 2       |
| 1   | LFO Frequency     | 8       |
| 80  | Polyphonic Mode\* | 2       |

_Range determines how the possible 128 MIDI values are divided to give the respective YM2612 register value, using the formula `_midiValue / (128 / range) = registerValue` (e.g. MIDI value of 32, with a range of 8 translates into to a YM2612 register value of 2)_

#### Per Channel

| CC  | Description                      | Range |
| --- | -------------------------------- | ----- |
| 14  | Algorithm                        | 8     |
| 15  | Feedback                         | 8     |
| 75  | Frequency Modulation Level (FMS) | 8     |
| 76  | Amplitude Modulation Level (AMS) | 4     |
| 77  | Stereo                           | 4     |

#### Per Channel & Operator

| CCs   | Description               | Range |
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

### FM Algorithm Operator Routing

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

### System Real-Time Messages

| Status | Description           | Effect                 |
| ------ | --------------------- | ---------------------- |
| 0xF2   | Song Position Pointer | Set Beat Counter       |
| 0xF8   | Timing Clock          | Increment Beat Counter |
| 0xFA   | Start                 | Reset Beat Counter     |
| 0xFB   | Continue              | None                   |
| 0xFC   | Stop                  | None                   |

### System Exclusive Messages

| Name                     | Sequence            | Description                                                                                                                                                      |
| ------------------------ | ------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| General MIDI Reset       | `7E 7F 09 01`       | Force all notes off on all channels                                                                                                                              |
| Remap MIDI Channel       | `00 22 77 00 xx yy` | Remap MIDI channel _xx_ to device channel _yy_<br/>_xx_ = MIDI channel (0-15)<br/>_yy_ = FM (0-5), PSG (6-9), unassigned (127)                                   |
| Ping                     | `00 22 77 01`       | Interface responds with a _pong_ SysEx reply (`00 22 77 02`). Intended for use in measuring MIDI round-trip latency.                                             |
| Dynamic Channelling Mode | `00 22 77 03 xx`    | Dynamically assigns MIDI channels to idle FM/PSG channels to allow for maximum polyphony and variation in instrumentation.<br/>_xx_ = Enable (01) / Disable (00) |

All other messages are ignored.

## Performance

Releases are tested for performance using the [Mega MIDI Ping Pong](https://github.com/rhargreaves/mega-midi-ping-pong) tool which makes use of the "Ping" SysEx message to measure round-trip time.

[Test results](https://github.com/rhargreaves/mega-midi-ping-pong#results)

## Stretch Goals

- Support for MIDI messages to be received via the controller ports (in serial mode) as an alternative to the EverDrive X7.
- Support for loading & playback of PCM samples.

## Contributions

Pull requests are welcome, as are donations!

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=4VY6LCUMYLD42&source=url)
