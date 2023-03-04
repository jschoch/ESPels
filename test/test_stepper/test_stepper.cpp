#include <Arduino.h> //redirects to ArduinoFake.h in this case
#include <unity.h>
#include <iostream>
#include "src/genStepper.h"
#include "src/log.h"

using namespace fakeit;

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

int main( int argc, char **argv) {
    UNITY_BEGIN();
    std::cout << "stepper test\n";
    RUN_TEST(test_genstepper);
    return UNITY_END();
}
