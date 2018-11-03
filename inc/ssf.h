/*
 * File:   ssf.h
 * Author: krik
 *
 * Created on September 22, 2014, 12:25 AM
 */

#pragma once
#include "types.h"

void ssf_led_off();
void ssf_led_on();
u16 ssf_usb_rd_ready();
u16 ssf_usb_wr_ready();
u16 ssd_spi_ready();
u8 ssf_usb_read();
u8 ssf_usb_write(u8 data);
void ssf_init();
void ssf_set_rom_bank(u8 bank, u8 val);
u8 ssf_spi(u8 data);
void ssf_spi_ss_off();
void ssf_spi_ss_on();
void ssf_spi_qrd_off();
void ssf_spi_qrd_on();
u16 ssf_spi_qrd();
void ssf_spi_qwr_on();
void ssf_spi_qwr_off();
void ssf_spi_qwr(u16 data);
u16 ssf_card_type();
void ssf_rom_wr_on();
void ssf_rom_wr_off();
u32 ssf_mul32(u32 arg1, u32 arg2);
u32 ssf_div32(u32 arg1, u32 arg2);
u16 ssf_mul16(u16 arg1, u16 arg2);
u16 ssf_div16(u16 arg1, u16 arg2);
