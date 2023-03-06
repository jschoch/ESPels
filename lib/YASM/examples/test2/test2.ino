/* YASM test file 2
  
This file is not relly intended to be a use case example but was used to 
test and debug next() functionnalities.

Running this sketch you should get in serial monitor :

YASM test2
loop 0 fisrtRun state3 0 0 
loop 1          state3 1 1 
loop 2  
loop 3          state3 2 2 
loop 4  
loop 5  
loop 6 fisrtRun state3 3 0 
loop 7          state3 4 1 
loop 8 fisrtRun state3 5 0 
loop 9          state3 6 1 
loop 10          state3 7 2 
loop 11  
loop 12          state3 8 3 
loop 13          state3 9 4 
loop 14          state3 10 5 
loop 15          state3 11 6 
loop 16 fisrtRun state3 12 0 
loop 17          state3 13 1 
loop 18  
loop 19 fisrtRun state3 14 0 
loop 20          state3 15 1

*/



#include <yasm.h>
YASM Machine; //the state machine we will use in this example

void setup() {
    Serial.begin(9600); 
    Machine.next(S3); 
    Serial.println("YASM test2");
}

void loop() {
    static int j = 0;
    if (j>20) return;  

    Serial.print("loop "); Serial.print(j++); Serial.print(" ");
    Machine.run(); 
    
    if (j==2) Machine.stop();    
    if (j==3) Machine.resume();
    if (j==4) Machine.stop();
    if (j==6) Machine.next(S3);
    if (j==8) Machine.next(S3,true);
    if (j==12) Machine.resume();
    if (j==18) {Machine.next(S3,true);Machine.stop();}
    if (j==19) Machine.resume();
    
    Serial.println(" ");
}

//////////////// state machine states //////////////////
void S3() {
    static int i =0;
    if (Machine.isFirstRun()) Serial.print("fisrtRun");
    else Serial.print("        ");
    Serial.print(" state3 "); Serial.print(i++); Serial.print(" ");
    Serial.print(Machine.runCount());
    
    if(i==8) Machine.stop();
    if(Machine.runCount() == 5) Machine.next(S3);
    if(Machine.runCount() == 6) Machine.next(S3,true);
}
