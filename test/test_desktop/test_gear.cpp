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

    gear::state.output_position = 0;
    int motor_steps = 200 * 32;
    float pitch = 0.1;
    float lead_screw_pitch = 2.0;
    int enc_res = 2400;

    int dem = lead_screw_pitch * enc_res;
    int nom = motor_steps * pitch;

    std::cout << "nom: " << nom << " dem: " << dem << "\n";
    gear::state.N = nom;
    gear::state.D = dem;
    std::cout << "test empty " << gear::state.output_position << "\n";
    


    gear::state.output_position = 0;
    int d = gear::state.D;
    int n = gear::state.N;
    std::cout << "d: " << d << " n: " << n <<"\n";
    int e = 0;


    // this changes the formula
    bool dir = true;
    gear::calc_jumps(0,dir);
    
    for(int i = 0;i < 301;i++){
        // if the "encoder" is incrementing

        // calculate both forward and reverse
        
        if(i == gear::state.jumps.next ) {
            gear::calc_jumps(i,dir);
            //std::cout << "+ p:" << gear::state.jumps.prev << " -- count: "<<  i << " -- n: " 
                //<< gear::state.jumps.next << "+";
            std::cout << "+" << i << " ";
        }


    }

    std::cout << "\nREVERSE\n\n";
    std::cout << "prev is: " << gear::state.jumps.prev << " err: " << gear::state.perror << "\n";

    for(int i = 400;i > -1;i--){
        if(i == gear::state.jumps.prev){
            gear::calc_jumps(i,dir);
            std::cout << "-" << i << " ";
        }
    }

    return 0;
    UNITY_END();
}

#endif