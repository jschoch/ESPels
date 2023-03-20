#include "Stepper.h"
#include "state.h"


int microsteps = Z_MICROSTEPPING;

int native_steps = Z_NATIVE_STEPS_PER_REV;
int motor_steps = Z_MICROSTEPPING * Z_NATIVE_STEPS_PER_REV;



// this is the main flag for signaling movement to begin
volatile bool pos_feeding = false;


