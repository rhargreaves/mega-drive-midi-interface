# Sega Mega Drive MIDI Interface
Exposes the YM2612 FM synth chip as a MIDI interface

## Getting Started

## Compiling ROM

Docker:

```sh
./docker-make all
```

Linux (requires `cmake`):

```sh
make all
```

## Running Unit Tests

Unit tests built with `cmocka` and are compiled to x86. Mega Drive interfaces are mocked out.

Docker:

```sh
./docker-make test
```

Linux:

```sh
make test
```
