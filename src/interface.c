#include <comm.h>
#include <interface.h>
#include <midi.h>
#include <string.h>
#include <synth.h>

#define STATUS_CHANNEL(status) status & 0x0F
#define STATUS_EVENT(status) status >> 4

#define RANGE(value, range) value / (128 / range)

#define EVENT_NOTE_ON 0x9
#define EVENT_NODE_OFF 0x8
#define EVENT_CC 0xB

#define CC_VOLUME 7
#define CC_PAN 10
#define CC_GENMDM_FM_ALGORITHM 14
#define CC_GENMDM_FM_FEEDBACK 15
#define CC_GENMDM_TOTAL_LEVEL_OP1 16
#define CC_GENMDM_TOTAL_LEVEL_OP2 17
#define CC_GENMDM_TOTAL_LEVEL_OP3 18
#define CC_GENMDM_TOTAL_LEVEL_OP4 19
#define CC_GENMDM_MULTIPLE_OP1 20
#define CC_GENMDM_MULTIPLE_OP2 21
#define CC_GENMDM_MULTIPLE_OP3 22
#define CC_GENMDM_MULTIPLE_OP4 23
#define CC_GENMDM_DETUNE_OP1 24
#define CC_GENMDM_DETUNE_OP2 25
#define CC_GENMDM_DETUNE_OP3 26
#define CC_GENMDM_DETUNE_OP4 27
#define CC_GENMDM_RATE_SCALING_OP1 39
#define CC_GENMDM_RATE_SCALING_OP2 40
#define CC_GENMDM_RATE_SCALING_OP3 41
#define CC_GENMDM_RATE_SCALING_OP4 42
#define CC_GENMDM_ATTACK_RATE_OP1 43
#define CC_GENMDM_ATTACK_RATE_OP2 44
#define CC_GENMDM_ATTACK_RATE_OP3 45
#define CC_GENMDM_ATTACK_RATE_OP4 46
#define CC_GENMDM_FIRST_DECAY_RATE_OP1 47
#define CC_GENMDM_FIRST_DECAY_RATE_OP2 48
#define CC_GENMDM_FIRST_DECAY_RATE_OP3 49
#define CC_GENMDM_FIRST_DECAY_RATE_OP4 50
#define CC_GENMDM_SECOND_DECAY_RATE_OP1 51
#define CC_GENMDM_SECOND_DECAY_RATE_OP2 52
#define CC_GENMDM_SECOND_DECAY_RATE_OP3 53
#define CC_GENMDM_SECOND_DECAY_RATE_OP4 54
#define CC_GENMDM_SECOND_AMPLITUDE_OP1 55
#define CC_GENMDM_SECOND_AMPLITUDE_OP2 56
#define CC_GENMDM_SECOND_AMPLITUDE_OP3 57
#define CC_GENMDM_SECOND_AMPLITUDE_OP4 58
#define CC_GENMDM_AMPLITUDE_MODULATION_OP1 70
#define CC_GENMDM_AMPLITUDE_MODULATION_OP2 71
#define CC_GENMDM_AMPLITUDE_MODULATION_OP3 72
#define CC_GENMDM_AMPLITUDE_MODULATION_OP4 73

#define CC_ALL_NOTES_OFF 123

static u8 lastUnknownStatus = 0;
static ControlChange lastUnknownControlChange;

static void noteOn(u8 status);
static void noteOff(u8 status);
static void controlChange(u8 status);

void interface_init(void)
{
    synth_init();
}

void interface_loop(void)
{
    while (TRUE) {
        interface_tick();
    }
}

void interface_tick(void)
{
    u8 status = comm_read();
    u8 event = STATUS_EVENT(status);
    switch (event) {
    case EVENT_NOTE_ON:
        noteOn(status);
        break;
    case EVENT_NODE_OFF:
        noteOff(status);
        break;
    case EVENT_CC:
        controlChange(status);
        break;
    default:
        lastUnknownStatus = status;
        break;
    }
}

ControlChange* interface_lastUnknownCC(void)
{
    return &lastUnknownControlChange;
}

