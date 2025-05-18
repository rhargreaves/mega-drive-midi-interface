#include "cmocka_inc.h"
#include "genesis.h"

int test_serial_setup(void** state);
void test_serial_readyToReceive_when_not_ready(void** state);
void test_serial_readyToReceive_when_ready(void** state);
void test_serial_receive(void** state);
void test_serial_sctrl(void** state);
void test_serial_inits(void** state);
void test_serial_sends(void** state);
void test_serial_readyToSend_when_not_ready(void** state);
void test_serial_readyToSend_when_ready(void** state);
void test_serial_sendWhenReady(void** state);
void test_serial_sendWhenReady_waits_until_ready(void** state);
void test_serial_setReadyToReceiveCallback(void** state);
