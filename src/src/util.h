#pragma once
#include <Bounce2.h>

enum display_mode_t {
  STARTUP,
  CONFIGURE,
  DSTATUS,
  READY,
  FEEDING,
  SLAVE_READY
} ;

enum feed_mode_t {
  RAPID_FEED,
  FEED,
  THREAD
} ;

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