vpath %.c ../src

CC=gcc
SGDK?=/opt/gendev/sgdk

INCLUDE_DIRS+=-I$(SGDK)/inc
INCLUDE_DIRS+=-I$(SGDK)/res
INCLUDE_DIRS+=-I.
INCLUDE_DIRS+=-I../inc
INCLUDE_DIRS+=-I../src
INCLUDE_DIRS+=-I../include
INCLUDE_DIRS+=-I$(CMOCKA_DIR)/include

CFLAGS=$(INCLUDE_DIRS) -Wno-builtin-declaration-mismatch

 # list of function signature to mock
MOCKS=midi_process \
	comm_read \
	synth_init \
	synth_noteOn \
	synth_noteOff \
	synth_pitch \
	synth_totalLevel \
	fm_writeReg \
	SYS_setVIntCallback \
	VDP_setTextPalette \
	YM2612_writeReg \
	VDP_drawText \
	VDP_clearText \
	midi_noteOff \
	midi_noteOn

LDFLAGS=-ffreestanding -l cmocka -L$(CMOCKA_DIR)/build/src -Wl,-rpath=$(CMOCKA_DIR)/build/src
LDFLAGS+=$(foreach MOCK,$(MOCKS),-Wl,--wrap=$(MOCK))

TARGET=test_main

SRC=$(shell find ../src/* -type f -name '*.c' ! -name '*main.c*' -printf '%f\n')
TEST_SRC=$(shell find * -maxdepth 0 -type f -name '*.c' -print)

SRC_OBJ=$(patsubst %.c,obj/%.o,$(SRC))
TEST_OBJ=$(patsubst %.c,obj/%.o,$(TEST_SRC))
OBJ_DIR=obj

CMOCKA_TAR=cmocka-1.1.2.tar.xz
CMOCKA_DIR=cmocka

all: $(CMOCKA_DIR) $(TARGET)
	./$(TARGET)

$(SRC_OBJ): | $(OBJ_DIR)

$(OBJ_DIR):
	mkdir -p $@

$(OBJ_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(SRC_OBJ) $(TEST_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(CMOCKA_DIR):
	mkdir -p $@
	tar -xf $(CMOCKA_TAR) -C $@ --strip-components 1
	mkdir -p $@/build
	@(cd $@/build && cmake ../ && make -s)

clean-target:
	rm -rf $(TARGET) $(OBJ_DIR)

clean: clean-target
	rm -rf $(CMOCKA_DIR)
