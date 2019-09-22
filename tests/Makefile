vpath %.c ../src

CC=gcc
SGDK?=/opt/gendev/sgdk

INCLUDE_DIRS+=-I$(SGDK)/inc
INCLUDE_DIRS+=-I$(SGDK)/res
INCLUDE_DIRS+=-I.
INCLUDE_DIRS+=-I../res
INCLUDE_DIRS+=-I../src
INCLUDE_DIRS+=-I$(CMOCKA_DIR)/include

CFLAGS=$(INCLUDE_DIRS) \
	-Wall \
	-std=c11 \
	-Werror \
	-Wno-builtin-declaration-mismatch \
	-Wno-unused-function

 # list of function signature to mock
MOCKS=midi_process \
	comm_read \
	comm_write \
	comm_idleCount \
	comm_busyCount \
	comm_resetCounts \
	synth_init \
	synth_noteOn \
	synth_noteOff \
	synth_pitch \
	synth_stereo \
	synth_algorithm \
	synth_feedback \
	synth_ams \
	synth_fms \
	synth_operatorTotalLevel \
	synth_operatorMultiple \
	synth_operatorDetune \
	synth_operatorRateScaling \
	synth_operatorAttackRate \
	synth_operatorFirstDecayRate \
	synth_operatorSecondDecayRate \
	synth_operatorSecondaryAmplitude \
	synth_operatorAmplitudeModulation \
	synth_operatorReleaseRate \
	synth_operatorSsgEg \
	synth_enableLfo \
	synth_globalLfoFrequency \
	synth_preset \
	synth_volume \
	fm_writeReg \
	psg_noteOn \
	psg_noteOff \
	psg_attenuation \
	psg_frequency \
	midi_noteOff \
	midi_noteOn \
	midi_pitchBend \
	midi_getPolyphonic \
	midi_cc \
	midi_clock \
	midi_start \
	midi_position \
	midi_timing \
	midi_program \
	midi_sysex \
	midi_mappings

MD_MOCKS=SYS_setVIntCallback \
	VDP_setTextPalette \
	YM2612_writeReg \
	VDP_drawText \
	VDP_clearText \
	VDP_setBackgroundColor \
	VDP_setPaletteColor \
	PSG_setEnvelope \
	PSG_setFrequency \
	SPR_addSprite \
	SPR_update \
	SYS_disableInts \
	SYS_enableInts \
	SPR_init \
	SPR_setAnim \
	SPR_setFrame \
	SPR_setAnimAndFrame \
	VDP_setPaletteColors \
	ssf_usb_read \
    ssf_usb_rd_ready \
	ssf_usb_write \
	ssf_usb_wr_ready

LDFLAGS=-ffreestanding -l cmocka -L$(CMOCKA_DIR)/build/src -Wl,-rpath=$(CMOCKA_DIR)/build/src
LDFLAGS+=$(foreach MOCK,$(MD_MOCKS),-Wl,--wrap=$(MOCK))

SYSTEM_TEST_LDFLAGS=$(LDFLAGS)
UNIT_TEST_LDFLAGS=$(LDFLAGS) $(foreach MOCK,$(MOCKS),-Wl,--wrap=$(MOCK))

SRC=$(shell find ../src/* -type f -name '*.c' ! -name '*main.c*' -printf '%f\n')
COMMON_TEST_SRC=$(shell find * -maxdepth 0 -type f -name '*.c' -print)
UNIT_TEST_SRC=$(shell find ./unit/* -maxdepth 0 -type f -name '*.c' -print)
SYSTEM_TEST_SRC=$(shell find ./system/* -maxdepth 0 -type f -name '*.c' -print)

SRC_OBJ=$(patsubst %.c,obj/%.o,$(SRC))
COMMON_TEST_OBJ=$(patsubst %.c,obj/%.o,$(COMMON_TEST_SRC))
UNIT_TEST_OBJ=$(patsubst %.c,obj/%.o,$(UNIT_TEST_SRC))
SYSTEM_TEST_OBJ=$(patsubst %.c,obj/%.o,$(SYSTEM_TEST_SRC))
BIN_DIR=bin
OBJ_DIR=obj

CMOCKA_TAR=cmocka-1.1.2.tar.xz
CMOCKA_DIR=cmocka

UNIT_TESTS_TARGET=$(BIN_DIR)/unit_tests
SYSTEM_TESTS_TARGET=$(BIN_DIR)/system_tests

all: unit system

unit: $(UNIT_TESTS_TARGET)
	./$(UNIT_TESTS_TARGET)
.PHONY: unit

system: $(SYSTEM_TESTS_TARGET)
	./$(SYSTEM_TESTS_TARGET)
.PHONY: system

$(SRC_OBJ): | $(OBJ_DIR)

$(OBJ_DIR):
	mkdir -p $@
	mkdir -p $@/unit
	mkdir -p $@/system

$(OBJ_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(UNIT_TESTS_TARGET): $(CMOCKA_DIR) $(SRC_OBJ) $(UNIT_TEST_OBJ) $(COMMON_TEST_OBJ)
	mkdir -p $(BIN_DIR)
	$(CC) -o $@ $^ $(UNIT_TEST_LDFLAGS)

$(SYSTEM_TESTS_TARGET): $(CMOCKA_DIR) $(SRC_OBJ) $(SYSTEM_TEST_OBJ) $(COMMON_TEST_OBJ)
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
