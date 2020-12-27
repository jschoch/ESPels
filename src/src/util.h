#pragma once
#include <Bounce2.h>

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
  SLAVE_JOG_READY,
  DEBUG_READY,
  RUNNING //  Any mode where we are actually moving.  
};

enum feed_mode_t {
  RAPID_FEED,
  FEED,
  THREAD
} ;


#define zPos  1
#define zNeg  -1

// button data definition

struct Bd{
  uint8_t pin;
  const char* name;
  bool changed;
  bool change_read;
  bool state;
  Bounce *deb;
  uint8_t idx;
};
