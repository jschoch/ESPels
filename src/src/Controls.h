#ifndef _Controls_h
#define _Controls_h

#include <Arduino.h>
#include "config.h"
#include "yasm.h"

void init_controls(void);

void feed_parameters(void);
void startupState(void);
void slaveJogReadyState(void);
void feedingState(void);
void slaveJogStatusState(void);
void resetToolPos(void);
void updateMode(RunMode run);
void slaveJogPosState();

#endif
