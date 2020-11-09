vpath %.c ../src

CC=gcc
SGDK?=/opt/gendev/sgdk

ifeq ($(DEBUG),1)
	GDB=gdbserver host:2345
else
	GDB=
endif

INCLUDE_DIRS+=-I$(SGDK)/inc
INCLUDE_DIRS+=-I$(SGDK)/res
INCLUDE_DIRS+=-I.
INCLUDE_DIRS+=-I../res
INCLUDE_DIRS+=-I../src
INCLUDE_DIRS+=-I../src/mw
INCLUDE_DIRS+=-I$(CMOCKA_DIR)/include

CFLAGS=$(INCLUDE_DIRS) \
	-DUNIT_TESTS \
	-ggdb \
	-O0 \
	-Wall \
	-std=c11 \
	-Werror \
	-Wextra \
	-Wno-char-subscripts \
	-Wno-unused-parameter \
	-Wno-builtin-declaration-mismatch \
	-Wno-unused-function

 # list of function signature to mock
MOCKS=midi_process \
	comm_init \
	comm_read \
	comm_write \
	comm_idleCount \
	comm_busyCount \
	comm_resetCounts \
	comm_readReady \
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
	synth_channelParameters \
	synth_globalParameters \
	fm_writeReg \
	midi_noteOff \
	midi_noteOn \
	midi_pitchBend \
	midi_getPolyphonic \
	midi_cc \
	midi_program \
	midi_sysex \
	midi_mappings \
	midi_dynamicMode \
	midi_channelMappings \
	midi_psg_tick \
	midi_psg_loadEnvelope \
	log_init \
	log_info \
	log_warn \
	log_dequeue \
	ui_fm_setMidiChannelParametersVisibility \
	ui_update \
	scheduler_init \
	midi_emit \
	comm_megawifi_init \
	mediator_recv_event \
	mediator_send_packet

MD_MOCKS=SYS_setVIntCallback \
	VDP_setTextPalette \
	YM2612_writeReg \
	VDP_drawText \
	VDP_clearText \
	VDP_setBackgroundColor \
	VDP_setPaletteColor \
	PSG_setEnvelope \
	PSG_setTone \
	SPR_addSprite \
	SPR_update \
	SYS_disableInts \
	SYS_enableInts \
	VDP_setReg \
	VDP_getReg \
	SYS_setExtIntCallback \
	SYS_setInterruptMaskLevel \
	SPR_init \
	SPR_setAnim \
	SPR_setFrame \
	SPR_setAnimAndFrame \
	SPR_setVisibility \
	VDP_setPaletteColors \
	comm_serial_init \
	comm_serial_readReady \
	comm_serial_read \
	comm_serial_writeReady \
	comm_serial_write \
	comm_everdrive_init \
	comm_everdrive_readReady \
	comm_everdrive_read \
	comm_everdrive_writeReady \
	comm_everdrive_write \
	comm_everdrive_pro_init \
	comm_everdrive_pro_readReady \
	comm_everdrive_pro_read \
	comm_everdrive_pro_writeReady \
	comm_everdrive_pro_write \
	SYS_getCPULoad \
	getFPS \
	VDP_clearTextArea \
	region_isPal \
	SYS_die \
	mw_init \
	mw_process

LDFLAGS=-ggdb -ffreestanding -lcmocka -L$(CMOCKA_DIR)/build/src -Wl,-rpath=$(CMOCKA_DIR)/build/src
LDFLAGS+=$(foreach MOCK,$(MD_MOCKS),-Wl,--wrap=$(MOCK))

SYSTEM_TEST_LDFLAGS=$(LDFLAGS)
UNIT_TEST_LDFLAGS=$(LDFLAGS) $(foreach MOCK,$(MOCKS),-Wl,--wrap=$(MOCK))

SRC=$(shell find ../src/* -maxdepth 0 -type f -name '*.c' ! -name '*main.c*' -printf '%f\n')
SRC+=$(shell find ../src/mw/* -maxdepth 0 -type f -name '*.c' ! -name '*main.c*' -printf 'mw/%f\n')
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

all: clean-target unit system

unit: $(UNIT_TESTS_TARGET)
	$(GDB) ./$(UNIT_TESTS_TARGET)
.PHONY: unit

system: $(SYSTEM_TESTS_TARGET)
	$(GDB) ./$(SYSTEM_TESTS_TARGET)
.PHONY: system

$(SRC_OBJ): | $(OBJ_DIR) $(CMOCKA_DIR)

$(OBJ_DIR):
	mkdir -p $@
	mkdir -p $@/mw
	mkdir -p $@/unit
	mkdir -p $@/system

$(OBJ_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(UNIT_TESTS_TARGET): $(SRC_OBJ) $(UNIT_TEST_OBJ) $(COMMON_TEST_OBJ)
	mkdir -p $(BIN_DIR)
	$(CC) -o $@ $^ $(UNIT_TEST_LDFLAGS)

$(SYSTEM_TESTS_TARGET): $(SRC_OBJ) $(SYSTEM_TEST_OBJ) $(COMMON_TEST_OBJ)
	mkdir -p $(BIN_DIR)
	$(CC) -o $@ $^ $(SYSTEM_TEST_LDFLAGS)

$(CMOCKA_DIR):
	mkdir -p $@
	tar -xf $(CMOCKA_TAR) -C $@ --strip-components 1
	mkdir -p $@/build
	@(cd $@/build && cmake ../ && make -s)

clean-target:
	rm -rf $(UNIT_TESTS_TARGET) $(SYSTEM_TESTS_TARGET) $(OBJ_DIR)

clean: clean-target
	rm -rf $(CMOCKA_DIR)
