/* YASM test file

This file is not relly intended to be a use case example but was used to 
test and debug most functionnalities.

Running this sketch you should get in serial monitor :

YASM test
loop 0 first run state1 0 loop 0    -> first pass in a state detection
loop 1           state1 1 loop 1 
loop 2           state1 2 loop 2    ->no first pass detection because same state is called so we did not change state
loop 3           state1 3 loop 3    
loop 4                    loop 4    ->suspend machine execution from outside the machine
loop 5                    loop 5 
loop 6           state1 4 loop 6 	->resume machine execution
loop 7           state1 5 loop 7 
loop 8 fisrt run state2   loop 8 	->state change
loop 9                    loop 9    ->suspend machine from inside a state
loop 10                    loop 10 
loop 11                    loop 11 
loop 12                    loop 12 
loop 13           state2   loop 13  ->no first pass detection (resume same state)
loop 14           state2   loop 14 
loop 15                    loop 15 
loop 16 first run state1 6 loop 16 
loop 17 fisrt run state2   loop 17 
loop 18                    loop 18 
loop 19 first run state1 7 loop 19 
loop 20 fisrt run state2   loop 20 
loop 21                    loop 21 
loop 22                    loop 22 
loop 23                    loop 23 
loop 24                    loop 24 
loop 25                    loop 25 

*/



#include <yasm.h>

YASM Machine; //the state machine we will use in this example

void setup() 
{
  Serial.begin(9600); 
  Machine.next(S1); //here we give the state machine it's starting state
  Serial.println("YASM test");
}

void loop() 
{
  static int j = 0;
  if (j>25) return;  
  //This is a way to make nonblocking while or for loops.
  //Here it doesn't really matters because we are inside arduino's loop() and we 
  //enclose all the function, so behaviour will be the same as if we just did 
  //while(j<25), but if we where in some other place in the sketch, it may be 
  //necessary to return from the loop at each time we run it,not only when it finish.
  //It's often a good practice when using state machines to make the loops this way.
  
  Serial.print("loop ");
  Serial.print(j);
  Serial.print(" ");
  
  //check if machine is running (while running it) so we can do some output 
  //formatting for clarity in serial monitor
  if(!Machine.run()) Serial.print("                  "); 
  
  if (j==3) Machine.stop();
  if (j==5) Machine.resume();
  
  if (j==12) Machine.resume();
  if (j==13) Machine.resume();
  
  if (j==15) Machine.next(S1);  
  
  if (j==18) Machine.next(S1);
  
  Serial.print(" loop ");
  Serial.print(j);
  Serial.println(" ");

  j++;

}


//////////////// state machine states //////////////////

// state 1
void S1()
{
  static int i=0;
  
  if (Machine.isFirstRun()) Serial.print("first run");
  else Serial.print("         ");
  
  Serial.print(" state1 ");
  Serial.print(i);
  if (i==2) Machine.next(S1); //this does nothing more because we stay in the same state
  
  if (i>4) Machine.next(S2); //here we switch to another state
  i++;
}

// state 2
void S2()
{
  if (Machine.isFirstRun()) Serial.print("fisrt run");
  else Serial.print("         ");
  Serial.print(" state2  ");
  Machine.stop();
}


