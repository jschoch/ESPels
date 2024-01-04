#pragma once
#include "genStepper.h"
#include "moveConfig.h"

extern volatile bool feeding;

void init_motion(void);
void IRAM_ATTR calcDelta(void);
void IRAM_ATTR processMotion(void);
void start_rapid(double distance);
void init_hob_feed(void);
void init_pos_feed();


