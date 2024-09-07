SGDK=/sgdk
MAKE=make
BUILD?=$(shell git rev-parse --short HEAD)

all: release test

release: res/samples

include $(SGDK)/makefile.gen

EXTRA_FLAGS:=-DMODULE_MEGAWIFI=1 \
	-DBUILD='"$(BUILD)"' \
	-Wl,--wrap=SYS_enableInts \
	-Wl,--wrap=SYS_disableInts

ifeq ($(ROM_TYPE), MEGAWIFI)
	EXTRA_FLAGS += -DMEGAWIFI -DENABLE_MEGAWIFI=1
	LTO_FLAGS:=
else
	LTO_FLAGS:=-flto
endif

res/samples:
	wget "https://github.com/rhargreaves/mega-drive-pcm-samples/releases/download/v1/samples.zip" \
		-O temp.zip
	unzip temp.zip -d res/samples
	rm temp.zip

debug: FLAGS= $(DEFAULT_FLAGS) -O1 -DDEBUG=1
debug: CFLAGS= $(FLAGS) -ggdb
debug: AFLAGS= $(FLAGS)
debug: LIBMD= $(LIB)/libmd_debug.a
debug: pre-build out/rom.bin out/rom.out out/symbol.txt

release: FLAGS= $(DEFAULT_FLAGS) -O3 -fuse-linker-plugin -fno-web -fno-gcse \
	-fno-unit-at-a-time -fomit-frame-pointer $(LTO_FLAGS)
release: LIBMD= $(LIB)/libmd.a
release: pre-build out/rom.bin out/symbol.txt

clean: cleanobj cleanres cleanlst cleandep
	$(RM) -f out.lst out/cmd_ out/symbol.txt out/rom.nm out/rom.wch out/rom.bin out/rom.s

out/rom.s: out/rom.out
	m68k-elf-objdump -D -S $^ > $@

unit-test:
	$(MAKE) -C tests clean-target unit
.PHONY: unit-test

test:
	$(MAKE) -C tests
.PHONY: test
