#ifdef UNIT_TEST

#include <unity.h>
#include <iostream>


void playPos(){

    std::cout << "bsz\n";
}

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
    //TEST_ASSERT_TRUE(false);
}

int main( int argc, char **argv) {
    UNITY_BEGIN();

    //Pos startPos;
    
    std::cout << "foo\n";
    //Serial.println("foobar");
    RUN_TEST(playPos);    

    return 0;
    UNITY_END();
}

#endif
