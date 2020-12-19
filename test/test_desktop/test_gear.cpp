#ifdef UNIT_TEST

#include <unity.h>
#include "src/gear.h"
#include <iostream>


void printJump(gear::Jump j){
    std::cout << "jump delta " << j.delta  << "\n";
    std::cout << "jump count " << j.count << "\n";
    std::cout << "jump error" << j.error<< "\n";


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
    std::cout << gear::state.N << "\n";
    


    std::cout << " gear range next count" << gear::range.next.count << " \n";
    bool dir = false;
    int steps = 0;
    gear::state.output_position = 50;
    gear::range.next_jump(dir,50);
    for(int i = 50; i < 100;i++){
        gear::state.output_position++;

        if(i == gear::range.next.count){
            // calculate next jumps only after the encoder reaches the value

            gear::range.next_jump(dir,i);
            gear::state.err = gear::range.next.error;
            std::cout << "\n\t\t\t\t STEP on encoder count: " << i << "\n" ;
            steps++;
        }else{

            std::cout << "else on count " << i << "\n";
        }
    }


    std::cout << "total steps: " << steps << "\n";

    gear::state.output_position = 96;

    std::cout << "reversing encoder for 50 tics\n";
    dir = !dir;
    gear::state.err = gear::range.prev.error;
    gear::range.next_jump(dir,96);

    for(int i = 96; i > 50;i--){
        // ?? doing this correctly?
        gear::state.output_position--;

        if(i == gear::range.next.count){
            // calculate next jumps only after the encoder reaches the value

            gear::range.next_jump(dir,i);
            gear::state.err = gear::range.next.error;
            std::cout << "\n\t\t\t\t STEP on encoder count: " << i << "\n" ;
            steps++;
        }

        else{
            std::cout << "else calculated on encoder count: " << i << "\n" ;
            steps--;
        }

    }
    // manual calculation of jump based on input N and D
    gear::Jump j = gear::next_jump_forward(1,1,0,0);
    printJump(j);

    return 0;
    UNITY_END();
}

#endif