#include "test_comm_megawifi.h"
#include "comm/applemidi.h"
#include "comm/comm_megawifi.h"
#include "ext/mw/megawifi.h"
#include "ext/mw/lsd.h"
#include "comm/ring_buf.h"
#include "settings.h"
#include "comm/ip_util.h"
#include "mocks/mock_log.h"
#include "mocks/mock_comm.h"
#include "mocks/mock_sgdk.h"
#include "mocks/mock_scheduler.h"

int test_comm_megawifi_setup(UNUSED void** state)
{
    log_init();
    ring_buf_init();
    mock_log_enable_checks();
    return 0;
}

#define expect_udp_port_open(c, s_port)                                                            \
    do {                                                                                           \
        expect_value(__wrap_mw_udp_set, ch, c);                                                    \
        expect_value(__wrap_mw_udp_set, dst_addr, NULL);                                           \
        expect_value(__wrap_mw_udp_set, dst_port, NULL);                                           \
        expect_memory(__wrap_mw_udp_set, src_port, s_port, sizeof(s_port));                        \
        will_return(__wrap_mw_udp_set, MW_ERR_NONE);                                               \
    } while (0)

static void expect_mw_init(void)
{
    expect_any(__wrap_mw_init, cmd_buf);
    expect_any(__wrap_mw_init, buf_len);
    will_return(__wrap_mw_init, MW_ERR_NONE);
}

static void expect_mw_detect(void)
{
    mock_mw_detect(3, 1);
    will_return(__wrap_mw_detect, MW_ERR_NONE);
    if (settings_debug_megawifi_init()) {
        expect_log_info("MW: Detected v%d.%d");
    }
}

static void expect_ap_connection(void)
{
    will_return(__wrap_mw_def_ap_cfg_get, 0);

    expect_value(__wrap_mw_ap_assoc, slot, 0);
    will_return(__wrap_mw_ap_assoc, MW_ERR_NONE);

    expect_any(__wrap_mw_ap_assoc_wait, tout_frames);
    will_return(__wrap_mw_ap_assoc_wait, MW_ERR_NONE);
}

static void expect_ip_cfg(void)
{
    mock_ip_cfg(IP(127, 1, 2, 3));
    will_return(__wrap_mw_ip_current, MW_ERR_NONE);
}

static void megawifi_init(void)
{
    expect_any(__wrap_scheduler_addTickHandler, onTick);
    expect_any(__wrap_scheduler_addFrameHandler, onFrame);
    will_return(__wrap_mw_uart_is_present, true);
    expect_log_info("MW: Detecting...");
    expect_scheduler_yield();
    expect_mw_init();
    expect_log_info("MW: Found v%d.%d");
    expect_scheduler_yield();
    expect_mw_detect();
    expect_log_info("MW: Ctrl UDP %s:%u");
    expect_scheduler_yield();
    expect_scheduler_yield();
    expect_ap_connection();
    expect_ip_cfg();
    expect_udp_port_open(CH_CONTROL_PORT, "5006");
    expect_udp_port_open(CH_MIDI_PORT, "5007");
    __real_comm_megawifi_init();
}

void test_comm_megawifi_reads_midi_message(UNUSED void** state)
{
    megawifi_init();
}

void test_comm_megawifi_logs_if_buffer_full(UNUSED void** state)
{
    expect_log_warn("MW: MIDI buffer full!");

    for (u16 i = 0; i < RING_BUF_CAPACITY + 1; i++) {
        __real_comm_megawifi_midiEmitCallback(0x00);
    }
}

void test_comm_megawifi_returns_zero_when_buffer_empty(UNUSED void** state)
{
    megawifi_init();

    expect_log_warn("MW: Attempted read from empty buffer");
    u8 data = __real_comm_megawifi_read();

    assert_int_equal(data, 0);
}