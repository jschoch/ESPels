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

    StaticJsonDocument<1000> badMoveConfig;
    JsonObject c = badMoveConfig.createNestedObject();
    c["movePitch"] = 1.0;
    c["rapidPitch"] = 1.1;
    c["accel"] = 100000;
    //c["f"] = true;
    c["dwell"] = 0;

    Vres validMoveConfig = validateMoveConfig(c);

    TEST_ASSERT(!validMoveConfig.test);

    c["movePitch"] = "not ";
    c["rapidPitch"] = "not a float";
    c["accel"] = 100000;
    c["f"] = true;
    c["dwell"] = 0;

    validMoveConfig = validateMoveConfig(c);

    TEST_ASSERT(!validMoveConfig.test);


    c["movePitch"] = 1.0;
    c["rapidPitch"] = 1.1;
    c["accel"] = 100000;
    c["f"] = true;
    c["dwell"] = 0;

    validMoveConfig = validateMoveConfig(c);
    TEST_ASSERT(validMoveConfig.test);
    //return;
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
