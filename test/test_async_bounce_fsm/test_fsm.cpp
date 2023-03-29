
#include <unity.h>
#include <iostream>
#include <yasm.h>

#include "src/mocks.h"
#include <test_lib_helper.h>
#include "src/AsyncBounceMode.h"
#include <ctime>
//#include <chrono>
//#include "src/mocks.h"

using namespace fakeit;


void now(){
    //std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    //auto foo = std::chrono::high_resolution_clock::now();

}

void setUp(void) {
    // set stuff up here
    ArduinoFakeReset();
    
}

void tearDown(void) {
    // clean stuff up here
}

static int thetime = 1;

void fsm_async_bounce(){
    printf("starting fsm_async_bounce..\n");
    When(Method(ArduinoFake(), millis)).Return(thetime++);

    async_bounce_yasm.run();

    // start the fsm
    async_bounce_yasm.next(AsyncBounceIdleState);
    //my_fsm.next(S1);
    //my_fsm.run();

    async_bounce_yasm.run();

    async_bounce_yasm.run();


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
