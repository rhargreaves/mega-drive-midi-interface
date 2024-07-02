SGDK=/sgdk
MAKE=make

all: release out/rom.s test

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

release: FLAGS= $(DEFAULT_FLAGS) -O3 -fuse-linker-plugin -fno-web -fno-gcse \
	-fno-unit-at-a-time -fomit-frame-pointer $(LTO_FLAGS)
release: LIBMD= $(LIB)/libmd.a
release: pre-build out/rom.bin out/symbol.txt

out/rom.s: out/rom.out
	m68k-elf-objdump -S $^ > $@

unit-test:
	$(MAKE) -C tests clean-target unit
.PHONY: unit-test

test:
	$(MAKE) -C tests
.PHONY: test
