#include <Arduino.h> //redirects to ArduinoFake.h in this case
#include <unity.h>
#include <iostream>
#include "src/genStepper.h"

using namespace fakeit;


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

}

int main( int argc, char **argv) {
    UNITY_BEGIN();
    std::cout << "stepper test\n";
    return UNITY_END();
}
