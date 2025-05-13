vpath %.c ../src

CC=gcc
SGDK?=/sgdk
BIN_DIR=bin
OBJ_DIR=obj
CMOCKA_TAR=third_party/cmocka-1.1.7.tar.xz
CMOCKA_DIR=build/deps/cmocka

ifeq ($(DEBUG),1)
	GDB=gdbserver :2345
else
	GDB=
endif

INCLUDE_DIRS+=-I$(SGDK)/inc/ext
INCLUDE_DIRS+=-I$(SGDK)/inc
INCLUDE_DIRS+=-I$(SGDK)/res
INCLUDE_DIRS+=-I.
INCLUDE_DIRS+=-I../res
INCLUDE_DIRS+=-I../src
INCLUDE_DIRS+=-I../src/comm
INCLUDE_DIRS+=-Imocks
INCLUDE_DIRS+=-I$(CMOCKA_DIR)/include

CFLAGS=$(INCLUDE_DIRS) \
	-DBUILD='"$(BUILD)"' \
	-DUNIT_TESTS \
	-DMEGAWIFI \
	-ggdb \
	-O0 \
	-Wall \
	-std=c11 \
	-Werror \
	-Wextra \
	-Wno-char-subscripts \
	-Wno-unused-parameter \
	-Wno-builtin-declaration-mismatch \
	-Wno-unused-function \
	-fsigned-char


 # list of function signature to mock
MOCKS=midi_process \
	comm_init \
	comm_read \
	comm_write \
	comm_idle_count \
	comm_busy_count \
	comm_reset_counts \
	comm_read_ready \
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
	synth_operatorDecayRate \
	synth_operatorSustainRate \
	synth_operatorSustainLevel \
	synth_operatorAmplitudeModulation \
	synth_operatorReleaseRate \
	synth_operatorSsgEg \
	synth_enableLfo \
	synth_globalLfoFrequency \
	synth_preset \
	synth_volume \
	synth_channelParameters \
	synth_globalParameters \
	synth_setSpecialMode \
	synth_specialModePitch \
	synth_specialModeVolume \
	synth_directWriteYm2612 \
	synth_enableDac \
	midi_note_off \
	midi_note_on \
	midi_pitch_bend \
	midi_getPolyphonic \
	midi_cc \
	midi_program \
	midi_sysex \
	midi_mappings \
	midi_dynamic_mode \
	midi_channel_mappings \
	midi_psg_tick \
	midi_psg_load_envelope \
	midi_reset \
	log_init \
	log_info \
	log_warn \
	log_dequeue \
	ui_fm_set_parameters_visibility \
	scheduler_init \
	scheduler_tick \
	scheduler_addTickHandler \
	scheduler_addFrameHandler \
	comm_megawifi_midiEmitCallback \
	comm_megawifi_init \
	comm_megawifi_tick \
	comm_megawifi_send \
	settings_startup_midi_sequence

MD_MOCKS=SYS_setVIntCallback \
	VDP_setTextPalette \
	YM2612_writeReg \
	YM2612_write \
	VDP_drawText \
	VDP_clearText \
	VDP_setBackgroundColor \
	PAL_setColor \
	PAL_setColors \
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
	SPR_setFrame \
	SPR_setVisibility \
	comm_megawifi_is_present \
	comm_megawifi_read_ready \
	comm_megawifi_read \
	comm_megawifi_write_ready \
	comm_megawifi_write \
	comm_serial_init \
	comm_serial_is_present \
	comm_serial_read_ready \
	comm_serial_read \
	comm_serial_write_ready \
	comm_serial_write \
	comm_everdrive_init \
	comm_everdrive_is_present \
	comm_everdrive_read_ready \
	comm_everdrive_read \
	comm_everdrive_write_ready \
	comm_everdrive_write \
	comm_everdrive_pro_init \
	comm_everdrive_pro_is_present \
	comm_everdrive_pro_read_ready \
	comm_everdrive_pro_read \
	comm_everdrive_pro_write_ready \
	comm_everdrive_pro_write \
	comm_demo_init \
	comm_demo_is_present \
	comm_demo_read_ready \
	comm_demo_read \
	comm_demo_ready \
	comm_demo_write \
	comm_demo_vsync \
	VDP_clearTextArea \
	SYS_isPAL \
	SYS_die \
	mw_init \
	mw_process \
	lsd_process \
	mw_detect \
	loop_init \
	loop_func_add \
	mw_ap_assoc \
	mw_ap_assoc_wait \
	mw_ip_current \
	mw_udp_set \
	mw_sock_conn_wait \
	mw_send \
	lsd_recv \
	lsd_send \
	mw_udp_reuse_recv \
	mw_udp_reuse_send \
	SYS_doVBlankProcessEx \
	VDP_loadTileSet \
	VDP_setTileMapXY \
	VDP_drawImageEx \
	JOY_init \
	JOY_readJoypad \
	JOY_update \
	TSK_userSet \
	mw_def_ap_cfg_get \
	SND_PCM_startPlay \
	SND_PCM_stopPlay \
	Z80_loadDriver \
	Z80_releaseBus \
	Z80_requestBus \
	Z80_getAndRequestBus \
	SYS_doVBlankProcess \
	fix16ToInt \
	fix32ToInt \
	fix16Frac \
	fix32Frac \
	mw_uart_is_present

