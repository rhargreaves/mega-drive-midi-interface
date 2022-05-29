SGDK=/sgdk
MAKE=make

all: release test

include $(SGDK)/makefile.gen

EXTRA_FLAGS:=-I/sgdk/inc/ext/mw -DMODULE_MEGAWIFI=1

unit-test:
	$(MAKE) -C tests clean-target unit
.PHONY: unit-test

test:
	$(MAKE) -C tests
.PHONY: test
