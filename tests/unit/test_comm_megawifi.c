#include "cmocka_inc.h"
#include "applemidi.h"
#include "comm_megawifi.h"
#include "mw/loop.h"
#include "mw/megawifi.h"
#include "mw/mpool.h"
#include "mw/util.h"
#include "buffer.h"

extern void __real_comm_megawifi_init(void);

static int test_comm_megawifi_setup(UNUSED void** state)
{
    log_init();
    wraps_enable_logging_checks();
    return 0;
}

#define expect_udp_port_open(c, s_port)                                        \
    do {                                                                       \
        expect_value(__wrap_mw_udp_set, ch, c);                                \
        expect_value(__wrap_mw_udp_set, dst_addr, NULL);                       \
        expect_value(__wrap_mw_udp_set, dst_port, NULL);                       \
        expect_memory(__wrap_mw_udp_set, src_port, s_port, sizeof(s_port));    \
        will_return(__wrap_mw_udp_set, MW_ERR_NONE);                           \
    } while (0)

static void expect_mw_init(void)
{
    expect_any(__wrap_mw_init, cmd_buf);
    expect_any(__wrap_mw_init, buf_len);
    will_return(__wrap_mw_init, MW_ERR_NONE);

    expect_value(__wrap_loop_init, max_func, 2);
    expect_value(__wrap_loop_init, max_timer, 4);
    will_return(__wrap_loop_init, MW_ERR_NONE);

    expect_any(__wrap_loop_func_add, func);
    will_return(__wrap_loop_func_add, MW_ERR_NONE);
}

static void expect_mw_detect(void)
{
    mock_mw_detect(3, 1);
    will_return(__wrap_mw_detect, MW_ERR_NONE);
    expect_log_info("MW: Detected v%d.%d");
}

static void expect_ap_connection(void)
{
    expect_value(__wrap_mw_ap_assoc, slot, 0);
    will_return(__wrap_mw_ap_assoc, MW_ERR_NONE);

    expect_any(__wrap_mw_ap_assoc_wait, tout_frames);
    will_return(__wrap_mw_ap_assoc_wait, MW_ERR_NONE);
}

static void expect_ip_log(void)
{
    mock_ip_cfg(ip_str_to_uint32("127.1.2.3"));
    will_return(__wrap_mw_ip_current, MW_ERR_NONE);
    expect_log_info("MW: IP: %s");
}

static void megawifi_init(void)
{
    expect_mw_init();
    expect_mw_detect();
    expect_ap_connection();
    expect_ip_log();
    expect_udp_port_open(CH_CONTROL_PORT, "5006");
    expect_udp_port_open(CH_MIDI_PORT, "5007");
    expect_log_info("MW: Listening on UDP %d");
    __real_comm_megawifi_init();
}

static void test_comm_megawifi_initialises(UNUSED void** state)
{
    megawifi_init();
}

static void test_comm_megawifi_reads_midi_message(UNUSED void** state)
{
    megawifi_init();
}

static void test_comm_megawifi_logs_if_buffer_full(UNUSED void** state)
{
    expect_log_warn("MW: MIDI buffer full!");

    for (u16 i = 0; i < BUFFER_SIZE + 1; i++) {
        __real_comm_megawifi_midiEmitCallback(0x00);
    }
}
