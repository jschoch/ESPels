#pragma once

void init_motion(void);
void IRAM_ATTR calcDelta(void);
void IRAM_ATTR processMotion(void);
//void calcDelta(void *pvParaeters);

