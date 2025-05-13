#include "test_synth.h"
#include "synth.h"
#include "mocks/mock_comm.h"
#include "mocks/mock_synth.h"
#include "test_midi.h"
#include "z80_ctrl.h"
#include "mocks/mock_ym2612.h"
#include "ym2612_regs.h"
#include "test_helpers.h"

static bool updated = false;
static u8 lastChan = -1;
static ParameterUpdated lastParameterUpdated = -1;

static void set_initial_registers(void)
{
    expect_value(__wrap_Z80_requestBus, wait, TRUE);

    // Global
    expect_ym2612_write_reg_at_init(0, 0x2B, 0x0); // #1
    expect_ym2612_write_reg_at_init(0, 0x27, 0x0); // #2

    // Chan 1
    expect_ym2612_write_reg_at_init(0, YM_KEY_ON_OFF, 0x0); // #3
    expect_ym2612_write_reg_at_init(0, 0xB0, 0x0); // #4
    expect_ym2612_write_reg_at_init(0, 0xB4, 0xC0); // #5

    // Chan 1 Op 1
    expect_ym2612_write_reg_at_init(0, 0x30, 0x1); // #6
    expect_ym2612_write_reg_at_init(0, 0x50, 0x5A); // #7
    expect_ym2612_write_reg_at_init(0, 0x60, 0x7); // #8
    expect_ym2612_write_reg_at_init(0, 0x70, 0x4); // #9
    expect_ym2612_write_reg_at_init(0, 0x80, 0x71); // #10
    expect_ym2612_write_reg_at_init(0, 0x40, 0x23); // #11
    expect_ym2612_write_reg_at_init(0, 0x90, 0x0); // #12

    // Chan 1 Op 2
    expect_ym2612_write_reg_at_init(0, 0x38, 0x64); // #13
    expect_ym2612_write_reg_at_init(0, 0x58, 0x58); // #14
    expect_ym2612_write_reg_at_init(0, 0x68, 0x9); // #15
    expect_ym2612_write_reg_at_init(0, 0x78, 0x9); // #16
    expect_ym2612_write_reg_at_init(0, 0x88, 0x67); // #17
    expect_ym2612_write_reg_at_init(0, 0x48, 0x23); // #18
    expect_ym2612_write_reg_at_init(0, 0x98, 0x0); // #19

    // Chan 1 Op 3
    expect_ym2612_write_reg_at_init(0, 0x34, 0x72); // #20
    expect_ym2612_write_reg_at_init(0, 0x54, 0xDF); // #21
    expect_ym2612_write_reg_at_init(0, 0x64, 0x17); // #22
    expect_ym2612_write_reg_at_init(0, 0x74, 0xF); // #23
    expect_ym2612_write_reg_at_init(0, 0x84, 0x91); // #24
    expect_ym2612_write_reg_at_init(0, 0x44, 0x23); // #25
    expect_ym2612_write_reg_at_init(0, 0x94, 0x0); // #26

    // Chan 1 Op 4
    expect_ym2612_write_reg_at_init(0, 0x3C, 0x31); // #27
    expect_ym2612_write_reg_at_init(0, 0x5C, 0x9B); // #28
    expect_ym2612_write_reg_at_init(0, 0x6C, 0x4); // #29
    expect_ym2612_write_reg_at_init(0, 0x7C, 0x4); // #30
    expect_ym2612_write_reg_at_init(0, 0x8C, 0xA6); // #31
    expect_ym2612_write_reg_at_init(0, 0x4C, 0x23); // #32
    expect_ym2612_write_reg_at_init(0, 0x9C, 0x0); // #33

    // Chan 2
    expect_ym2612_write_reg_at_init(0, YM_KEY_ON_OFF, 0x1); // #34
    expect_ym2612_write_reg_at_init(0, 0xB1, 0x0); // #35
    expect_ym2612_write_reg_at_init(0, 0xB5, 0xC0); // #36

    // Chan 2 Op 1
    expect_ym2612_write_reg_at_init(0, 0x31, 0x1); // #37
    expect_ym2612_write_reg_at_init(0, 0x51, 0x5A); // #38
    expect_ym2612_write_reg_at_init(0, 0x61, 0x7); // #39
    expect_ym2612_write_reg_at_init(0, 0x71, 0x4); // #40
    expect_ym2612_write_reg_at_init(0, 0x81, 0x71); // #41
    expect_ym2612_write_reg_at_init(0, 0x41, 0x23); // #42
    expect_ym2612_write_reg_at_init(0, 0x91, 0x0); // #43

    // Chan 2 Op 2
    expect_ym2612_write_reg_at_init(0, 0x39, 0x64); // #44
    expect_ym2612_write_reg_at_init(0, 0x59, 0x58); // #45
    expect_ym2612_write_reg_at_init(0, 0x69, 0x9); // #46
    expect_ym2612_write_reg_at_init(0, 0x79, 0x9); // #47
    expect_ym2612_write_reg_at_init(0, 0x89, 0x67); // #48
    expect_ym2612_write_reg_at_init(0, 0x49, 0x23); // #49
    expect_ym2612_write_reg_at_init(0, 0x99, 0x0); // #50

    // Chan 2 Op 3
    expect_ym2612_write_reg_at_init(0, 0x35, 0x72); // #51
    expect_ym2612_write_reg_at_init(0, 0x55, 0xDF); // #52
    expect_ym2612_write_reg_at_init(0, 0x65, 0x17); // #53
    expect_ym2612_write_reg_at_init(0, 0x75, 0xF); // #54
    expect_ym2612_write_reg_at_init(0, 0x85, 0x91); // #55
    expect_ym2612_write_reg_at_init(0, 0x45, 0x23); // #56
    expect_ym2612_write_reg_at_init(0, 0x95, 0x0); // #57

    // Chan 2 Op 4
    expect_ym2612_write_reg_at_init(0, 0x3D, 0x31); // #58
    expect_ym2612_write_reg_at_init(0, 0x5D, 0x9B); // #59
    expect_ym2612_write_reg_at_init(0, 0x6D, 0x4); // #60
    expect_ym2612_write_reg_at_init(0, 0x7D, 0x4); // #61
    expect_ym2612_write_reg_at_init(0, 0x8D, 0xA6); // #62
    expect_ym2612_write_reg_at_init(0, 0x4D, 0x23); // #63
    expect_ym2612_write_reg_at_init(0, 0x9D, 0x0); // #64

    // Chan 3
    expect_ym2612_write_reg_at_init(0, YM_KEY_ON_OFF, 0x2); // #65
    expect_ym2612_write_reg_at_init(0, 0xB2, 0x0); // #66
    expect_ym2612_write_reg_at_init(0, 0xB6, 0xC0); // #67

    // Chan 3 Op 1
    expect_ym2612_write_reg_at_init(0, 0x32, 0x1); // #68
    expect_ym2612_write_reg_at_init(0, 0x52, 0x5A); // #69
    expect_ym2612_write_reg_at_init(0, 0x62, 0x7); // #70
    expect_ym2612_write_reg_at_init(0, 0x72, 0x4); // #71
    expect_ym2612_write_reg_at_init(0, 0x82, 0x71); // #72
    expect_ym2612_write_reg_at_init(0, 0x42, 0x23); // #73
    expect_ym2612_write_reg_at_init(0, 0x92, 0x0); // #74

    // Chan 3 Op 2
    expect_ym2612_write_reg_at_init(0, 0x3A, 0x64); // #75
    expect_ym2612_write_reg_at_init(0, 0x5A, 0x58); // #76
    expect_ym2612_write_reg_at_init(0, 0x6A, 0x9); // #77
    expect_ym2612_write_reg_at_init(0, 0x7A, 0x9); // #78
    expect_ym2612_write_reg_at_init(0, 0x8A, 0x67); // #79
    expect_ym2612_write_reg_at_init(0, 0x4A, 0x23); // #80
    expect_ym2612_write_reg_at_init(0, 0x9A, 0x0); // #81

    // Chan 3 Op 3
    expect_ym2612_write_reg_at_init(0, 0x36, 0x72); // #82
    expect_ym2612_write_reg_at_init(0, 0x56, 0xDF); // #83
    expect_ym2612_write_reg_at_init(0, 0x66, 0x17); // #84
    expect_ym2612_write_reg_at_init(0, 0x76, 0xF); // #85
    expect_ym2612_write_reg_at_init(0, 0x86, 0x91); // #86
    expect_ym2612_write_reg_at_init(0, 0x46, 0x23); // #87
    expect_ym2612_write_reg_at_init(0, 0x96, 0x0); // #88

    // Chan 3 Op 4
    expect_ym2612_write_reg_at_init(0, 0x3E, 0x31); // #89
    expect_ym2612_write_reg_at_init(0, 0x5E, 0x9B); // #90
    expect_ym2612_write_reg_at_init(0, 0x6E, 0x4); // #91
    expect_ym2612_write_reg_at_init(0, 0x7E, 0x4); // #92
    expect_ym2612_write_reg_at_init(0, 0x8E, 0xA6); // #93
    expect_ym2612_write_reg_at_init(0, 0x4E, 0x23); // #94
    expect_ym2612_write_reg_at_init(0, 0x9E, 0x0); // #95

    // Chan 4
    expect_ym2612_write_reg_at_init(0, YM_KEY_ON_OFF, 0x4); // #96
    expect_ym2612_write_reg_at_init(1, 0xB0, 0x0); // #97
    expect_ym2612_write_reg_at_init(1, 0xB4, 0xC0); // #98

    // Chan 4 Op 1
    expect_ym2612_write_reg_at_init(1, 0x30, 0x1); // #99
    expect_ym2612_write_reg_at_init(1, 0x50, 0x5A); // #100
    expect_ym2612_write_reg_at_init(1, 0x60, 0x7); // #101
    expect_ym2612_write_reg_at_init(1, 0x70, 0x4); // #102
    expect_ym2612_write_reg_at_init(1, 0x80, 0x71); // #103
    expect_ym2612_write_reg_at_init(1, 0x40, 0x23); // #104
    expect_ym2612_write_reg_at_init(1, 0x90, 0x0); // #105

    // Chan 4 Op 2
    expect_ym2612_write_reg_at_init(1, 0x38, 0x64); // #106
    expect_ym2612_write_reg_at_init(1, 0x58, 0x58); // #107
    expect_ym2612_write_reg_at_init(1, 0x68, 0x9); // #108
    expect_ym2612_write_reg_at_init(1, 0x78, 0x9); // #109
    expect_ym2612_write_reg_at_init(1, 0x88, 0x67); // #110
    expect_ym2612_write_reg_at_init(1, 0x48, 0x23); // #111
    expect_ym2612_write_reg_at_init(1, 0x98, 0x0); // #112

    // Chan 4 Op 3
    expect_ym2612_write_reg_at_init(1, 0x34, 0x72); // #113
    expect_ym2612_write_reg_at_init(1, 0x54, 0xDF); // #114
    expect_ym2612_write_reg_at_init(1, 0x64, 0x17); // #115
    expect_ym2612_write_reg_at_init(1, 0x74, 0xF); // #116
    expect_ym2612_write_reg_at_init(1, 0x84, 0x91); // #117
    expect_ym2612_write_reg_at_init(1, 0x44, 0x23); // #118
    expect_ym2612_write_reg_at_init(1, 0x94, 0x0); // #119

    // Chan 4 Op 4
    expect_ym2612_write_reg_at_init(1, 0x3C, 0x31); // #120
    expect_ym2612_write_reg_at_init(1, 0x5C, 0x9B); // #121
    expect_ym2612_write_reg_at_init(1, 0x6C, 0x4); // #122
    expect_ym2612_write_reg_at_init(1, 0x7C, 0x4); // #123
    expect_ym2612_write_reg_at_init(1, 0x8C, 0xA6); // #124
    expect_ym2612_write_reg_at_init(1, 0x4C, 0x23); // #125
    expect_ym2612_write_reg_at_init(1, 0x9C, 0x0); // #126

    // Chan 5
    expect_ym2612_write_reg_at_init(0, YM_KEY_ON_OFF, 0x5); // #127
    expect_ym2612_write_reg_at_init(1, 0xB1, 0x0); // #128
    expect_ym2612_write_reg_at_init(1, 0xB5, 0xC0); // #129

    // Chan 5 Op 1
    expect_ym2612_write_reg_at_init(1, 0x31, 0x1); // #130
    expect_ym2612_write_reg_at_init(1, 0x51, 0x5A); // #131
    expect_ym2612_write_reg_at_init(1, 0x61, 0x7); // #132
    expect_ym2612_write_reg_at_init(1, 0x71, 0x4); // #133
    expect_ym2612_write_reg_at_init(1, 0x81, 0x71); // #134
    expect_ym2612_write_reg_at_init(1, 0x41, 0x23); // #135
    expect_ym2612_write_reg_at_init(1, 0x91, 0x0); // #136

    // Chan 5 Op 2
    expect_ym2612_write_reg_at_init(1, 0x39, 0x64); // #137
    expect_ym2612_write_reg_at_init(1, 0x59, 0x58); // #138
    expect_ym2612_write_reg_at_init(1, 0x69, 0x9); // #139
    expect_ym2612_write_reg_at_init(1, 0x79, 0x9); // #140
    expect_ym2612_write_reg_at_init(1, 0x89, 0x67); // #141
    expect_ym2612_write_reg_at_init(1, 0x49, 0x23); // #142
    expect_ym2612_write_reg_at_init(1, 0x99, 0x0); // #143

    // Chan 5 Op 3
    expect_ym2612_write_reg_at_init(1, 0x35, 0x72); // #144
    expect_ym2612_write_reg_at_init(1, 0x55, 0xDF); // #145
    expect_ym2612_write_reg_at_init(1, 0x65, 0x17); // #146
    expect_ym2612_write_reg_at_init(1, 0x75, 0xF); // #147
    expect_ym2612_write_reg_at_init(1, 0x85, 0x91); // #148
    expect_ym2612_write_reg_at_init(1, 0x45, 0x23); // #149
    expect_ym2612_write_reg_at_init(1, 0x95, 0x0); // #150

    // Chan 5 Op 4
    expect_ym2612_write_reg_at_init(1, 0x3D, 0x31); // #151
    expect_ym2612_write_reg_at_init(1, 0x5D, 0x9B); // #152
    expect_ym2612_write_reg_at_init(1, 0x6D, 0x4); // #153
    expect_ym2612_write_reg_at_init(1, 0x7D, 0x4); // #154
    expect_ym2612_write_reg_at_init(1, 0x8D, 0xA6); // #155
    expect_ym2612_write_reg_at_init(1, 0x4D, 0x23); // #156
    expect_ym2612_write_reg_at_init(1, 0x9D, 0x0); // #157

    // Chan 6
    expect_ym2612_write_reg_at_init(0, YM_KEY_ON_OFF, 0x6); // #158
    expect_ym2612_write_reg_at_init(1, 0xB2, 0x0); // #159
    expect_ym2612_write_reg_at_init(1, 0xB6, 0xC0); // #160

    // Chan 6 Op 1
    expect_ym2612_write_reg_at_init(1, 0x32, 0x1); // #161
    expect_ym2612_write_reg_at_init(1, 0x52, 0x5A); // #162
    expect_ym2612_write_reg_at_init(1, 0x62, 0x7); // #163
    expect_ym2612_write_reg_at_init(1, 0x72, 0x4); // #164
    expect_ym2612_write_reg_at_init(1, 0x82, 0x71); // #165
    expect_ym2612_write_reg_at_init(1, 0x42, 0x23); // #166
    expect_ym2612_write_reg_at_init(1, 0x92, 0x0); // #167

    // Chan 6 Op 2
    expect_ym2612_write_reg_at_init(1, 0x3A, 0x64); // #168
    expect_ym2612_write_reg_at_init(1, 0x5A, 0x58); // #169
    expect_ym2612_write_reg_at_init(1, 0x6A, 0x9); // #170
    expect_ym2612_write_reg_at_init(1, 0x7A, 0x9); // #171
    expect_ym2612_write_reg_at_init(1, 0x8A, 0x67); // #172
    expect_ym2612_write_reg_at_init(1, 0x4A, 0x23); // #173
    expect_ym2612_write_reg_at_init(1, 0x9A, 0x0); // #174

    // Chan 6 Op 3
    expect_ym2612_write_reg_at_init(1, 0x36, 0x72); // #175
    expect_ym2612_write_reg_at_init(1, 0x56, 0xDF); // #176
    expect_ym2612_write_reg_at_init(1, 0x66, 0x17); // #177
    expect_ym2612_write_reg_at_init(1, 0x76, 0xF); // #178
    expect_ym2612_write_reg_at_init(1, 0x86, 0x91); // #179
    expect_ym2612_write_reg_at_init(1, 0x46, 0x23); // #180
    expect_ym2612_write_reg_at_init(1, 0x96, 0x0); // #181

    // Chan 6 Op 4
    expect_ym2612_write_reg_at_init(1, 0x3E, 0x31); // #182
    expect_ym2612_write_reg_at_init(1, 0x5E, 0x9B); // #183
    expect_ym2612_write_reg_at_init(1, 0x6E, 0x4); // #184
    expect_ym2612_write_reg_at_init(1, 0x7E, 0x4); // #185
    expect_ym2612_write_reg_at_init(1, 0x8E, 0xA6); // #186
    expect_ym2612_write_reg_at_init(1, 0x4E, 0x23); // #187
    expect_ym2612_write_reg_at_init(1, 0x9E, 0x0); // #188
    expect_ym2612_write_reg_any_data_at_init(0, 0x22); // #189

    expect_value(__wrap_YM2612_write, port, 0);
    expect_value(__wrap_YM2612_write, data, YM_DAC_DATA);
    expect_function_call(__wrap_Z80_releaseBus);

    const FmChannel M_BANK_0_INST_0_GRANDPIANO = { 0, 0, 3, 0, 0, 0, 0,
        { { 1, 0, 26, 1, 7, 0, 7, 4, 1, 35, 0 }, { 4, 6, 24, 1, 9, 0, 6, 9, 7, 35, 0 },
            { 2, 7, 31, 3, 23, 0, 9, 15, 1, 35, 0 }, { 1, 3, 27, 2, 4, 0, 10, 4, 6, 35, 0 } } };
    __real_synth_init(&M_BANK_0_INST_0_GRANDPIANO);
}

