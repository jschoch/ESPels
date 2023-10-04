#pragma once
#include "genStepper.h"
#include "moveConfig.h"

extern volatile bool feeding;


// i think this reverses everything, usecase is unclear
// should look at refactoring it out but unclear impact
// this is replaced by mc.feeding_ccw
//extern volatile bool feeding_ccw;

void init_motion(void);
void IRAM_ATTR calcDelta(void);
void IRAM_ATTR processMotion(void);
void start_rapid(double distance);
void init_hob_feed(void);
//void calcDelta(void *pvParaeters);
//void startCalcTask(void);

void init_pos_feed();


