#pragma once

//I suggest making a leadscrew class that owns things like this, leadscrew measurement system, etc...
extern double lead_screw_pitch;
extern int microsteps;

extern double pitch;
extern double oldPitch;

//threading parameters should be owned by and controlled by a thread class, 
// which inherits from a feeding class, which inherits from a simple movement class.
// that way you can pass the object in to the motion control class as params and it knows everything it needs to
// about the current mode, such as encoder steps per spindle rev. It can be passed in the event into the 
// state machine handlers too, for switching modes, etc
// jog can inherit from feeding
// feeding can contain a Bounce object, containing params for the bounce behavior. It can always run the bounce logic
// and depending on the Bounce object params, do the auto return to the start or not, in any mode.

extern int tpi;
extern double depth;

extern double rapids;
extern double backlash;

void init_machine(void);