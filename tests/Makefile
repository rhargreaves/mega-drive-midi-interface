vpath %.c ../src

CC=gcc
SGDK?=/opt/gendev/sgdk

INCLUDE_DIRS+=-I$(SGDK)/inc
INCLUDE_DIRS+=-I$(SGDK)/res
INCLUDE_DIRS+=-I.
INCLUDE_DIRS+=-I../src
INCLUDE_DIRS+=-I$(CMOCKA_DIR)/include

CFLAGS=$(INCLUDE_DIRS) -Wno-builtin-declaration-mismatch

 # list of function signature to mock
MOCKS=midi_process \
	comm_read \
	comm_idleCount \
	comm_busyCount \
	comm_resetCounts \
	synth_init \
	synth_noteOn \
	synth_noteOff \
	synth_pitch \
	synth_totalLevel \
	synth_stereo \
	synth_algorithm \
	synth_feedback \
	synth_operatorTotalLevel \
	synth_operatorMultiple \
	synth_operatorDetune \
	synth_operatorRateScaling \
	synth_operatorAttackRate \
	synth_operatorFirstDecayRate \
	synth_operatorSecondDecayRate \
	synth_operatorSecondaryAmplitude \
	synth_operatorAmplitudeModulation \
	fm_writeReg \
	psg_noteOn \
	psg_noteOff \
	psg_attenuation \
	midi_noteOff \
	midi_noteOn \
	midi_channelVolume \
	midi_pan

MD_MOCKS=SYS_setVIntCallback \
	VDP_setTextPalette \
	YM2612_writeReg \
	VDP_drawText \
	VDP_clearText \
	PSG_setEnvelope \
	PSG_setFrequency \
	ssf_usb_read \
    ssf_usb_rd_ready

LDFLAGS=-ffreestanding -l cmocka -L$(CMOCKA_DIR)/build/src -Wl,-rpath=$(CMOCKA_DIR)/build/src
LDFLAGS+=$(foreach MOCK,$(MD_MOCKS),-Wl,--wrap=$(MOCK))

SYSTEM_TEST_LDFLAGS=$(LDFLAGS)
UNIT_TEST_LDFLAGS=$(LDFLAGS) $(foreach MOCK,$(MOCKS),-Wl,--wrap=$(MOCK))

SRC=$(shell find ../src/* -type f -name '*.c' ! -name '*main.c*' -printf '%f\n')
COMMON_TEST_SRC=$(shell find * -maxdepth 0 -type f -name '*.c' -print)
UNIT_TEST_SRC=$(COMMON_TEST_SRC) $(shell find ./unit/* -maxdepth 0 -type f -name '*.c' -print)
SYSTEM_TEST_SRC=$(COMMON_TEST_SRC) $(shell find ./system/* -maxdepth 0 -type f -name '*.c' -print)

SRC_OBJ=$(patsubst %.c,obj/%.o,$(SRC))
UNIT_TEST_OBJ=$(patsubst %.c,obj/%.o,$(UNIT_TEST_SRC))
SYSTEM_TEST_OBJ=$(patsubst %.c,obj/%.o,$(SYSTEM_TEST_SRC))
BIN_DIR=bin
OBJ_DIR=obj

CMOCKA_TAR=cmocka-1.1.2.tar.xz
CMOCKA_DIR=cmocka

UNIT_TESTS_TARGET=$(BIN_DIR)/unit_tests
SYSTEM_TESTS_TARGET=$(BIN_DIR)/system_tests

all: unit system

unit: $(CMOCKA_DIR) $(UNIT_TESTS_TARGET)
	./$(UNIT_TESTS_TARGET)
.PHONY: unit

system: $(CMOCKA_DIR) $(SYSTEM_TESTS_TARGET)
	./$(SYSTEM_TESTS_TARGET)
.PHONY: system

$(SRC_OBJ): | $(OBJ_DIR)

$(OBJ_DIR):
	mkdir -p $@
	mkdir -p $@/unit
	mkdir -p $@/system

$(OBJ_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(UNIT_TESTS_TARGET): $(SRC_OBJ) $(UNIT_TEST_OBJ)
	mkdir -p $(BIN_DIR)
	$(CC) -o $@ $^ $(UNIT_TEST_LDFLAGS)

$(SYSTEM_TESTS_TARGET): $(SRC_OBJ) $(SYSTEM_TEST_OBJ)
	mkdir -p $(BIN_DIR)
	$(CC) -o $@ $^ $(SYSTEM_TEST_LDFLAGS)

$(CMOCKA_DIR):
	mkdir -p $@
	tar -xf $(CMOCKA_TAR) -C $@ --strip-components 1
	mkdir -p $@/build
	@(cd $@/build && cmake ../ && make -s)

clean-target:
	rm -rf $(TARGET) $(OBJ_DIR)

clean: clean-target
	rm -rf $(CMOCKA_DIR)