static void loads_pcm_driver(void)
{
    expect_value(__wrap_Z80_loadDriver, driver, Z80_DRIVER_PCM);
    expect_value(__wrap_Z80_loadDriver, waitReady, true);
}

int test_synth_setup(UNUSED void** state)
{
    updated = false;
    loads_pcm_driver();
    set_initial_registers();
    return 0;
}

void test_synth_init_sets_initial_registers(UNUSED void** state)
{
    loads_pcm_driver();
    set_initial_registers();
}

void test_synth_sets_note_on_fm_reg_chan_0_to_2(UNUSED void** state)
{
    for (u8 chan = 0; chan < 3; chan++) {
        expect_ym2612_note_on(chan);
        __real_synth_note_on(chan);
    }
}

void test_synth_sets_note_on_fm_reg_chan_3_to_5(UNUSED void** state)
{
    for (u8 chan = 3; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_note_on(chan);
        __real_synth_note_on(chan);
    }
}

void test_synth_sets_note_off_fm_reg_chan_0_to_2(UNUSED void** state)
{
    for (u8 chan = 0; chan < 3; chan++) {
        expect_ym2612_note_off(chan);
        __real_synth_note_off(chan);
    }
}

void test_synth_sets_note_off_fm_reg_chan_3_to_5(UNUSED void** state)
{
    for (u8 chan = 3; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_note_off(chan);
        __real_synth_note_off(chan);
    }
}

