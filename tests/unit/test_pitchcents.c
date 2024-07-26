#include "test_midi.h"

static void test_pitchcents_shift_extreme_up(UNUSED void** state)
{
    PitchCents pc = { .pitch = 50, .cents = 99 };
    pc = pitchcents_shift(pc, 255);

    assert_int_equal(pc.pitch, 53);
    assert_int_equal(pc.cents, 54);
}

static void test_pitchcents_bend_nil(UNUSED void** state)
{
    PitchCents pc = pitchcents_bend(50, 0, 0x2000);

    assert_int_equal(pc.pitch, 50);
    assert_int_equal(pc.cents, 0);
}

static void test_pitchcents_bend_down_fully(UNUSED void** state)
{
    PitchCents pc = pitchcents_bend(50, 0, 0);

    assert_int_equal(pc.pitch, 48);
    assert_int_equal(pc.cents, 0);
}

static void test_pitchcents_bend_up_fully(UNUSED void** state)
{
    PitchCents pc = pitchcents_bend(50, 0, 0x4000);

    assert_int_equal(pc.pitch, 52);
    assert_int_equal(pc.cents, 0);
}

static void test_pitchcents_bend_up(UNUSED void** state)
{
    PitchCents pc = pitchcents_bend(50, 0, 0x3000);

    assert_int_equal(pc.pitch, 51);
    assert_int_equal(pc.cents, 0);
}

static void test_pitchcents_bend_down(UNUSED void** state)
{
    PitchCents pc = pitchcents_bend(50, 0, 0x1800);

    assert_int_equal(pc.pitch, 49);
    assert_int_equal(pc.cents, 50);
}

static void test_pitchcents_bend_up_2(UNUSED void** state)
{
    PitchCents pc = pitchcents_bend(50, 0, 0x2800);

    assert_int_equal(pc.pitch, 50);
    assert_int_equal(pc.cents, 50);
}

static void test_pitchcents_bend_cents_with_partial_bend_down(UNUSED void** state)
{
    PitchCents pc = pitchcents_bend(50, 25, 0x1800);

    assert_int_equal(pc.pitch, 49);
    assert_int_equal(pc.cents, 75);
}

static void test_pitchcents_bend_high_cents_with_partial_bend_down(UNUSED void** state)
{
    PitchCents pc = pitchcents_bend(50, 80, 0x1800);

    assert_int_equal(pc.pitch, 50);
    assert_int_equal(pc.cents, 30);
}

static void test_pitchcents_bend_cents_with_full_bend_up(UNUSED void** state)
{
    PitchCents pc = pitchcents_bend(50, 25, 0x4000);

    assert_int_equal(pc.pitch, 52);
    assert_int_equal(pc.cents, 25);
}

static void test_pitchcents_bend_high_cents_with_full_bend_up(UNUSED void** state)
{
    PitchCents pc = pitchcents_bend(50, 80, 0x4000);

    assert_int_equal(pc.pitch, 52);
    assert_int_equal(pc.cents, 80);
}
