#include <Arduino.h> //redirects to ArduinoFake.h in this case
#include <unity.h>
#include <iostream>
#include "src/genStepper.h"
#include "src/log.h"
#include "src/moveConfig.h"
#include "src/mocks.h"

using namespace fakeit;

// run using `pio test -e ESPelsTest -v`

Log::Msg el;


void tearDown(void) {
    // clean stuff up here
    //TEST_ASSERT_TRUE(false);
}

void setUp(void)
{
    ArduinoFakeReset();
}

void setup(){

}
void loop(){

}

void test_genstepper(){
    //GenStepper::State gs;
    GenStepper::State gs = GenStepper::init("Z");

    // how does this not crash
    //el.error("foo");
    TEST_ASSERT(gs.c.dir == 0);
    std::cout << "name: " << gs.c.name << " pitch: " <<  gs.mygear.pitch << "\n";
    
    gs.mygear.calc_jumps(100,true);
    std::cout << "gear nom: " << gs.mygear.N << " den: " << gs.mygear.D << " \n";
    std::cout << "gs nom: " << gs.nom << " den: " << gs.den << " \n";

    // should be 1:1 max and jumps should happen every encoder tick
    std::cout << "jumps: " << gs.mygear.jumps.last << " - " << gs.mygear.jumps.prev <<  "\n";
    gs.setELSFactor(2.0);
    std::cout << "gear nom: " << gs.mygear.N << " den: " << gs.mygear.D << " \n";
    gs.mygear.calc_jumps(100,true);
    std::cout << "jumps: " << gs.mygear.jumps.last << " - " << gs.mygear.jumps.prev <<  "\n";
}
void test_moveConfig(){
    MoveConfig::State mc = MoveConfig::init();


    // test abs move from "0" to 1000 steps
    int distance = 1000;
    mc.moveDistanceSteps = distance;
    bool r = mc.setStops(0);

    std::cout << "step dir bool: " << r << " stopPos: " << mc.stopPos << " stopNeg: " << mc.stopNeg << " target: " << mc.moveSyncTarget <<  "\n";
    TEST_ASSERT(r);
    TEST_ASSERT(mc.stopPos == distance);
}


void test_fakePosFeeding(){
    //      How it works
    // 1.record current encoder position
    // 2. sanity check to make sure jumps are sane
    // 3. deal with spindle direction changes
    // 4. deal with dir change pauses for stepper
    // 5. evaluate if we have reached a "jump"
    // 6. if yes command stepper
    // 7. evaluate stops, if reached finish move

    //      Impl

    GenStepper::State gs = GenStepper::init("Z");
    MoveConfig::State mc = MoveConfig::init();

    // must mock this
    //rmtStepper::State rmt;
    //rmt.init();
    Mocks::rmtState rmt;

    //  record current encoder position
    //int64_t pulse_counter = encoder.getCount();
    
    // setup
    //bool t = true;
    int64_t pulse_counter = 99;
    /*
    gs.setELSFactor(1.0);
    gs.mygear.calc_jumps(pulse_counter,true);

    // sanity check to make sure jumps are sane
    

    if(pulse_counter > gs.mygear.jumps.next+1 || pulse_counter < gs.mygear.jumps.prev -1){
        std::cout << "doh! sanity check fail\n";
    }else{
        std::cout << "sanity check passed\n";
    }

    // deal with spindle direction changes

    // deal with dir change pauses for stepper

    // evaluate if we have reached a "jump"

    // if yes command stepper

    // evaluate stops, if reached finish move

    */
    TEST_ASSERT(true);
}

int main( int argc, char **argv) {
    UNITY_BEGIN();
    std::cout << "stepper test\n";
    RUN_TEST(test_genstepper);
    RUN_TEST(test_moveConfig);
    RUN_TEST(test_fakePosFeeding);
    return UNITY_END();
}