void test_synth_sets_octave_and_freq_reg_chan(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_channel(chan, YM_BASE_FREQ_MSB_BLK, 0x22);
        expect_ym2612_write_channel(chan, YM_BASE_FREQ_LSB, 0x84);
        __real_synth_pitch(chan, 4, SYNTH_NTSC_C);
    }
}

void test_synth_sets_stereo_ams_and_freq(UNUSED void** state)
{
    const u8 ams = 1;
    const u8 fms = 1;
    const u8 stereo = 1;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_channel_any_data(chan, YM_BASE_STEREO_AMS_PMS);
        __real_synth_stereo(chan, stereo);
        expect_ym2612_write_channel_any_data(chan, YM_BASE_STEREO_AMS_PMS);
        __real_synth_ams(chan, ams);
        expect_ym2612_write_channel(chan, YM_BASE_STEREO_AMS_PMS, (stereo << 6) + (ams << 4) + fms);
        __real_synth_fms(chan, fms);
    }
}

void test_synth_sets_algorithm(UNUSED void** state)
{
    const u8 defaultFeedback = 0;
    const u8 algorithm = 1;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_channel(
            chan, YM_BASE_ALGORITHM_FEEDBACK, (defaultFeedback << 3) + algorithm);
        __real_synth_algorithm(chan, algorithm);
    }
}

