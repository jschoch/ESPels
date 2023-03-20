#include <Arduino.h> //redirects to ArduinoFake.h in this case
#include <unity.h>
#include <iostream>
#include <ArduinoJson.h>
#include "src/jsonValidation.h"

using namespace fakeit;

void tearDown(void)
{
    // clean stuff up here
    // TEST_ASSERT_TRUE(false);
}

void setUp(void)
{
    ArduinoFakeReset();
}

/*
void setup()
{
}
void loop()
{
}
*/

void test_validate()
{
    std::cout << "validate this\n";

    char buff[1000];

    MCDOC start_struct = {
        1.0, // move pitch
        1.0, // rapid pitch
        100000, // accel
        1, // dwell
        1, // moveSteps
        true, // f
        true, // startSync
        true, // feeding_ccw
        false
    };

    StaticJsonDocument<200> doc;
    static const char* raw = "{\"cmd\":\"world\"}";
    deserializeJson(doc, raw);
    JsonObject object = doc.as<JsonObject>();
    bool otest = object.isNull();
    TEST_ASSERT(!otest);

    //bool is_valid_json = validateJson(raw);
    bool is_valid_json = validateJson("foo");
    TEST_ASSERT(!is_valid_json);
    is_valid_json = validateJson(raw);
    TEST_ASSERT(is_valid_json);


    std::cout << "test: missing f param \n";
    StaticJsonDocument<1000> badMoveConfig;
    JsonObject c = badMoveConfig.createNestedObject();
    c["movePitch"] = 1.0;
    c["rapidPitch"] = 1.1;
    c["accel"] = 100000;
    //c["f"] = true;

    MCDOC validMoveConfig = validateMoveConfig(c);

    TEST_ASSERT(!validMoveConfig.valid);

    c["movePitch"] = "not ";
    c["rapidPitch"] = "not a float";
    c["accel"] = 100000;
    c["f"] = true;
    c["dwell"] = 0;
    c["moveSteps"] = start_struct.moveSteps;
    c["startSync"] = start_struct.startSync;

    std::cout << "test bad float \n";

    serializeJsonPretty(c,buff);
    validMoveConfig = validateMoveConfig(c);
    printf("moveConfig doc: bad float\nc.movepitch: %d\n  %s\n",validMoveConfig.movePitch,buff);
    printf("%s is<double> %d",c["rapidPitch"],c["rapidPitch"].is<double>());
    

    TEST_ASSERT(!validMoveConfig.valid);

    


    c["movePitch"] = start_struct.movePitch;
    c["rapidPitch"] = start_struct.rapidPitch;
    c["accel"] = start_struct.accel;
    c["f"] = start_struct.f;
    c["dwell"] = start_struct.dwell;
    c["moveSteps"] = start_struct.moveSteps;
    c["feeding_ccw"] = start_struct.feeding_ccw;
    c["startSync"] = start_struct.startSync;

    
    serializeJsonPretty(c,buff);
    printf("moveConfig doc:\n  %s\n",buff);
    validMoveConfig = validateMoveConfig(c);
    printMCDOC(validMoveConfig);
    TEST_ASSERT(validMoveConfig.valid);

    TEST_ASSERT(start_struct == validMoveConfig);
    std::cout << "done validate\n";
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
     //When(Method(ArduinoFake(), Serial)).AlwaysReturn();
    std::cout << "json validation test\n";
    RUN_TEST(test_validate);
    UNITY_END();
    return 0;
}
