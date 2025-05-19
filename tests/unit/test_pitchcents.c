#include "test_pitchcents.h"
#include "test_midi.h"

const PitchCents DEFAULT_RANGE = { .pitch = 2, .cents = 0 };

void test_pitchcents_shift_extreme_up(UNUSED void** state)
{
    PitchCents pc = { .pitch = 50, .cents = 99 };
    pc = pc_shift(pc, 255);

    assert_int_equal(pc.pitch, 53);
    assert_int_equal(pc.cents, 54);
}

void test_pitchcents_bend_nil(UNUSED void** state)
{
    PitchCents pc = pc_bend(50, 0, 0x2000, DEFAULT_RANGE);

    assert_int_equal(pc.pitch, 50);
    assert_int_equal(pc.cents, 0);
}

void test_pitchcents_bend_down_fully(UNUSED void** state)
{
    PitchCents pc = pc_bend(50, 0, 0, DEFAULT_RANGE);

    assert_int_equal(pc.pitch, 48);
    assert_int_equal(pc.cents, 0);
}

void test_pitchcents_bend_up_fully(UNUSED void** state)
{
    PitchCents pc = pc_bend(50, 0, 0x4000, DEFAULT_RANGE);

    assert_int_equal(pc.pitch, 52);
    assert_int_equal(pc.cents, 0);
}

void test_pitchcents_bend_up(UNUSED void** state)
{
    PitchCents pc = pc_bend(50, 0, 0x3000, DEFAULT_RANGE);

    assert_int_equal(pc.pitch, 51);
    assert_int_equal(pc.cents, 0);
}

void test_pitchcents_bend_down(UNUSED void** state)
{
    PitchCents pc = pc_bend(50, 0, 0x1800, DEFAULT_RANGE);

    assert_int_equal(pc.pitch, 49);
    assert_int_equal(pc.cents, 50);
}

void test_pitchcents_bend_up_2(UNUSED void** state)
{
    PitchCents pc = pc_bend(50, 0, 0x2800, DEFAULT_RANGE);

    assert_int_equal(pc.pitch, 50);
    assert_int_equal(pc.cents, 50);
}

void test_pitchcents_bend_cents_with_partial_bend_down(UNUSED void** state)
{
    PitchCents pc = pc_bend(50, 25, 0x1800, DEFAULT_RANGE);

    assert_int_equal(pc.pitch, 49);
    assert_int_equal(pc.cents, 75);
}

void test_pitchcents_bend_high_cents_with_partial_bend_down(UNUSED void** state)
{
    PitchCents pc = pc_bend(50, 80, 0x1800, DEFAULT_RANGE);

    assert_int_equal(pc.pitch, 50);
    assert_int_equal(pc.cents, 30);
}

void test_pitchcents_bend_cents_with_full_bend_up(UNUSED void** state)
{
    PitchCents pc = pc_bend(50, 25, 0x4000, DEFAULT_RANGE);

    assert_int_equal(pc.pitch, 52);
    assert_int_equal(pc.cents, 25);
}

void test_pitchcents_bend_high_cents_with_full_bend_up(UNUSED void** state)
{
    PitchCents pc = pc_bend(50, 80, 0x4000, DEFAULT_RANGE);

    assert_int_equal(pc.pitch, 52);
    assert_int_equal(pc.cents, 80);
}

void test_pitchcents_bend_st_up_4(UNUSED void** state)
{
    PitchCents range = { .pitch = 4, .cents = 0 };
    PitchCents pc = pc_bend(50, 0, 0x4000, range);

    assert_int_equal(pc.pitch, 54);
    assert_int_equal(pc.cents, 0);
}

void test_pitchcents_bend_st_down_4(UNUSED void** state)
{
    PitchCents range = { .pitch = 4, .cents = 0 };
    PitchCents pc = pc_bend(50, 0, 0x0000, range);

    assert_int_equal(pc.pitch, 46);
    assert_int_equal(pc.cents, 0);
}

void test_pitchcents_bend_st_up_extreme(UNUSED void** state)
{
    PitchCents range = { .pitch = 48, .cents = 50 };
    PitchCents pc = pc_bend(50, 0, 0x4000, range);

    assert_int_equal(pc.pitch, 98);
    assert_int_equal(pc.cents, 50);
}

void test_pitchcents_bend_st_down_extreme(UNUSED void** state)
{
    PitchCents range = { .pitch = 48, .cents = 50 };
    PitchCents pc = pc_bend(50, 0, 0x0000, range);

    assert_int_equal(pc.pitch, 1);
    assert_int_equal(pc.cents, 50);
}