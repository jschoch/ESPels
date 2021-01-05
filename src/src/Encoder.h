#pragma once

#include "config.h"

enum Quadrature{
  ON, //!<  Enable quadrature mode CPR = 4xPPR
  OFF //!<  Disable quadrature mode / CPR = PPR
};

enum Direction{
    CW      = 1,  //clockwise
    CCW     = -1, // counter clockwise
    UNKNOWN = 0   //not yet known or invalid state
};

/**
 *  Pullup configuration structure
 */
enum Pullup{
    INTERN, //!< Use internal pullups
    EXTERN //!< Use external pullups
};

void init_encoder(void);
void do_rpm(void);

// sign function
#define _sign(a) ( ( (a) < 0 )  ?  -1   : ( (a) > 0 ) )
#define _round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
#define _constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

// utility defines
#define _2_SQRT3 1.15470053838
#define _SQRT3 1.73205080757
#define _1_SQRT3 0.57735026919
#define _SQRT3_2 0.86602540378
#define _SQRT2 1.41421356237
#define _120_D2R 2.09439510239
#define _PI 3.14159265359
#define _PI_2 1.57079632679
#define _PI_3 1.0471975512
#define _2PI 6.28318530718
#define _3PI_2 4.71238898038

class Encoder {
 public:
 Encoder(int encA, int encB , float ppr);

 /** encoder initialise pins */
 void init();
 void enableInterrupts(void (*doA)() = nullptr, void(*doB)() = nullptr);
    
    //  Encoder interrupt callback functions
    /** A channel callback function */
    void handleA();
    /** B channel callback function */
    void handleB();
    float getAngle();
    float initRelativeZero();
    float initAbsoluteZero() ;
    int pinA; //!< encoder hardware pin A
    int pinB; //!< encoder hardware pin B
    int cpr;
    Pullup pullup;
    int quadrature;
    int natural_direction = Direction::CW;
    void setCount(int64_t count);
    int64_t getCount();
    volatile int64_t pulse_counter;
    volatile int64_t prev_pulse_counter;
    volatile bool dir;

private:
    //!< current pulse counter
    volatile long pulse_timestamp;//!< last impulse timestamp in us
    volatile int A_active; //!< current active states of A channel
    volatile int B_active; //!< current active states of B channel
    float prev_Th, pulse_per_second;

    volatile long prev_timestamp_us;

};