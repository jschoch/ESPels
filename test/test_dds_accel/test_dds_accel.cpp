#ifdef UNIT_TEST

#include <unity.h>
#include <iostream>


uint8_t   Direction=0;
#define   MICROSTEPS       16

int32_t   SpeedNow         = 0;
int32_t   TargetSpeed      = 2000 * 65536*MICROSTEPS;
uint16_t  Acceleration     = 4000 * MICROSTEPS;
uint16_t  DDS_Accumulator  = 0;

#define   HOLDSPEED_STEPS   20000

#define   STATE_ACCELERATE  0
#define   STATE_HOLDSPEED   1
#define   STATE_DECELERATE  2

uint8_t   state             = STATE_ACCELERATE; // initial state

uint32_t  HoldSpeed_Counter = 0;


void accel_loop(){
  DDS_Accumulator+=(SpeedNow>>16);
  //digitalWrite(STEPPER_PULSE_PIN,DDS_Accumulator>>15);
  //delayMicroseconds(10);
  auto s = DDS_Accumulator>>15;
  std::cout << " accumulator state was: " << s << "\n";

  switch(state)
  {
    case STATE_ACCELERATE:{
      SpeedNow+=Acceleration;
      if(SpeedNow>TargetSpeed){
        state=STATE_HOLDSPEED;
        HoldSpeed_Counter=0;
      }
    }break;

    case STATE_HOLDSPEED:{
      HoldSpeed_Counter++;
      if(HoldSpeed_Counter>HOLDSPEED_STEPS)
      {
        state=STATE_DECELERATE;
      }
    }break;

    case STATE_DECELERATE:{
       SpeedNow-=Acceleration;
       if(SpeedNow<0)
       {
         SpeedNow=0;
         state=STATE_ACCELERATE;
         Direction^=1; // reverse direction
         //digitalWrite(DIRECTION_PIN,Direction);
         std::cout << "state decelerate speednow<0\n";
       }
    }break;

    default:{
        state=STATE_ACCELERATE;
    }
  }
}

int distance_in_steps; // distance to travel in steps
float acceleration_mm_m2; // acceleration in mm/m2
float max_speed = 1000; // maximum speed in mm/s

float steps_per_mm = 600; // example value
float max_speed_steps_per_sec = max_speed * steps_per_mm;

void accelerate(int distance_in_steps, float acceleration_mm_m2, float max_speed) {
  float current_speed = 0.0;
  float current_position = 0.0;
  float current_velocity = 0.0;
  float time_step = 0.0001; // example value
  float t = 0.0;
  float dds_accumulator = 0.0;
  float dds_increment = 0.0;
  float dds_overflow = 1.0;
  int ticker = 0;

  // calculate the time to reach maximum speed
  float time_to_max_speed = max_speed_steps_per_sec / acceleration_mm_m2;
  // calculate the distance covered during acceleration
  float distance_covered = 0.5 * acceleration_mm_m2 * time_to_max_speed * time_to_max_speed;
  std::cout << "ttms: " << time_to_max_speed << "distance for acceleration: " << distance_covered << "\n";

  // accelerate until we reach the maximum speed
  while (current_speed < max_speed_steps_per_sec) {
    // calculate the incremental step rate using DDS
    dds_increment = dds_overflow * acceleration_mm_m2 * time_step;
    dds_accumulator += dds_increment;
    if (dds_accumulator >= 1.0) {
      dds_overflow = dds_accumulator - 1.0;
      dds_accumulator -= 1.0;
      current_velocity += time_step * acceleration_mm_m2;
      current_position++;
      std::cout << "step here: " << dds_accumulator << " tick: " << ticker <<" step count: " << current_position << "\n";
    }
    else {
      dds_overflow = dds_accumulator;
    }
    // calculate the current position and speed
    //current_position += current_velocity * time_step;
    ticker++;
    current_speed += dds_increment * max_speed_steps_per_sec;

    // check if we have reached the end of the acceleration phase
    if (current_position >= distance_covered) {
        std::cout << "end acceleration phase\n";
      break;
    }
    std::cout << "accel p: " << current_position << " velocity: " << current_velocity << "speed: " << current_speed << "\n";
  }

  // maintain maximum speed until we reach the destination
  while (current_position < distance_in_steps) {
    //current_position += current_speed * time_step;
    current_position++;
    std::cout << "coast p: " << current_position << " speed: " << current_speed << "\n";
  }

  // decelerate until we reach a stop
  while (current_speed > 0.0) {
    // calculate the incremental step rate using DDS
    dds_increment = dds_overflow * acceleration_mm_m2 * time_step;
    dds_accumulator += dds_increment;
    if (dds_accumulator >= 1.0) {
      dds_overflow = dds_accumulator - 1.0;
      dds_accumulator -= 1.0;
      current_velocity -= time_step * acceleration_mm_m2;
    }
    else {
      dds_overflow = dds_accumulator;
    }
    // calculate the current position and speed
    //current_position += current_velocity * time_step;
    current_position++;
    current_speed -= dds_increment * max_speed_steps_per_sec;
    std::cout << "decel p: " << current_position << " speed: " << current_speed << "\n";
  }

}

