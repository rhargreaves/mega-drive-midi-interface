#pragma once

// Global
#define YM_LFO_ENABLE 0x22
#define YM_CH3_MODE 0x27
#define YM_KEY_ON_OFF 0x28
#define YM_DAC_DATA 0x2A
#define YM_DAC_ENABLE 0x2B
#define YM_TEST 0x2C

// Channel 1, Operator 1
#define YM_BASE_MULTIPLE_DETUNE 0x30
#define YM_BASE_TOTAL_LEVEL 0x40
#define YM_BASE_ATTACK_RATE_SCALING_RATE 0x50
#define YM_BASE_DECAY_RATE_AM_ENABLE 0x60
#define YM_BASE_SUSTAIN_RATE 0x70
#define YM_BASE_RELEASE_RATE_SUSTAIN_LEVEL 0x80
#define YM_BASE_SSG_EG 0x90

// Channel 1
#define YM_BASE_FREQ_LSB 0xA0
#define YM_BASE_FREQ_MSB_BLK 0xA4
#define YM_BASE_ALGORITHM_FEEDBACK 0xB0
#define YM_BASE_STEREO_AMS_PMS 0xB4

// Channel 3 Special Mode
#define YM_CH3SM_BASE_FREQ_LSB 0xA8
#define YM_CH3SM_BASE_FREQ_MSB_BLK 0xAC

// Operator Offsets
#define YM_OP2_OFFSET 0x08
#define YM_OP3_OFFSET 0x04
#define YM_OP4_OFFSET 0x0C

// Test Helpers
#define YM_CH3SM_OP_SELECT(baseReg, op) (baseReg + ((op + 1) % 3))
#define YM_CH_SELECT(baseReg, channel) (baseReg + (channel % 3))
#define YM_OP_REG_INDEX(op) ((op) == 1 ? 2 : ((op) == 2 ? 1 : (op)))
#define YM_OP_SELECT(baseReg, op) (baseReg + (YM_OP_REG_INDEX(op) * 4))

#define YM_CH1 0
#define YM_CH2 1
#define YM_CH3 2
#define YM_CH4 0
#define YM_CH5 1
#define YM_CH6 2

#define YM_OP1 0
#define YM_OP2 1
#define YM_OP3 2
#define YM_OP4 3
