#include "midi_fm_sram.h"
#include "midi.h"
#include "sram.h"
#include "log.h"
#include "crc.h"

const u16 FM_PRESET_MAGIC_NUMBER = 0x9E1D;
const u16 SRAM_PRESET_BASE_OFFSET = 32;
const u8 FM_PRESET_VERSION = 1;

static void sram_preset_slot(SramFmPresetSlot* slot, const FmPreset* preset);
static void preset_from_sram_preset_slot(FmPreset* preset, const SramFmPresetSlot* slot);

void midi_fm_sram_init(void)
{
    sram_init();
}

void midi_fm_sram_save_preset(u8 program, const FmPreset* preset)
{
    if (!sram_is_present()) {
        log_warn("No SRAM");
        return;
    }

    SramFmPresetSlot slot = { 0 };
    sram_preset_slot(&slot, preset);
    u16 crc = crc_calc_crc16((u8*)&slot, sizeof(SramFmPresetSlot) - sizeof(slot.checksum));
    slot.checksum = crc;

    if (sizeof(SramFmPresetSlot) != SRAM_FM_PRESET_LENGTH) {
        log_warn(
            "Bad slot struct size (%d != %d)", (u8)sizeof(SramFmPresetSlot), SRAM_FM_PRESET_LENGTH);
        return;
    }

    u16 offset = SRAM_PRESET_BASE_OFFSET + SRAM_FM_PRESET_LENGTH * program;
    sram_enable(true);
    for (u8 i = 0; i < sizeof(SramFmPresetSlot); i++) {
        u8 val = ((u8*)&slot)[i];
        sram_write(offset, val);
        u8 readVal = sram_read(offset);
        if (readVal != val) {
            log_warn("Bad SRAM write (%d: %02X != %02X)", offset, readVal, val);
        }
        offset++;
    }
    sram_disable();
}

void midi_fm_sram_clear_preset(u8 program)
{
    if (!sram_is_present()) {
        log_warn("No SRAM");
        return;
    }

    u16 offset = SRAM_PRESET_BASE_OFFSET + SRAM_FM_PRESET_LENGTH * program;
    sram_enable(true);
    sram_write(offset, 0); // wipe magic number
    sram_write(offset + 1, 0);
    sram_disable();
}

void midi_fm_sram_load_presets(FmPreset* const userPresets, FmPreset** const activeUserPresets)
{
    if (!sram_is_present()) {
        return;
    }
    sram_enable(false);
    u8 count = 0;
    for (u8 program = 0; program < MIDI_PROGRAMS; program++) {
        u16 offset = SRAM_PRESET_BASE_OFFSET + SRAM_FM_PRESET_LENGTH * program;
        u16 magic = (sram_read(offset) << 8) | sram_read(offset + 1);
        if (magic != FM_PRESET_MAGIC_NUMBER) {
            continue;
        }

        SramFmPresetSlot slot = { 0 };
        for (u8 j = 0; j < sizeof(SramFmPresetSlot); j++) {
            ((u8*)&slot)[j] = sram_read(offset + j);
        }
        u16 crc = crc_calc_crc16((u8*)&slot, sizeof(SramFmPresetSlot) - sizeof(slot.checksum));
        if (slot.checksum != crc) {
            log_warn("Prg %d: Bad checksum", program);
            continue;
        }
        FmPreset preset;
        preset_from_sram_preset_slot(&preset, &slot);
        memcpy(&userPresets[program], &preset, sizeof(FmPreset));
        activeUserPresets[program] = &userPresets[program];
        count++;
    }
    if (count != 0) {
        log_info("Loaded %d FM presets", count);
    }
    sram_disable();
}

static void sram_preset_slot(SramFmPresetSlot* slot, const FmPreset* preset)
{
    slot->magic_number = FM_PRESET_MAGIC_NUMBER;
    slot->version = FM_PRESET_VERSION;
    slot->preset.algorithm = preset->algorithm;
    slot->preset.feedback = preset->feedback;
    slot->preset.ams = preset->ams;
    slot->preset.fms = preset->fms;
    for (u8 i = 0; i < sizeof(slot->reserved); i++) {
        slot->reserved[i] = 0;
    }
    slot->checksum = 0;
    for (u8 i = 0; i < MAX_FM_OPERATORS; i++) {
        slot->preset.operators[i].multiple = preset->operators[i].multiple;
        slot->preset.operators[i].detune = preset->operators[i].detune;
        slot->preset.operators[i].attackRate = preset->operators[i].attackRate;
        slot->preset.operators[i].rateScaling = preset->operators[i].rateScaling;
        slot->preset.operators[i].decayRate = preset->operators[i].decayRate;
        slot->preset.operators[i].amplitudeModulation = preset->operators[i].amplitudeModulation;
        slot->preset.operators[i].sustainLevel = preset->operators[i].sustainLevel;
        slot->preset.operators[i].sustainRate = preset->operators[i].sustainRate;
        slot->preset.operators[i].releaseRate = preset->operators[i].releaseRate;
        slot->preset.operators[i].totalLevel = preset->operators[i].totalLevel;
        slot->preset.operators[i].ssgEg = preset->operators[i].ssgEg;
    }
}

static void preset_from_sram_preset_slot(FmPreset* preset, const SramFmPresetSlot* slot)
{
    preset->algorithm = slot->preset.algorithm;
    preset->feedback = slot->preset.feedback;
    preset->ams = slot->preset.ams;
    preset->fms = slot->preset.fms;
    for (u8 j = 0; j < MAX_FM_OPERATORS; j++) {
        preset->operators[j].multiple = slot->preset.operators[j].multiple;
        preset->operators[j].detune = slot->preset.operators[j].detune;
        preset->operators[j].attackRate = slot->preset.operators[j].attackRate;
        preset->operators[j].rateScaling = slot->preset.operators[j].rateScaling;
        preset->operators[j].decayRate = slot->preset.operators[j].decayRate;
        preset->operators[j].amplitudeModulation = slot->preset.operators[j].amplitudeModulation;
        preset->operators[j].sustainLevel = slot->preset.operators[j].sustainLevel;
        preset->operators[j].sustainRate = slot->preset.operators[j].sustainRate;
        preset->operators[j].releaseRate = slot->preset.operators[j].releaseRate;
        preset->operators[j].totalLevel = slot->preset.operators[j].totalLevel;
        preset->operators[j].ssgEg = slot->preset.operators[j].ssgEg;
    }
}
