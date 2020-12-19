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
    gear::state.output_position = 0;
    gear::state.N = 1;
    gear::state.D = 6;
    std::cout << "test empty " << gear::state.output_position << "\n";
    std::cout << gear::state.N << "\n";
    


    std::cout << " gear range next count" << gear::range.next.count << " \n";
    bool dir = false;
    int steps = 0;

    for(int i = 0; i < 50;i++){
        gear::state.output_position++;
        /*
        std::cout << " 'encoder' count: " << i << " \n";
        
        std::cout << "Jump: next\n\t";
        printJump(gear::range.next);

        std::cout << "\nJump: prev\n\t";
        printJump(gear::range.prev);

        */

        if(i == gear::range.next.count){
            // calculate next jumps only after the encoder reaches the value

            gear::range.next_jump(dir,i);
            gear::state.err = gear::range.next.error;
            std::cout << "\n\t\t\t\t STEP on encoder count: " << i << "\n" ;
            steps++;
        }else{
            /*
            bool tdir = !dir;
            gear::state.err = gear::range.prev.error;
            gear::range.next_jump(tdir,i);
            std::cout << "\n\t\t\t\t not \n" ;
            */
        }
    }

    std::cout << "total steps: " << steps << "\n";

    // manual calculation of jump based on input N and D
    gear::Jump j = gear::next_jump_forward(1,1,0,0);
    printJump(j);

    /* 
    
        what is a jump?
        struct of count, delta, and error

        k = encoder_pulses per step

        count returned by jump is count+k where k is calculated by next_jump_forward or next_jump_reverse
    
    */


   /*

   What is a Range?

   it seems range just runs next_jump_forward and next_jump_reverse and returns the two 
   results as {Jump next, Jump prev}

   */

    return 0;
    UNITY_END();
}

#endif