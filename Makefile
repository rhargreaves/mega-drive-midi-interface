SGDK=/sgdk
MAKE=make

all: release out/rom.s test

include $(SGDK)/makefile.gen

EXTRA_FLAGS:=-I/sgdk/inc/ext/mw \
    -DMODULE_MEGAWIFI=1 \
    -DBUILD='"$(BUILD)"'

ifeq ($(ROM_TYPE), MEGAWIFI)
	EXTRA_FLAGS += -DMEGAWIFI -DENABLE_MEGAWIFI=1
endif

out/rom.s: out/rom.out
	m68k-elf-objdump -S $^ > $@

unit-test:
	$(MAKE) -C tests clean-target unit
.PHONY: unit-test

test:
	$(MAKE) -C tests
.PHONY: test