void test_synth_sets_feedback(UNUSED void** state)
{
    const u8 defaultAlgorithm = 0;
    const u8 feedback = 1;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_channel(
            chan, YM_BASE_ALGORITHM_FEEDBACK, (feedback << 3) + defaultAlgorithm);
        __real_synth_feedback(chan, feedback);
    }
}

void test_synth_sets_feedback_and_algorithm(UNUSED void** state)
{
    const u8 defaultFeedback = 0;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        u8 feedback = 1;
        u8 algorithm = 1;

        expect_ym2612_write_channel(
            chan, YM_BASE_ALGORITHM_FEEDBACK, (defaultFeedback << 3) + algorithm);
        __real_synth_algorithm(chan, feedback);
        expect_ym2612_write_channel(chan, YM_BASE_ALGORITHM_FEEDBACK, (feedback << 3) + algorithm);
        __real_synth_feedback(chan, feedback);

        feedback++;
        algorithm++;
    }
}

void test_synth_does_not_reset_operator_level_if_equal(UNUSED void** state)
{
    const u8 totalLevel = 50;
    u8 chan = 0;
    u8 op = 0;

    expect_ym2612_write_operator(chan, op, YM_BASE_TOTAL_LEVEL, totalLevel);
    __real_synth_operator_total_level(chan, op, totalLevel);
    __real_synth_operator_total_level(chan, op, totalLevel);
}

