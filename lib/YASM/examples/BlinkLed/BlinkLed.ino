/* 
   BlinkLed.ino example file for yasm library.
   
   The purpose of this arduino sketch is to blink a led on pin 13.
   
   It creates a state machine with two states : ledOn, ledOff, 
   and illustrate the use of elapsed(delay_in_ms) timing function to trigger 
   state change.                                                              
*/


#include <yasm.h>	//include the yasm library

#define OnDelay		500 //500ms led "On" state delay
#define OffDelay	750 //750ms led "Off" state delay

#define LedPin 		LED_BUILTIN  	// Most Arduino boards have a pin connected to an on-board LED in series with a resistor. 
					// The constant LED_BUILTIN is the number of the pin to which the on-board LED is connected. 
					// Most boards have this LED connected to digital pin 13

YASM led; //declaration of the "led" state machine

void setup()
{
	pinMode(LedPin, OUTPUT); //declare the led pin as output
	led.next(ledOn); //set the initial state of the led state machine
}

void loop()
{
	led.run();  //update the state machine
}

//////////led state machine/////////////

void ledOn()
{
	digitalWrite(LedPin, HIGH); //this is the "On" state so we turn on the led
	
	if(led.elapsed(OnDelay)) //check if the delay for the "on" state is elapsed
		led.next(ledOff); //if the delay is elapsed we switch to the "off" state
}

void ledOff()
{
	digitalWrite(LedPin, LOW); //this is the "Off" state so we turn off the led
	
	if(led.elapsed(OffDelay)) //check if the delay for the "off" state is elapsed
		led.next(ledOn); //if the delay is elapsed we switch to the "on" state
}
