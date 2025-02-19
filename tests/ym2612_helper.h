#pragma once

#define YM_CH3SM_OP_SELECT(baseReg, op) (baseReg + ((op + 1) % 3))
#define YM_REG(baseReg, channel) (baseReg + (channel % 3))
#define YM_OP_REG_INDEX(op) ((op) == 1 ? 2 : ((op) == 2 ? 1 : (op)))
#define YM_OP_SELECT(baseReg, op) (baseReg + (YM_OP_REG_INDEX(op) * 4))

#define YM_CH3SM_OP1_FREQ_LSB 0xA9
#define YM_CH3SM_OP1_FREQ_MSB_BLK 0xAD
#define YM_CH3SM_OP2_FREQ_LSB 0xA8
#define YM_CH3SM_OP2_FREQ_MSB_BLK 0xAC
#define YM_CH3SM_OP3_FREQ_LSB 0xAA
#define YM_CH3SM_OP3_FREQ_MSB_BLK 0xAE
#define YM_CH3SM_OP4_FREQ_LSB 0xA2
#define YM_CH3SM_OP4_FREQ_MSB_BLK 0xA6

#define YM_TOTAL_LEVEL_LOUDEST 0
#define YM_TOTAL_LEVEL_SILENCE 0x7F

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

#define PSG_CH1 0
#define PSG_CH2 1
#define PSG_CH3 2
#define PSG_NOISE_CH4 3