void test_synth_sets_operator_total_level(UNUSED void** state)
{
    const u8 totalLevel = 50;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            expect_ym2612_write_operator(chan, op, YM_BASE_TOTAL_LEVEL, totalLevel);
            __real_synth_operator_total_level(chan, op, totalLevel);
        }
    }
}

void test_synth_sets_operator_multiple_and_detune(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        u8 multiple = 0;
        u8 detune = 0;
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            expect_ym2612_write_operator_any_data(chan, op, YM_BASE_MULTIPLE_DETUNE);
            __real_synth_operator_multiple(chan, op, multiple);
            expect_ym2612_write_operator(
                chan, op, YM_BASE_MULTIPLE_DETUNE, (detune << 4) | multiple);
            __real_synth_operator_detune(chan, op, detune);
            multiple++;
            detune++;
        }
    }
}

void test_synth_sets_operator_attack_rate_and_rate_scaling(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        u8 attackRate = 0;
        u8 rateScaling = 0;
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            expect_ym2612_write_operator_any_data(chan, op, YM_BASE_ATTACK_RATE_SCALING_RATE);
            __real_synth_operator_attack_rate(chan, op, attackRate);
            expect_ym2612_write_operator(
                chan, op, YM_BASE_ATTACK_RATE_SCALING_RATE, attackRate | (rateScaling << 6));
            __real_synth_operator_rate_scaling(chan, op, rateScaling);
            attackRate++;
            rateScaling++;
        }
    }
}

void test_synth_sets_operator_sustain_rate(UNUSED void** state)
{
    u8 sustainRate = 16;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            expect_ym2612_write_operator(chan, op, YM_BASE_SUSTAIN_RATE, sustainRate);
            __real_synth_operator_sustain_rate(chan, op, sustainRate);
        }
    }
}

void test_synth_sets_operator_release_rate_and_sustain_level(UNUSED void** state)
{
    u8 sustainLevel = 15;
    u8 releaseRate = 4;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            expect_ym2612_write_operator_any_data(chan, op, YM_BASE_RELEASE_RATE_SUSTAIN_LEVEL);
            __real_synth_operator_sustain_level(chan, op, sustainLevel);
            expect_ym2612_write_operator(
                chan, op, YM_BASE_RELEASE_RATE_SUSTAIN_LEVEL, releaseRate + (sustainLevel << 4));
            __real_synth_operator_release_rate(chan, op, releaseRate);
        }
    }
}

void test_synth_sets_operator_amplitude_modulation_and_decay_rate(UNUSED void** state)
{
    u8 amplitudeModulation = 1;
    u8 decayRate = 16;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            expect_ym2612_write_operator_any_data(chan, op, YM_BASE_DECAY_RATE_AM_ENABLE);
            __real_synth_operator_decay_rate(chan, op, decayRate);
            expect_ym2612_write_operator(
                chan, op, YM_BASE_DECAY_RATE_AM_ENABLE, decayRate + (amplitudeModulation << 7));
            __real_synth_operator_amplitude_modulation(chan, op, amplitudeModulation);
        }
    }
}

void test_synth_sets_operator_ssg_eg(UNUSED void** state)
{
    const u8 ssgEg = 11;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            expect_ym2612_write_operator(chan, op, YM_BASE_SSG_EG, ssgEg);
            __real_synth_operator_ssg_eg(chan, op, ssgEg);
        }
    }
}

void test_synth_sets_global_LFO_enable_and_frequency(UNUSED void** state)
{
    expect_ym2612_write_reg_any_data(0, YM_LFO_ENABLE);
    __real_synth_enable_lfo(1);
    expect_ym2612_write_reg(0, YM_LFO_ENABLE, (1 << 3) | 1);
    __real_synth_global_lfo_frequency(1);
}

void test_synth_sets_busy_indicators(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan += 2) {
        expect_ym2612_write_reg_any_data(0, YM_KEY_ON_OFF);
        __real_synth_note_on(chan);
    }
    u8 busy = synth_busy();
    assert_int_equal(busy, 0b00010101);
}

