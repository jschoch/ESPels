#pragma once

//I suggest making a leadscrew class that owns things like this, leadscrew measurement system, etc...
//extern double lead_screw_pitch;
//extern int microsteps;

extern double backlash;

void init_machine(void);