static void controlChange(u8 status)
{
    u8 chan = STATUS_CHANNEL(status);
    u8 controller = comm_read();
    u8 value = comm_read();
    switch (controller) {
    case CC_VOLUME:
        midi_channelVolume(chan, value);
        break;
    case CC_PAN:
        midi_pan(chan, value);
        break;
    case CC_ALL_NOTES_OFF:
        midi_noteOff(chan);
        break;
    case CC_GENMDM_FM_ALGORITHM:
        synth_algorithm(chan, RANGE(value, 8));
        break;
    case CC_GENMDM_FM_FEEDBACK:
        synth_feedback(chan, RANGE(value, 8));
        break;
    case CC_GENMDM_TOTAL_LEVEL_OP1:
    case CC_GENMDM_TOTAL_LEVEL_OP2:
    case CC_GENMDM_TOTAL_LEVEL_OP3:
    case CC_GENMDM_TOTAL_LEVEL_OP4:
        synth_operatorTotalLevel(
            chan, controller - CC_GENMDM_TOTAL_LEVEL_OP1, value);
        break;
    case CC_GENMDM_MULTIPLE_OP1:
    case CC_GENMDM_MULTIPLE_OP2:
    case CC_GENMDM_MULTIPLE_OP3:
    case CC_GENMDM_MULTIPLE_OP4:
        synth_operatorMultiple(
            chan, controller - CC_GENMDM_MULTIPLE_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_DETUNE_OP1:
    case CC_GENMDM_DETUNE_OP2:
    case CC_GENMDM_DETUNE_OP3:
    case CC_GENMDM_DETUNE_OP4:
        synth_operatorDetune(
            chan, controller - CC_GENMDM_DETUNE_OP1, RANGE(value, 8));
        break;
    case CC_GENMDM_RATE_SCALING_OP1:
    case CC_GENMDM_RATE_SCALING_OP2:
    case CC_GENMDM_RATE_SCALING_OP3:
    case CC_GENMDM_RATE_SCALING_OP4:
        synth_operatorRateScaling(
            chan, controller - CC_GENMDM_RATE_SCALING_OP1, RANGE(value, 4));
        break;
    case CC_GENMDM_ATTACK_RATE_OP1:
    case CC_GENMDM_ATTACK_RATE_OP2:
    case CC_GENMDM_ATTACK_RATE_OP3:
    case CC_GENMDM_ATTACK_RATE_OP4:
        synth_operatorAttackRate(
            chan, controller - CC_GENMDM_ATTACK_RATE_OP1, RANGE(value, 32));
        break;
    case CC_GENMDM_FIRST_DECAY_RATE_OP1:
    case CC_GENMDM_FIRST_DECAY_RATE_OP2:
    case CC_GENMDM_FIRST_DECAY_RATE_OP3:
    case CC_GENMDM_FIRST_DECAY_RATE_OP4:
        synth_operatorFirstDecayRate(chan,
            controller - CC_GENMDM_FIRST_DECAY_RATE_OP1, RANGE(value, 32));
        break;
    case CC_GENMDM_SECOND_DECAY_RATE_OP1:
    case CC_GENMDM_SECOND_DECAY_RATE_OP2:
    case CC_GENMDM_SECOND_DECAY_RATE_OP3:
    case CC_GENMDM_SECOND_DECAY_RATE_OP4:
        synth_operatorSecondDecayRate(chan,
            controller - CC_GENMDM_SECOND_DECAY_RATE_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_SECOND_AMPLITUDE_OP1:
    case CC_GENMDM_SECOND_AMPLITUDE_OP2:
    case CC_GENMDM_SECOND_AMPLITUDE_OP3:
    case CC_GENMDM_SECOND_AMPLITUDE_OP4:
        synth_operatorSecondaryAmplitude(chan,
            controller - CC_GENMDM_SECOND_AMPLITUDE_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_AMPLITUDE_MODULATION_OP1:
    case CC_GENMDM_AMPLITUDE_MODULATION_OP2:
    case CC_GENMDM_AMPLITUDE_MODULATION_OP3:
    case CC_GENMDM_AMPLITUDE_MODULATION_OP4:
        synth_operatorAmplitudeModulation(chan,
            controller - CC_GENMDM_AMPLITUDE_MODULATION_OP1, RANGE(value, 2));
        break;
    default:
        lastUnknownControlChange.controller = controller;
        lastUnknownControlChange.value = value;
        break;
    }
}

static void noteOn(u8 status)
{
    u8 chan = STATUS_CHANNEL(status);
    u8 pitch = comm_read();
    u8 velocity = comm_read();
    midi_noteOn(chan, pitch, velocity);
}

static void noteOff(u8 status)
{
    u8 chan = STATUS_CHANNEL(status);
    comm_read();
    comm_read();
    midi_noteOff(chan);
}

u8 interface_lastUnknownStatus(void)
{
    return lastUnknownStatus;
}