void test_synth_sets_preset(UNUSED void** state)
{
    const u8 chan = 0;

    const FmChannel M_BANK_0_INST_7_CLAVINET = { 1, 7, 3, 0, 0, 0, 0,
        { { 1, 1, 31, 2, 0, 1, 0, 1, 6, 28, 0 }, { 1, 3, 31, 2, 0, 0, 0, 1, 7, 33, 0 },
            { 1, 5, 31, 3, 0, 1, 0, 0, 2, 30, 0 }, { 1, 7, 31, 0, 6, 0, 4, 6, 7, 6, 0 } } };

    expect_ym2612_write_channel_any_data(chan, YM_BASE_ALGORITHM_FEEDBACK);
    expect_ym2612_write_channel_any_data(chan, YM_BASE_STEREO_AMS_PMS);

    for (u8 op = YM_OP1; op <= YM_OP4; op++) {
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_MULTIPLE_DETUNE, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_ATTACK_RATE_SCALING_RATE, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_DECAY_RATE_AM_ENABLE, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_SUSTAIN_RATE, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_RELEASE_RATE_SUSTAIN_LEVEL, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_TOTAL_LEVEL, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_SSG_EG, op));
    }

    __real_synth_preset(chan, &M_BANK_0_INST_7_CLAVINET);
}

void test_synth_sets_preset_retaining_pan(UNUSED void** state)
{
    const u8 chan = 0;

    expect_ym2612_write_channel(chan, YM_BASE_STEREO_AMS_PMS, 0x80);
    __real_synth_stereo(chan, STEREO_MODE_LEFT);

    const FmChannel M_BANK_0_INST_7_CLAVINET = { 1, 7, 3, 0, 0, 0, 0,
        { { 1, 1, 31, 2, 0, 1, 0, 1, 6, 28, 0 }, { 1, 3, 31, 2, 0, 0, 0, 1, 7, 33, 0 },
            { 1, 5, 31, 3, 0, 1, 0, 0, 2, 30, 0 }, { 1, 7, 31, 0, 6, 0, 4, 6, 7, 6, 0 } } };

    expect_ym2612_write_channel_any_data(chan, YM_BASE_ALGORITHM_FEEDBACK);
    expect_ym2612_write_channel(chan, YM_BASE_STEREO_AMS_PMS, 0x80);

    for (u8 op = YM_OP1; op <= YM_OP4; op++) {
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_MULTIPLE_DETUNE, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_ATTACK_RATE_SCALING_RATE, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_DECAY_RATE_AM_ENABLE, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_SUSTAIN_RATE, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_RELEASE_RATE_SUSTAIN_LEVEL, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_TOTAL_LEVEL, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_SSG_EG, op));
    }
    __real_synth_preset(chan, &M_BANK_0_INST_7_CLAVINET);
}

void test_synth_applies_volume_modifier_to_output_operators_algorithm_7(UNUSED void** state)
{
    const u8 algorithm = 7;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_channel_any_data(chan, YM_BASE_ALGORITHM_FEEDBACK);
        __real_synth_algorithm(chan, algorithm);

        expect_ym2612_write_all_operators(chan, YM_BASE_TOTAL_LEVEL, YM_TOTAL_LEVEL_LOUDEST);
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            __real_synth_operator_total_level(chan, op, YM_TOTAL_LEVEL_LOUDEST);
        }

        const u8 expectedTotalLevel = 0xb;
        expect_ym2612_write_all_operators(chan, YM_BASE_TOTAL_LEVEL, expectedTotalLevel);
        __real_synth_volume(chan, SYNTH_VOLUME_MAX / 2);

        expect_ym2612_write_all_operators(chan, YM_BASE_TOTAL_LEVEL, YM_TOTAL_LEVEL_LOUDEST);
        __real_synth_volume(chan, SYNTH_VOLUME_MAX);
    }
}

void test_synth_does_not_apply_volume_if_equal(UNUSED void** state)
{
    const u8 algorithm = 7;
    u8 chan = 0;
    expect_ym2612_write_channel_any_data(chan, YM_BASE_ALGORITHM_FEEDBACK);
    __real_synth_algorithm(chan, algorithm);

    expect_ym2612_write_all_operators(chan, YM_BASE_TOTAL_LEVEL, YM_TOTAL_LEVEL_LOUDEST);
    for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
        __real_synth_operator_total_level(chan, op, YM_TOTAL_LEVEL_LOUDEST);
    }

    const u8 expectedTotalLevel = 0xb;
    expect_ym2612_write_all_operators(chan, YM_BASE_TOTAL_LEVEL, expectedTotalLevel);
    __real_synth_volume(chan, SYNTH_VOLUME_MAX / 2);

    __real_synth_volume(chan, SYNTH_VOLUME_MAX / 2);
}

void test_synth_applies_volume_modifier_to_output_operators_algorithm_7_quieter(UNUSED void** state)
{
    const u8 algorithm = 7;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_channel_any_data(chan, YM_BASE_ALGORITHM_FEEDBACK);
        __real_synth_algorithm(chan, algorithm);

        expect_ym2612_write_all_operators(chan, YM_BASE_TOTAL_LEVEL, 0);
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            __real_synth_operator_total_level(chan, op, YM_TOTAL_LEVEL_LOUDEST);
        }

        const u8 expectedTotalLevel = 0x26;
        expect_ym2612_write_all_operators(chan, YM_BASE_TOTAL_LEVEL, expectedTotalLevel);
        __real_synth_volume(chan, SYNTH_VOLUME_MAX / 4);
    }
}

