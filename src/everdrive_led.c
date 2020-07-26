#include "everdrive_led.h"
#include <stdbool.h>

#define CTRL_LED_ON 0x1000
#define CTRL_LED_OFF 0x0000
#define CTRL_UNPROTECT 0x8000

#define REG_CTRL *((vu16*)0xA130F0)

static bool ledOn = false;

static void led_on(void)
{
    REG_CTRL = CTRL_LED_ON | CTRL_UNPROTECT;
    ledOn = true;
}

static void led_off(void)
{
    REG_CTRL = CTRL_LED_OFF | CTRL_UNPROTECT;
    ledOn = false;
}

void everdrive_led_tick(void)
{
    if (ledOn) {
        led_off();
    }
}

void everdrive_led_blink(void)
{
    if (!ledOn) {
        led_on();
    }
}
