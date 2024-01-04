#include <Arduino.h>
#include "yasm.h"
#include "genStepper.h"
#include "moveConfig.h"

// the flag for the bounce
volatile bool async_bouncing = false;

#include <Ticker.h>
#include "AsyncBounceMode.h"

bool vTck::O::running = false;

Ticker vTcker;