void test_synth_applies_volume_modifier_to_output_operators_algorithms_0_to_3(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 algorithm = 0; algorithm < 4; algorithm++) {
            print_message("chan %d, alg %d setup\n", chan, algorithm);
            expect_ym2612_write_channel(chan, YM_BASE_ALGORITHM_FEEDBACK, algorithm);
            __real_synth_algorithm(chan, algorithm);

            if (algorithm == 0) {
                /* Operator values are not re-applied for algorithms 1-3 due
                to unnecessary YM2612 writing optimisation */
                expect_ym2612_write_operator(chan, 0, YM_BASE_TOTAL_LEVEL, 0x23);
                expect_ym2612_write_operator(chan, 1, YM_BASE_TOTAL_LEVEL, 0x23);
                expect_ym2612_write_operator(chan, 2, YM_BASE_TOTAL_LEVEL, 0x23);
                expect_ym2612_write_operator(chan, 3, YM_BASE_TOTAL_LEVEL, 0x3f);
            }
            __real_synth_volume(chan, SYNTH_VOLUME_MAX / 4);
        }
    }
}

void test_synth_applies_volume_modifier_to_output_operators_algorithm_4(UNUSED void** state)
{
    const u8 algorithm = 4;

    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_channel(chan, YM_BASE_ALGORITHM_FEEDBACK, algorithm);
        __real_synth_algorithm(chan, algorithm);

        expect_ym2612_write_operator(chan, 0, YM_BASE_TOTAL_LEVEL, 0x23);
        expect_ym2612_write_operator(chan, 1, YM_BASE_TOTAL_LEVEL, 0x3f);
        expect_ym2612_write_operator(chan, 2, YM_BASE_TOTAL_LEVEL, 0x23);
        expect_ym2612_write_operator(chan, 3, YM_BASE_TOTAL_LEVEL, 0x3f);
        __real_synth_volume(chan, SYNTH_VOLUME_MAX / 4);
    }
}

void test_synth_applies_volume_modifier_to_output_operators_algorithms_5_and_6(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 algorithm = 5; algorithm < 7; algorithm++) {
            expect_ym2612_write_channel(chan, YM_BASE_ALGORITHM_FEEDBACK, algorithm);
            __real_synth_algorithm(chan, algorithm);

            if (algorithm == 5) {
                /* Operator values are not re-applied for algorithms 1-3 due
                to unnecessary YM2612 writing optimisation */
                expect_ym2612_write_operator(chan, 0, YM_BASE_TOTAL_LEVEL, 0x23);
                expect_ym2612_write_operator(chan, 1, YM_BASE_TOTAL_LEVEL, 0x3f);
                expect_ym2612_write_operator(chan, 2, YM_BASE_TOTAL_LEVEL, 0x3f);
                expect_ym2612_write_operator(chan, 3, YM_BASE_TOTAL_LEVEL, 0x3f);
            }
            __real_synth_volume(chan, SYNTH_VOLUME_MAX / 4);
        }
    }
}

void test_synth_exposes_fm_channel_parameters(UNUSED void** state)
{
    const FmChannel* chan = __real_synth_channel_parameters(0);

    assert_int_equal(chan->stereo, STEREO_MODE_CENTRE);
}

void test_synth_exposes_global_parameters(UNUSED void** state)
{
    expect_ym2612_write_reg(0, YM_LFO_ENABLE, 0x09);
    __real_synth_global_lfo_frequency(1);

    const Global* global = __real_synth_global_parameters();

    assert_int_equal(global->lfoFrequency, 1);
}

static void updateCallback(u8 chan, ParameterUpdated parameterUpdated)
{
    updated = true;
    lastChan = chan;
    lastParameterUpdated = parameterUpdated;
}

void test_synth_calls_callback_when_parameter_changes(UNUSED void** state)
{
    synth_set_parameter_update_callback(&updateCallback);

    const u8 defaultFeedback = 0;
    const u8 algorithm = 1;
    u8 fmChan = 1;
    expect_ym2612_write_channel(
        fmChan, YM_BASE_ALGORITHM_FEEDBACK, (defaultFeedback << 3) + algorithm);
    __real_synth_algorithm(fmChan, algorithm);

    assert_true(updated);
    assert_int_equal(lastChan, fmChan);
    assert_int_equal(lastParameterUpdated, Channel);
}

void test_synth_calls_callback_when_lfo_freq_changes(UNUSED void** state)
{
    synth_set_parameter_update_callback(&updateCallback);

    expect_ym2612_write_reg_any_data(0, YM_LFO_ENABLE);
    __real_synth_global_lfo_frequency(1);

    assert_true(updated);
    assert_int_equal(lastParameterUpdated, Lfo);
}

void test_synth_calls_callback_when_lfo_enable_changes(UNUSED void** state)
{
    synth_set_parameter_update_callback(&updateCallback);

    expect_ym2612_write_reg_any_data(0, YM_LFO_ENABLE);
    __real_synth_enable_lfo(0);

    assert_true(updated);
    assert_int_equal(lastParameterUpdated, Lfo);
}

void test_synth_calls_callback_when_special_mode_changes(UNUSED void** state)
{
    synth_set_parameter_update_callback(&updateCallback);

    expect_ym2612_write_reg_any_data(0, YM_CH3_MODE);
    __real_synth_set_special_mode(true);

    assert_true(updated);
    assert_int_equal(lastParameterUpdated, SpecialMode);
}