LDFLAGS=-ggdb -ffreestanding -lcmocka \
	-L$(CMOCKA_DIR)/build/src -Wl,-rpath=$(CMOCKA_DIR)/build/src \
	-fsanitize=address -fsanitize=undefined -fsanitize=leak
LDFLAGS+=$(foreach MOCK,$(MD_MOCKS),-Wl,--wrap=$(MOCK))
SYSTEM_TEST_LDFLAGS=$(LDFLAGS)
UNIT_TEST_LDFLAGS=$(LDFLAGS) $(foreach MOCK,$(MOCKS),-Wl,--wrap=$(MOCK))

LAST_FLAGS := bin/last_flags
CHECK_FLAGS := $(LAST_FLAGS)_$(shell echo $(CFLAGS) \
	$(SYSTEM_TEST_LDFLAGS) $(UNIT_TEST_LDFLAGS) | md5sum | awk '{ print $$1 }')

SRC=$(wildcard ../src/*.c ../src/*/*.c)
SRC:=$(filter-out ../src/boot/rom_head.c,$(SRC))
SRC:=$(filter-out ../src/main.c,$(SRC))
COMMON_TEST_SRC=$(wildcard *.c mocks/*.c)
UNIT_TEST_SRC=$(wildcard unit/*.c)
SYSTEM_TEST_SRC=$(wildcard system/*.c)

SRC_OBJ=$(patsubst ../src/%.c,obj/%.o,$(SRC))
COMMON_TEST_OBJ=$(patsubst %.c,obj/%.o,$(COMMON_TEST_SRC))
UNIT_TEST_OBJ=$(patsubst %.c,obj/%.o,$(UNIT_TEST_SRC))
SYSTEM_TEST_OBJ=$(patsubst %.c,obj/%.o,$(SYSTEM_TEST_SRC))

UNIT_TESTS_TARGET=$(BIN_DIR)/unit_tests
SYSTEM_TESTS_TARGET=$(BIN_DIR)/system_tests

all: unit system
.PHONY: all

unit: $(UNIT_TESTS_TARGET) $(CHECK_FLAGS)
	$(GDB) ./$(UNIT_TESTS_TARGET)
.PHONY: unit

system: $(SYSTEM_TESTS_TARGET) $(CHECK_FLAGS)
	$(GDB) ./$(SYSTEM_TESTS_TARGET)
.PHONY: system

$(OBJ_DIR)/%.o: %.c | $(CMOCKA_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MT "$@ $(basename $@).d" -c $< -o $@

-include $(wildcard $(OBJ_DIR)/*.d) $(wildcard $(OBJ_DIR)/*/*.d)

$(UNIT_TESTS_TARGET): $(SRC_OBJ) $(UNIT_TEST_OBJ) $(COMMON_TEST_OBJ) $(CHECK_FLAGS)
	@mkdir -p $(dir $@)
	$(CC) -o $@ $^ $(UNIT_TEST_LDFLAGS)

$(SYSTEM_TESTS_TARGET): $(SRC_OBJ) $(SYSTEM_TEST_OBJ) $(COMMON_TEST_OBJ) $(CHECK_FLAGS)
	@mkdir -p $(dir $@)
	$(CC) -o $@ $^ $(SYSTEM_TEST_LDFLAGS)

$(CMOCKA_DIR):
	mkdir -p $@
	tar -xf $(CMOCKA_TAR) -C $@ --strip-components 1 --no-same-owner
	mkdir -p $@/build
	@(cd $@/build && cmake ../ && make -s)

$(LAST_FLAGS)_%:
	-rm $(LAST_FLAGS)_*
	touch $@

clean-target:
	rm -rf $(UNIT_TESTS_TARGET) $(SYSTEM_TESTS_TARGET) $(OBJ_DIR)
.PHONY: clean-target

clean: clean-target
	rm -rf build/
.PHONY: clean
