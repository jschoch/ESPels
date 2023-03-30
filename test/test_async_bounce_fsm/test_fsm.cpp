// hack for uncaught_exception deprication warnings
#undef _GLIBCXX_DEPRECATED




#include <unity.h>
#include <iostream>
#include <yasm.h>

#include "src/mocks.h"
#include <test_lib_helper.h>
#include "src/AsyncBounceMode.h"
#include <ctime>

using namespace fakeit;



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

    async_bounce_yasm.run();

    // start the fsm
    async_bounce_yasm.next(AsyncBounceIdleState);
    //my_fsm.next(S1);
    //my_fsm.run();

    async_bounce_yasm.run();

    async_bounce_yasm.run();

    // start bounce

    async_bouncing = true;
    
    async_bounce_yasm.run();
    async_bounce_yasm.run();
    async_bounce_yasm.run();
    // stop move
    pos_feeding = false;
    async_bounce_yasm.run();
    async_bounce_yasm.run();
    // dwell
    
    async_bounce_yasm.run();
    async_bounce_yasm.run();

    // move back
    async_bounce_yasm.run();
    async_bounce_yasm.run();

    // start again

    async_bounce_yasm.run();
    async_bounce_yasm.run();

    pos_feeding = false;
    async_bounce_yasm.run();
    async_bounce_yasm.run();

    //stop
    printf("test_fsm: setting bouncing false\n");
    async_bouncing = false;
    async_bounce_yasm.run();
    printf("2nd run next\n");
    async_bounce_yasm.run();

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



#define _GLIBCXX_DEPRECATED // end hack
