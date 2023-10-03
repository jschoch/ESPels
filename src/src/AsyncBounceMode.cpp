#include <Arduino.h>
#include "yasm.h"
#include "genStepper.h"
#include "moveConfig.h"

//YASM async_bounce_yasm;
volatile bool async_bouncing = false;

#include <Ticker.h>
#include "AsyncBounceMode.h"

bool vTck::O::running = false;

Ticker vTcker;
