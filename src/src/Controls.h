#ifndef _Controls_h
#define _Controls_h

#include <Arduino.h>
#include "config.h"
#include "yasm.h"

//#include <neotimer.h>

// btn pins


// Left Button
#define LBP 15
// Right Button
#define RBP 2
// Menu switch button
#define SBP 14
// Up Button
#define UBP 4
// Down Button
#define DBP 16 
// Modifier button
#define MBP 0

extern YASM btn_yasm;

void init_controls(void);
void read_buttons(void);

void handleLBP(void);
void handleRBP(void);
void handleSBP(void);
void handleUBP(void);
void handleDBP(void);
void setHobbFactor(void);
void feed_parameters(void);
void startupState(void);
void slaveJogReadyState(void);
void feedingState(void);
void slaveJogStatusState(void);
void resetToolPos(void);
void updateMode(RunMode run);
void slaveJogPosState();

#endif
