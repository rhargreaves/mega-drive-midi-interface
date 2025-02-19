#pragma once

#define YM_CH3SM_OP_SELECT(baseReg, op) (baseReg + ((op + 1) % 3))
#define YM_CH_SELECT(baseReg, channel) (baseReg + (channel % 3))
#define YM_OP_REG_INDEX(op) ((op) == 1 ? 2 : ((op) == 2 ? 1 : (op)))
#define YM_OP_SELECT(baseReg, op) (baseReg + (YM_OP_REG_INDEX(op) * 4))

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
