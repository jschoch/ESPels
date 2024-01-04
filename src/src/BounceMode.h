#pragma once
#include "yasm.h"



extern volatile bool bouncing;
extern double old_jog_mm;

void BounceMoveState();
void BounceIdleState();
void BounceRapidState();
void BounceMoveDwellState();
bool maybeStopBounce();

