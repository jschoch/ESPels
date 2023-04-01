#pragma once
#include "yasm.h"

extern YASM bounce_yasm;

extern volatile bool bouncing;
extern double old_jog_mm;

void BounceMoveState();
void BounceIdleState();
void BounceRapidState();
