#ifndef _Stepper_h
#define _Stepper_h

#include <Arduino.h>
#include "config.h"


void init_stepper(void);
bool getDir(void);
bool setDir(bool);
void updatePosition();
void init_feed();

#endif