void test_synth_enables_ch3_special_mode(UNUSED void** state)
{
    expect_ym2612_write_reg(0, YM_CH3_MODE, 0x40);
    __real_synth_set_special_mode(true);
}

void test_synth_disables_ch3_special_mode(UNUSED void** state)
{
    expect_ym2612_write_reg(0, YM_CH3_MODE, 0);
    __real_synth_set_special_mode(false);
}

void test_synth_sets_ch3_special_mode_operator_pitches(UNUSED void** state)
{
    const u8 upperRegs[]
        = { YM_CH3SM_OP1_FREQ_MSB_BLK, YM_CH3SM_OP3_FREQ_MSB_BLK, YM_CH3SM_OP2_FREQ_MSB_BLK };
    const u8 lowerRegs[] = { YM_CH3SM_OP1_FREQ_LSB, YM_CH3SM_OP3_FREQ_LSB, YM_CH3SM_OP2_FREQ_LSB };

    for (u8 op = 0; op < 3; op++) {
        expect_ym2612_write_reg(0, upperRegs[op], 0x22);
        expect_ym2612_write_reg(0, lowerRegs[op], 0x84);

        __real_synth_special_mode_pitch(op, 4, SYNTH_NTSC_C);
    }
}

void test_synth_handles_out_of_range_ch3_special_mode_operator(UNUSED void** state)
{
    const u8 op = 3; // invalid op
    expect_ym2612_write_reg(0, YM_CH3SM_OP1_FREQ_MSB_BLK, 0x22); // safely wrap to valid reg
    expect_ym2612_write_reg(0, YM_CH3SM_OP1_FREQ_LSB, 0x84);

    __real_synth_special_mode_pitch(op, 4, SYNTH_NTSC_C);
}

void test_synth_sets_ch3_special_mode_op_tl_only_if_output_operator(UNUSED void** state)
{
    int algorithm = (*(int*)(*state));

    expect_ym2612_write_channel(CH3_SPECIAL_MODE, YM_BASE_ALGORITHM_FEEDBACK, algorithm);
    __real_synth_algorithm(CH3_SPECIAL_MODE, algorithm);

    for (u8 op = 0; op <= 2; op++) {
        // set initial TL
        expect_ym2612_write_operator(CH3_SPECIAL_MODE, op, YM_BASE_TOTAL_LEVEL, 2);
        __real_synth_operator_total_level(CH3_SPECIAL_MODE, op, 2);

        switch (algorithm) {
        case 0:
        case 1:
        case 2:
        case 3: {
            // expect nothing to happen
            __real_synth_special_mode_volume(op, 60);
            break;
        }
        case 4: {
            if (op == 0 || op == 2) {
                // expect nothing to happen
                __real_synth_special_mode_volume(op, 60);
            } else {
                // set volume
                expect_ym2612_write_operator(CH3_SPECIAL_MODE, op, YM_BASE_TOTAL_LEVEL, 14);
                __real_synth_special_mode_volume(op, 60);
            }
            break;
        }
        case 5:
        case 6: {
            if (op == 0) {
                // expect nothing to happen
                __real_synth_special_mode_volume(op, 60);
            } else {
                // set volume
                expect_ym2612_write_operator(CH3_SPECIAL_MODE, op, YM_BASE_TOTAL_LEVEL, 14);
                __real_synth_special_mode_volume(op, 60);
            }
            break;
        }
        case 7: {
            // set volume
            expect_ym2612_write_operator(CH3_SPECIAL_MODE, op, YM_BASE_TOTAL_LEVEL, 14);
            __real_synth_special_mode_volume(op, 60);
            break;
        }
        }
    }
}

void test_synth_enables_dac(UNUSED void** state)
{
    expect_ym2612_write_reg(0, YM_DAC_ENABLE, 0x80);

    __real_synth_enable_dac(true);
}

void test_synth_disables_dac(UNUSED void** state)
{
    expect_ym2612_write_reg(0, YM_DAC_ENABLE, 0);

    __real_synth_enable_dac(false);
}

void test_requests_Z80_bus_if_not_already_taken(UNUSED void** state)
{
    expect_value(__wrap_Z80_getAndRequestBus, wait, TRUE);
    will_return(__wrap_Z80_getAndRequestBus, false);

    expect_value(__wrap_YM2612_writeReg, part, 0);
    expect_value(__wrap_YM2612_writeReg, reg, YM_DAC_ENABLE);
    expect_value(__wrap_YM2612_writeReg, data, 0);

    expect_value(__wrap_YM2612_write, port, 0);
    expect_value(__wrap_YM2612_write, data, YM_DAC_DATA);
    expect_function_call(__wrap_Z80_releaseBus);

    __real_synth_direct_write_ym2612(0, YM_DAC_ENABLE, 0);
}

void test_does_not_release_Z80_bus_when_taken_prior_to_call(UNUSED void** state)
{
    expect_value(__wrap_Z80_getAndRequestBus, wait, TRUE);
    will_return(__wrap_Z80_getAndRequestBus, true);

    expect_value(__wrap_YM2612_writeReg, part, 0);
    expect_value(__wrap_YM2612_writeReg, reg, YM_DAC_ENABLE);
    expect_value(__wrap_YM2612_writeReg, data, 0);

    __real_synth_direct_write_ym2612(0, YM_DAC_ENABLE, 0);
}
