#pragma once
#include "genesis.h"
#include "debug.h"
#include "cmocka.h"

void mock_ym2612_disable_checks(void);
void mock_ym2612_enable_checks(void);

void __wrap_YM2612_writeReg(const u16 part, const u8 reg, const u8 data);
void __wrap_YM2612_write(const u16 port, const u8 data);

void _expect_ym2612_write_reg(u8 part, u8 reg, u8 data, const char* const file, const int line);
void _expect_ym2612_write_reg_any_data(u8 part, u8 reg, const char* const file, const int line);
void _expect_ym2612_write_channel(
    u8 chan, u8 baseReg, u8 data, const char* const file, const int line);
void _expect_ym2612_write_channel_any_data(
    u8 chan, u8 baseReg, const char* const file, const int line);
void _expect_ym2612_write_operator(
    u8 chan, u8 op, u8 baseReg, u8 data, const char* const file, const int line);
void _expect_ym2612_write_operator_any_data(
    u8 chan, u8 op, u8 baseReg, const char* const file, const int line);
void _expect_ym2612_write_all_operators(
    u8 chan, u8 baseReg, u8 data, const char* const file, const int line);
void _expect_ym2612_write_all_operators_any_data(
    u8 chan, u8 baseReg, const char* const file, const int line);
void _expect_ym2612_note_on(u8 chan, const char* const file, const int line);
void _expect_ym2612_note_off(u8 chan, const char* const file, const int line);
void _expect_ym2612_write_frequency(
    u8 chan, u16 msb, u16 lsb, const char* const file, const int line);
void _expect_ym2612_write_frequency_any_data(u8 chan, const char* const file, const int line);
void _expect_ym2612_write_operator_volumes(
    u8 chan, const u8* volumes, u8 count, const char* const file, const int line);

#define expect_ym2612_write_reg(part, reg, data)                                                   \
    _expect_ym2612_write_reg(part, reg, data, __FILE__, __LINE__)
#define expect_ym2612_write_reg_any_data(part, reg)                                                \
    _expect_ym2612_write_reg_any_data(part, reg, __FILE__, __LINE__)
#define expect_ym2612_write_channel(chan, baseReg, data)                                           \
    _expect_ym2612_write_channel(chan, baseReg, data, __FILE__, __LINE__)
#define expect_ym2612_write_channel_any_data(chan, baseReg)                                        \
    _expect_ym2612_write_channel_any_data(chan, baseReg, __FILE__, __LINE__)
#define expect_ym2612_write_operator(chan, op, baseReg, data)                                      \
    _expect_ym2612_write_operator(chan, op, baseReg, data, __FILE__, __LINE__)
#define expect_ym2612_write_operator_any_data(chan, op, baseReg)                                   \
    _expect_ym2612_write_operator_any_data(chan, op, baseReg, __FILE__, __LINE__)
#define expect_ym2612_write_all_operators(chan, baseReg, data)                                     \
    _expect_ym2612_write_all_operators(chan, baseReg, data, __FILE__, __LINE__)
#define expect_ym2612_write_all_operators_any_data(chan, baseReg)                                  \
    _expect_ym2612_write_all_operators_any_data(chan, baseReg, __FILE__, __LINE__)
#define expect_ym2612_note_on(chan) _expect_ym2612_note_on(chan, __FILE__, __LINE__)
#define expect_ym2612_note_off(chan) _expect_ym2612_note_off(chan, __FILE__, __LINE__)
#define expect_ym2612_write_frequency(chan, msb, lsb)                                              \
    _expect_ym2612_write_frequency(chan, msb, lsb, __FILE__, __LINE__)
#define expect_ym2612_write_frequency_any_data(chan)                                               \
    _expect_ym2612_write_frequency_any_data(chan, __FILE__, __LINE__)
#define expect_ym2612_write_operator_volumes(chan, volumes, count)                                 \
    _expect_ym2612_write_operator_volumes(chan, volumes, count, __FILE__, __LINE__)
