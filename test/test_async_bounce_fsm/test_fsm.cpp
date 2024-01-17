
#include <unity.h>
#include <iostream>
#include <yasm.h>

#include "src/mocks.h"
#include <test_lib_helper.h>
#include "src/genStepper.h"
#include "src/moveConfig.h"
#include "src/Stepper.h"
#include "src/AsyncBounceMode.h"
#include <ctime>

using namespace fakeit;

// because we are not compiling the .cpp
int32_t s_0 = 0;
int32_t move_distance = 0;
uint32_t vstart = 0;
uint32_t vend = 0;
uint32_t vtarget = 0;
int64_t vs_sqr = 0;
int64_t ve_sqr = 0;
int64_t vt_sqr = 0;
uint32_t two_a = 0;
int32_t accEnd = 0;
int32_t  decStart = 0;
int32_t volatile stepsDelta = 0;

int move_start_position_0 = 0;

void setUp(void) {
    // set stuff up here
    ArduinoFakeReset();
    
}

void tearDown(void) {
    // clean stuff up here
}


volatile bool async_bouncing = false;


void fsm_async_bounce(){
    long unsigned int thetime = 0;
    printf("starting fsm_async_bounce..\n");
    //When(Method(ArduinoFake(), millis)).Return(thetime++);
    //When(Method(ArduinoFake(),millis)).Do([](long unsigned int a) -> long unsigned int{a++;});
    When(Method(ArduinoFake(),millis)).AlwaysDo([&thetime]() mutable -> long unsigned int{return thetime++;});

    printf("wtf is up with mc?  %i\n",mc.dwell);
    mc.dwell =50;
    mc.moveDirection = 1;
    mc.moveDistanceSteps = 100;
    mc.accel = 5000;
    mc.moveSpeed = 1000;

    main_yasm.run();

    // start the fsm
    main_yasm.next(AsyncBounceIdleState);
    //my_fsm.next(S1);
    //my_fsm.run();

    main_yasm.run();

    main_yasm.run();

    // start bounce

    async_bouncing = true;
    
    main_yasm.run();
    main_yasm.run();
    main_yasm.run();
    // stop move
    pos_feeding = false;
    main_yasm.run();
    main_yasm.run();
    // dwell
    
    main_yasm.run();
    main_yasm.run();

    // move back
    main_yasm.run();
    main_yasm.run();

    // start again

    main_yasm.run();
    main_yasm.run();

    pos_feeding = false;
    main_yasm.run();
    main_yasm.run();

    //stop
    printf("test_fsm: setting bouncing false\n");
    async_bouncing = false;
    main_yasm.run();
    printf("2nd run next\n");
    main_yasm.run();

    // run

    // halt

    printf("done fsm_async_bounce\n");




}


int main( int argc, char **argv) {
    
    UNITY_BEGIN();
    std::cout << "starting fsm bounce\n";
    RUN_TEST(fsm_async_bounce);

    std::cout << "test done\n";

   
    UNITY_END();
    return 0;
}



