# YASM
## Yet Another State Machine library - a function pointer based state machine library for arduino

*A library for easy creation and managing of states machines based on function pointers.*

## Installation :

Library now directly available from Arduino IDE library manager. Just select YASM in the list and click install.


## Quick start :

First you need to include the library header, then create the state machine object with :
```cpp
#include "yasm.h"

YASM MyMachine; 
```

Then create the states you need. Each state must be a separate function of type "void" with no parameter :
```cpp
void State1()
{
  do_something;  
  if (need_trigger_state_change)  MyMachine.next(State2);
}

void State2()
{
  .....
```

Then initialise the machine telling it wich one is the initial state :
```cpp
void setup() {
  MyMachine.next(State1);
}
```

Then call repeatedly the update function from your main loop :
```cpp
void loop() {
  MyMachine.run();
}
```

Most important thing to remind is the state functions should be non-blocking so the whole system remains responsive no matter
how complex the task is. So you may avoid using `while` or `delay()` or anything that may keep the processor busy for a long time.
If you need a delay before doing something when entering a state, instead of `delay()` use the provided timing functions.
For example if you need a 3 seconds delay when you enter State3, use `elapsed(delay)` function :
```cpp
void State3()
{
  do_something_imediately;
  if (elapsed(3E3)) do_something_only_after_a_3s_delay;
}
```

## Complete tour : 

The YASM class provide the following control and timing functions and datas :
```cpp
class YASM{
	public:
		YASM();
		void next(void (*)(), bool);
		bool run();
		void stop();
		void resume(bool);
		unsigned long timeOnState();
		bool elapsed(unsigned long);
		bool periodic(unsigned long);
		bool isFirstRun();
		unsigned int runCount();
		bool isInState(void (*)());
```		
    
`YASM() `
is class constructor, called when creating your state machine object :
`YASM MyMachine;`




```cpp
void next(void (*nextstate)(), bool reset=false) 
```
is the function to call when requesting a state change or when giving the initial state to te machine :
`MyMachine.next(StateX);`
The optional "reset" bool parameter is usefull to force the reset running counter and timers if a state re-enters itself, or needs to be reset by external process :
```cpp
void State1()
{
  do_something;  
  if (need_reset_current_state)  MyMachine.next(State1, true);
}
```

`bool run() `
is the update function. You need to keep calling this function to keep the machine running. 
Each time the function is called the current state function is launched. It returns true if the machine is actually running or false if it is stopped (or not initialized).

`void stop() `
stops the machine. This function can be called at any moment, even from inside the current running state. It causes the machine to stop so any further call to run() will do nothing and return false. It also records what the current state was so it can be resumed by :

`void resume(bool reset=false)` 
resume the machine execution to the last used state. 
The optional "reset" bool parameter acts just like in next(). Usefull sometimes (menus, etc)
A stopped machine can also be restarted using `next(state)` to restart from another state.

`unsigned long timeOnState() `
returns the time in milliseconds spent in the current state since the last state change.

`bool elapsed(timeout) `
returns true if the timeout (in ms) is elapsed since last state change. It is very convenient to implement delays. Please see BlinkLed example.

`bool periodic(period); `
returns true periodically, everytime it is called after the period time (in ms) is elapsed (while in the same state)

`bool isFirstRun() `
returns true if called during the first execution of a state, false otherwise.
Please see blinkLed2 example.

`unsigned int runCount()`
returns the number of time current state have been called since the last state change.
Usefull to implement non blocking `for` loops inside a state.

`bool isInState(void (*pstate)())`
returns true if the given pointer is equal to the running state pointer, false otherwise. This is usefull if you need to know from outside the machine if it is in a particular state.





