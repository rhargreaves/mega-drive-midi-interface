#include "cmocka_inc.h"
#include "mocks/mock_ui.h"

void __wrap_ui_fm_set_parameters_visibility(u8 chan, bool show)
{
    check_expected(chan);
    check_expected(show);
}
