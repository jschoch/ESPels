#include <vector>
#include <chrono>

//#include <Arduino.h> //redirects to ArduinoFake.h in this case
#include <unity.h>
#include <iostream>
#include "src/log.h"
#include "src/genStepper.h"
#include "src/moveConfig.h"
#include "src/mocks.h"


using namespace fakeit;

// run using `pio test -e ESPelsTest -v`
// debug `pio debug -e ESPelsTest --interface gdb -x .pioinit`

#define UNITY_INCLUDE_EXEC_TIME


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


void test_venc_moveSync(){
    Log::Msg lm;

    GenStepper::State gs = GenStepper::init("Z",lm);
    MoveConfig::State mc = MoveConfig::init();
    //gs.c.spindle_encoder_resolution = 1000;
    //gs.c.lead_screw_pitch = 1.0;
    //gs.c.motor_steps = 1000;

    
    bool t = gs.setELSFactor(1,true);
    TEST_ASSERT(t == true);
    std::cout << "nom: " << gs.nom << " den: " << gs.den << "\n";

    int64_t pulse_counter = 0;
    t = gs.init_gear(pulse_counter);
    gs.mygear.calc_jumps(pulse_counter,true);
    int ts = 1000000;
    auto start = std::chrono::steady_clock::now();
    for(int i = 0;i <= ts;i++){
       if(i == gs.mygear.jumps.next || i == gs.mygear.jumps.prev){
            gs.mygear.calc_jumps(i);
            gs.step();
       } 
    }
    auto stop = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    //auto duration = (start - stop);
    int counter = duration.count();
    std::cout << "time for bench: " << counter << " us\n";
    TEST_ASSERT_EQUAL_INT(ts,gs.position );
    


}

int main( int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_venc_moveSync);
    return UNITY_END();
}

