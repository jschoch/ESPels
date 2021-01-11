#ifdef UNIT_TEST

#include <unity.h>
#include "src/gear.h"
#include <iostream>


void printJump(char *s,gear::Jump j){
    std::cout << s << "jump delta " << j.delta  << " , ";
    std::cout << "count " << j.count << " , ";
    std::cout << "error" << j.error<< "\n";


}



int main( int argc, char **argv) {
    UNITY_BEGIN();
    
    
    /* 
    
        what is a jump?
        struct of count, delta, and error

        k = encoder_pulses per step

        count returned by jump is count+k where k is calculated by next_jump_forward or next_jump_reverse
    
    */


   /*

   What is a Range?

   it seems range just runs next_jump_forward and next_jump_reverse and returns the two 
   results as {Jump next, Jump prev}, the ranges are only calculated once the encoder count reaches the count in the
    next/prev jumps

   */

    gear::State state;
    state.output_position = 0;
    int micro_steps = 8;
    int motor_steps = 200 * micro_steps;
    //float pitch = 4;
    float pitch = 0.1;
    float lead_screw_pitch = 2.0;
    int enc_res = 2400;

    int den = lead_screw_pitch * enc_res;
    int nom = motor_steps * pitch;

    std::cout << "nom: " << nom << " / den: " << den << "\n";
    std::cout << "test empty " << state.output_position << "\n";
    


    int start = -10;
    int stop = 169;
    state.output_position = start;
    bool test = state.setRatio(nom,den);

    TEST_ASSERT_EQUAL_INT(1,(int)test);

    int e = 0;


    // this changes the formula
    bool dir = true;
    state.calc_jumps(start,dir);

    std::cout << "starting jumps: " << state.jumps.prev << " - " << state.jumps.next << "\n";
    
    for(int i = start;i < stop + 1;i++){
        // if the "encoder" is incrementing

        // calculate both forward and reverse
        
        if(i == state.jumps.next ) {
            state.calc_jumps(i,dir);
            //std::cout << "+ p:" << gear::state.jumps.prev << " -- count: "<<  i << " -- n: " 
                //<< gear::state.jumps.next << "+";
            std::cout << "*" << i << "," << state.jumps.next << "," << state.jumps.prev << " ";
        }


    }

    //state.calc_jumps(stop,dir);
    state.jumps.prev = state.jumps.last;
    std::cout << "\nREVERSE\n\n";
    std::cout << "next: " << state.jumps.next << " prev: " << state.jumps.prev << " err: " << state.perror << "\n";

    for(int i = stop +2;i > (start + 1);i--){
        if(i == state.jumps.prev){
            state.calc_jumps(i,dir);
            //std::cout << "@" << i << " ";
            std::cout << "@" << i << "," << state.jumps.next << "," << state.jumps.prev << " ";
            //std::cout << "next: " << state.jumps.next << " prev: " << state.jumps.prev << "\n";
        }
    }

    return 0;
    UNITY_END();
}

#endif