#include <Arduino.h> //redirects to ArduinoFake.h in this case
#include <unity.h>
#include <iostream>


//#define Z_NATIVE_STEPS_PER_REV 200
//#define Z_MICROSTEPPING 8
#include <test_lib_helper.h>


#include "src/genStepper.h"
#include "src/moveConfig.h"
#include "src/mocks.h"
#include "src/jsonValidation.h"
#include <elslog.h>

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
    std::cout << "test_genstepper\n";
    Log::Msg lm;
    // GenStepper::State gs;
    GenStepper::Config gconf = {
        0,
        "Z",
        2.0, // lead screw pitch
        2400, // spindle enc resolution
        200, // native steps
        8, // microsteps
        1600
    };

    MCDOC mcdoc = {
        1.0, // move pitch
        1.1, // rapid pitch
        100000, // accel
        1, // dwell
        1, // distance
        true, // f
        false
    };

    GenStepper::State gs = GenStepper::init("Z", lm,gconf);

    TEST_ASSERT(gs.c.dir == 0);

    std::cout << "name: " << gs.c.name << " movePitch: " << mc.movePitch << "\n";


    gs.mygear.calc_jumps(100, true);
    std::cout << "gear nom: " << gs.mygear.N << " den: " << gs.mygear.D << " \n";
    std::cout << "gs nom: " << gs.nom << " den: " << gs.den << " \n";
    TEST_ASSERT(gs.nom != 0);
    TEST_ASSERT(gs.den != 0);
    // should be 1:1 max and jumps should happen every encoder tick
    std::cout << "jumps: " << gs.mygear.jumps.last << " - " << gs.mygear.jumps.prev << "\n";
    gs.setELSFactor(2.0);
    std::cout << "gear nom: " << gs.mygear.N << " den: " << gs.mygear.D << " \n";
    gs.mygear.calc_jumps(100, true);
    std::cout << "jumps: " << gs.mygear.jumps.last << " - " << gs.mygear.jumps.prev << "\n";

    bool fails_zero_pitch = gs.setELSFactor(0, false);
    TEST_ASSERT(fails_zero_pitch == false);

    // test max pitch 
    std::cout << "test max pitch check\n";
    mcdoc.rapidPitch = 10;
    bool fails_max_pitch = gs.setELSFactor(mcdoc.rapidPitch);

    TEST_ASSERT(fails_max_pitch == false);
    std::cout << "\tdone\n";
}
void test_moveConfig()
{
    std::cout << "test_moveConfig\n";
    Log::Msg lm;
    MoveConfig::State mc = MoveConfig::init();

    // test abs move from "0" to 1000 steps
    int distance = 1000;
    mc.moveDistanceSteps = distance;

    // TODO:  do we always set the stepper when we set the stops?  If so consider factoring this out
    bool r = mc.setStops(0);
    gs.zstepper.setDir(r);

            std::cout
        << "step dir bool: " << r << " stopPos: " << mc.stopPos << " stopNeg: " << mc.stopNeg << " target: " << mc.moveDistanceSteps << "\n";
    TEST_ASSERT(r);
    TEST_ASSERT(mc.stopPos == distance);
}

void test_fakePosFeeding()
{
    std::cout << "test_fakePosFeeding\n";
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

    GenStepper::Config gconf = {
        0,
        "Z",
        2.0, // lead screw pitch
        2400, // spindle enc resolution
        200, // native steps
        8, // microsteps
        1600 // motor_steps  this should be calculated from native steps and microsteps
    };
    GenStepper::State gs = GenStepper::init("Z", lm,gconf);
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

    if (pulse_counter > gs.mygear.next + 1 || pulse_counter < gs.mygear.prev - 1)
    {
        std::cout << "doh! sanity check fail\n";
        std::cout << gs.mygear.next + 1 << " prev: " << gs.mygear.prev - 1 << "\n";
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
    gs.zstepper.setDir(false);
    gs.stepNeg();
    TEST_ASSERT(0 == gs.position);

    // evaluate stops, if reached finish move

    std::cout << "done\n";
}
void test_venc_moveSync()
{
    std::cout << "test_venc_moveSync \n";
    Log::Msg lm;


    GenStepper::Config gconf = {
        0,
        "Z",
        2.0, // lead screw pitch
        2400, // spindle enc resolution
        200, // native steps
        8, // microsteps
        1600 // motorsteps
    };
    GenStepper::State gs = GenStepper::init("Z", lm,gconf);
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
        if (i == gs.mygear.next || i == gs.mygear.prev)
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

void test_bad_config(){

    std::cout << "test_bad_config\n";
    Log::Msg lm;

    GenStepper::Config gconf = {
        0,
        "Z",
        2.0, // lead screw pitch
        2400, // spindle enc resolution
        200, // native steps
        8 // microsteps
        // **** missing motor_steps
    };
    GenStepper::State gs = GenStepper::init("Z", lm,gconf);
    MoveConfig::State mc = MoveConfig::init();


    gs.init_gear(0);
    bool bad = gs.setELSFactor(0.1,true);
    TEST_ASSERT_EQUAL(false,bad);
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
     //When(Method(ArduinoFake(), Serial)).AlwaysReturn();
    std::cout << "stepper test\n";
    RUN_TEST(test_genstepper);
    RUN_TEST(test_moveConfig);
    RUN_TEST(test_venc_moveSync);
    // TODO: get abs working
    // RUN_TEST(test_venc_moveAbsSync);
    RUN_TEST(test_fakePosFeeding);
    RUN_TEST(test_bad_config);
    return UNITY_END();
}
