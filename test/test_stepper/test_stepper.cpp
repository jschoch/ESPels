#include <Arduino.h> //redirects to ArduinoFake.h in this case
#include <unity.h>
#include <iostream>
#include "src/log.h"
#include "src/genStepper.h"
#include "src/moveConfig.h"
#include "src/mocks.h"

using namespace fakeit;

// run using `pio test -e ESPelsTest -v`
// debug `pio debug -e ESPelsTest --interface gdb -x .pioinit`

void tearDown(void)
{
    // clean stuff up here
    // TEST_ASSERT_TRUE(false);
}

void setUp(void)
{
    ArduinoFakeReset();
}

void setup()
{
}
void loop()
{
}

void test_genstepper()
{
    Log::Msg lm;
    // GenStepper::State gs;
    GenStepper::State gs = GenStepper::init("Z", lm);

    TEST_ASSERT(gs.c.dir == 0);
    std::cout << "name: " << gs.c.name << " pitch: " << gs.mygear.pitch << "\n";

    gs.mygear.calc_jumps(100, true);
    std::cout << "gear nom: " << gs.mygear.N << " den: " << gs.mygear.D << " \n";
    std::cout << "gs nom: " << gs.nom << " den: " << gs.den << " \n";

    // should be 1:1 max and jumps should happen every encoder tick
    std::cout << "jumps: " << gs.mygear.jumps.last << " - " << gs.mygear.jumps.prev << "\n";
    gs.setELSFactor(2.0);
    std::cout << "gear nom: " << gs.mygear.N << " den: " << gs.mygear.D << " \n";
    gs.mygear.calc_jumps(100, true);
    std::cout << "jumps: " << gs.mygear.jumps.last << " - " << gs.mygear.jumps.prev << "\n";

    bool fails_zero_pitch = gs.setELSFactor(0, false);
    TEST_ASSERT(fails_zero_pitch == false);
}
void test_moveConfig()
{
    Log::Msg lm;
    MoveConfig::State mc = MoveConfig::init();

    // test abs move from "0" to 1000 steps
    int distance = 1000;
    mc.moveDistanceSteps = distance;

    // TODO:  do we always set the stepper when we set the stops?  If so consider factoring this out
    bool r = mc.setStops(0);
    gs.stepper.setDir(r)

            std::cout
        << "step dir bool: " << r << " stopPos: " << mc.stopPos << " stopNeg: " << mc.stopNeg << " target: " << mc.moveSyncTarget << "\n";
    TEST_ASSERT(r);
    TEST_ASSERT(mc.stopPos == distance);
}

void test_fakePosFeeding()
{
    Log::Msg lm;
    //      How it works
    // 1.record current encoder position
    // 2. sanity check to make sure jumps are sane
    // 3. deal with spindle direction changes
    // 4. deal with dir change pauses for stepper
    // 5. evaluate if we have reached a "jump"
    // 6. if yes command stepper
    // 7. evaluate stops, if reached finish move

    //      Impl

    GenStepper::State gs = GenStepper::init("Z", lm);
    MoveConfig::State mc = MoveConfig::init();

    //  record current encoder position
    // int64_t pulse_counter = encoder.getCount();

    // setup
    // bool t = true;
    int64_t pulse_counter = 100;
    bool t = gs.setELSFactor(0.1);
    TEST_ASSERT(t == true);
    gs.mygear.calc_jumps(pulse_counter, true);

    // sanity check to make sure jumps are sane

    if (pulse_counter > gs.mygear.jumps.next + 1 || pulse_counter < gs.mygear.jumps.prev - 1)
    {
        std::cout << "doh! sanity check fail\n";
        std::cout << gs.mygear.jumps.next + 1 << " prev: " << gs.mygear.jumps.prev - 1 << "\n";
    }
    else
    {
        std::cout << "sanity check passed\n";
    }

    // deal with spindle direction changes

    // skipping, need to test in encoder tests

    // deal with dir change pauses for stepper

    // this goes away with fastaccelstepper

    // evaluate if we have reached a "jump"

    // based on pitch 0.1 and default setup we should be at a jump
    // TEST_ASSERT_EQUAL_INT(99,gs.mygear.jumps.next );

    // if yes command stepper

    gs.stepPos();
    TEST_ASSERT(1 == gs.position);
    gs.stepper.setDir(false);
    gs.stepNeg();
    TEST_ASSERT(0 == gs.position);

    // evaluate stops, if reached finish move

    std::cout << "done\n";
}
void test_venc_moveSync()
{
    Log::Msg lm;

    GenStepper::State gs = GenStepper::init("Z", lm);
    MoveConfig::State mc = MoveConfig::init();
    gs.c.spindle_encoder_resolution = 1000;
    gs.c.lead_screw_pitch = 1.0;
    gs.c.motor_steps = 1000;

    bool t = gs.setELSFactor(1, true);
    TEST_ASSERT(t == true);
    std::cout << "nom: " << gs.nom << " den: " << gs.den << "\n";

    int64_t pulse_counter = 0;
    t = gs.init_gear(pulse_counter);
    gs.mygear.calc_jumps(pulse_counter, true);
    for (int i = 0; i <= 10; i++)
    {
        if (i == gs.mygear.jumps.next || i == gs.mygear.jumps.prev)
        {
            gs.mygear.calc_jumps(i);
            gs.step();
        }
    }
    TEST_ASSERT_EQUAL_INT(10, gs.position);
}

void test_venc_moveAbsSync()
{
    TEST_ASSERT(false);
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    std::cout << "stepper test\n";
    RUN_TEST(test_genstepper);
    RUN_TEST(test_moveConfig);
    RUN_TEST(test_venc_moveSync);
    // TODO: get abs working
    // RUN_TEST(test_venc_moveAbsSync);
    RUN_TEST(test_fakePosFeeding);
    return UNITY_END();
}
