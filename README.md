# Sega Mega Drive MIDI Interface [![CircleCI](https://circleci.com/gh/rhargreaves/mega-drive-midi-interface.svg?style=svg)](https://circleci.com/gh/rhargreaves/mega-drive-midi-interface) [![GitHub release (latest by date)](https://img.shields.io/github/v/release/rhargreaves/mega-drive-midi-interface?style=plastic)](https://github.com/rhargreaves/mega-drive-midi-interface/releases)

Control the Yamaha YM2612 and PSG chips of the SEGA Mega Drive via MIDI.

<p align="center">
    <img src="https://github.com/rhargreaves/mega-drive-midi-interface/raw/master/docs/screenshot.jpg" width="600" />
</p>

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

## Getting Started

### Hardware Requirements

- SEGA Mega Drive/Genesis Console (Model 1 or 2, NTSC or PAL)

Either:

- [Mega EverDrive X7 cart](https://krikzz.com/store/home/33-mega-everdrive-v2.html)
- USB cable for connecting your PC to the EverDrive X7.

Or:

- Generic flash cart for loading the ROM
- USB serial cable for connectivity into Controller Port 2, [based on these specifications](https://github.com/rhargreaves/mega-drive-serial-port#hardware)

**Whilst connectivity via the controller port might be fine for simple use cases, the controller port's relatively slow speed makes it unsuitable for playback of busy MIDI sequences. EverDrive X7 USB connectivity is the recommended option for these cases.**

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

## Features

### FM Presets

Sending a MIDI program change (0xC) message will select a pre-defined FM preset.
The full list of presets available are defined in
[`presets.h`](https://github.com/rhargreaves/mega-drive-midi-interface/blob/master/src/presets.h). They are based on [Wohlstand's XG bank from libOPNMIDI](https://github.com/Wohlstand/libOPNMIDI/blob/master/fm_banks/xg.wopn). The interface defaults all FM channels to instrument 0 (Grand Piano) on start-up.

If MIDI channel 10 is mapped to an FM channel, the interface will make use of a separate bank of percussion instruments. An an example, to map MIDI channel 10 to FM channel 6, use the SysEx sequence `00 22 77 00 09 05`. Note that by default MIDI channel 10 is set to the PSG noise channel.

### Dynamic Mapping Mode

When dynamic mapping mode is enabled (SysEx `00 22 77 03 01`), MIDI channel note-on/off events are dynamically routed to free FM and PSG channels. That is, MIDI channels no-longer map directly onto device channels but are virtualised and note-on/off events and MIDI program data is set on the next available channel. This mode is best suited for playback of General MIDI files and makes full use of available YM2612/PSG capacity.

The following rules are used to determine which device channel receives the MIDI event:

1. FM channels 1 - 6 and PSG channels 1 - 3 (device channels) are included in the pool of available channels.
   The first available channel is assigned the first MIDI note on event. Future events are sent to this channel.
2. On subsequent events, if the note is already playing on that channel, the next available device channel is used.
3. The following MIDI parameters are automatically set on any mapped device channels:
   - Program
   - Volume
   - Pan

### Show FM Channel Parameters on UI

You can use CC 83 to show the current FM parameters for the specific channel:

<p align="center">
    <img src="https://github.com/rhargreaves/mega-drive-midi-interface/raw/master/docs/chan_paras.jpg" width="600" />
</p>

## Reference

Head over to the [Wiki](https://github.com/rhargreaves/mega-drive-midi-interface/wiki/MIDI-Message-Reference) for a detailed list of MIDI messages that can be sent to the interface.

## Channel Mappings

By default, MIDI channels are assigned in a static, one-to-one arrangement to FM or PSG channels as follows:

| Channels | Assignment                  |
| -------- | --------------------------- |
| 1 - 6    | YM2612 FM Channels 1 - 6    |
| 7 - 9    | PSG Square Wave Tones 1 - 3 |
| 10       | PSG Noise Channel           |

You can also [re-configure the MIDI mappings](#system-exclusive) via SysEx

### Polyphonic Mode

When polyphonic mode is enabled (CC 80), dynamic mapping mode is enabled and MIDI channel 1 is mapped to FM channels 1-6. This allows for polyphony within a single MIDI channel. In addition, any FM parameter change made will be sent to all FM channels. If all channels are busy, the note on event is dropped.

## Performance

Releases are tested for performance using the [Mega MIDI Ping Pong](https://github.com/rhargreaves/mega-midi-ping-pong) tool which makes use of the "Ping" SysEx message to measure round-trip time.

[Test results](https://github.com/rhargreaves/mega-midi-ping-pong#results)

## Stretch Goals

- Support for loading & playback of PCM samples.
- Support for loading & saving of FM/PSG presets via SD card or SysEx.

## Contributions

Pull requests are welcome, as are donations!

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=4VY6LCUMYLD42&source=url)
