#pragma once

#ifdef HAS_BUTTONS
#include <Bounce2.h>
#endif

enum display_mode_t {
  STARTUP,
  CONFIGURE,
  DSTATUS,
  READY,
  FEEDING,
  SLAVE_READY,
  DEBUG_READY
} ;

enum class RunMode{
  STARTUP,
  SLAVE_READY,
  SLAVE_JOG_READY, // 2
  DEBUG_READY, //3 
  RUNNING, //  4 Any mode where we are actually moving.  
  BounceIdle, // 5
  BounceJog, // 6
  BounceRapid, //7
  BounceFinish, //8
  HOB_READY, // 9
  HOB_RUN, // 10
  HOB_STOP, // 11
  DIVIDER_READY, //12
  DIVIDER_RUN //13
};

enum feed_mode_t {
  RAPID_FEED,
  FEED,
  THREAD
} ;


#define zPos  1
#define zNeg  0

// button data definition
#ifdef HAS_BUTTONS
struct Bd{
  uint8_t pin;
  const char* name;
  bool changed;
  bool change_read;
  bool state;
  Bounce *deb;
  uint8_t idx;
};
#endif
