#include <Arduino.h>
#include "util.h"
#include "config.h"






const char* DISPLAY_MODE [] =
{
  "Startup",
  "Configure",
  "Status",
  "Ready",
  "Feeding",
  "Slave Ready"
};


/* i hate C

void menu_next(int *index, (char**) ary){
  *index++;
  int size = sizeof(ary) / sizeof ary[0];
  if(*index > size){
    *index = 0;
  }
}
void menu_prev(int *index, const char* ary){
  *index--;
  int size = sizeof(ary) / sizeof ary[0];
  if(*index < size){
    *index = size;
  }
}
*/