#include <cmath>

struct LinearAcceleration {
    double max_speed = 0;
    double acceleration = 0;
    double acceleration_distance = 0;
    double deceleration = 0;
    double deceleration_distance = 0;
    double distance = 0;
    double time_at_max_speed = 0;
    double constant_speed_distance = 0;
};


constexpr LinearAcceleration calculate_linear_acceleration(double max_speed, double acceleration, double distance) {
    //LinearAcceleration result = {0,0,0,0}; 
    LinearAcceleration result;
    result.max_speed = max_speed;
    result.acceleration = acceleration;
    result.deceleration = acceleration;
    result.distance = distance;

    // Calculate the time it takes to reach the max speed
    double time_to_max_speed = max_speed / acceleration;

    // Calculate the distance traveled during the acceleration phase
    result.acceleration_distance = 0.5 * acceleration * time_to_max_speed * time_to_max_speed;

    // Calculate the time it takes to decelerate to a stop
    double time_to_stop = std::sqrt(2.0 * distance / acceleration);

    // Calculate the distance traveled during the deceleration phase
    result.deceleration_distance = 0.5 * acceleration * time_to_stop * time_to_stop;

    // Check if the distance is long enough to allow reaching the max speed
    if (distance >= 2.0 * result.acceleration_distance + 2.0 * result.deceleration_distance) {
        // Calculate the distance traveled during the constant speed phase
        result.constant_speed_distance = distance - (2.0 * result.acceleration_distance);

        // Calculate the time spent traveling at the max speed
        result.time_at_max_speed = result.constant_speed_distance / max_speed;

        // Adjust the deceleration to ensure that the total time is correct
        result.deceleration = max_speed / time_to_stop - acceleration;

        // Recalculate the distance traveled during the deceleration phase
        result.deceleration_distance = 0.5 * (acceleration + result.deceleration) * time_to_stop * time_to_stop;


        result.distance = result.acceleration_distance + result.constant_speed_distance + result.deceleration_distance;
    }
    
    return result;
}




void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}



int main2( int argc, char **argv) {
    UNITY_BEGIN();
    std::cout << "starting chatgpt algo tests\n";

    //accelerate(int distance_in_steps, float acceleration_mm_m2, float max_speed) 
    //accelerate(500,500,1000);
    //std::cout << "as: " << accel_steps << " cs: " << constant_steps << "ds: " << decel_steps << "\n";


    //constexpr LinearAcceleration calculate_linear_acceleration(double max_speed, double acceleration, double distance)
     constexpr auto ac= calculate_linear_acceleration(10000.0, 500.0, 1000.0);

    std::cout << "max_speed " << ac.max_speed << " acel time" << ac.acceleration << " coast time " << ac.time_at_max_speed << " deceleration time " << ac.deceleration << " d " << ac.distance << "\n";

   
    UNITY_END();
    return 0;
}


#endif
