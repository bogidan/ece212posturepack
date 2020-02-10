
#include "arduino.h"
#include "esp_log.h"

#include "core.h"
#include "_main.h"

const int pin_flex = register_pin(40, "flex");



// Measure the voltage at 5V and the actual resistance of your
// 47k resistor, and enter them below:
const float VCC = 3.3; // Measured voltage of Ardunio 5V line
const float R_DIV = 400000.0; // Measured resistance of 40k resistor

// Upload the code, then try to adjust these values to more
// accurately calculate bend degree.
const float STRAIGHT_RESISTANCE = 10000.0; // resistance when straight
const float BEND_RESISTANCE = 170000.0; // resistance at 90 deg



// Config
flex_mode_e config_flex_mode = FLEX_INSTANT;
int config_flex_instant_check = 1000; // 1s
int config_flex_instant_reset = 10000; // 10s
int config_flex_limit = 20; // degrees
int config_flex_average_check = 2000;
float config_flex_average_weight = 0.1;

static float readFlexAngle()
{
  // Read the ADC, and calculate voltage and resistance from it
  int flexADC = analogRead(pin_flex);
  flexADC = flexADC * 0.9;
  
  //Serial.println(flexADC);
  float flexV = flexADC * VCC / 4095.0;
  float flexR = (R_DIV * (VCC / flexV - 1.0))-80000;
  //Serial.println("Resistance: " + String(flexR) + " ohms");
 

  // Use the calculated resistance to estimate the sensor's
  // bend angle:
  float angle = map(flexR, STRAIGHT_RESISTANCE, BEND_RESISTANCE,
                   0, 90.0);
  angle = angle - 57;

  return (float) angle;
}


void _flex_instant() {
  ulong now = millis();
  static ulong ts = now;
  static bool waitToCheck = false;

  if(waitToCheck && now - ts > config_flex_instant_reset) {

    waitToCheck = false;
    ts = now;
  }

  // Check Every Second or so.
  if(now - ts > config_flex_instant_check) {
    float angle = readFlexAngle();
    if(angle > config_flex_limit) {
      handleNotify();

      waitToCheck = true;
    }

    ts = now;
  }
}

void _flex_average() {
  ulong now = millis();
  static ulong ts = now;
  static float average = 0.f;

  if(now - ts > config_flex_average_check) {
    float angle = readFlexAngle();
    average = average * (1.f - config_flex_average_weight) + angle * (config_flex_average_weight);

    if(average > config_flex_limit) {
        handleNotify();
        average = 0;
    }
    
    ts = now;
  }
}

int _flex_init()
{
   pinMode(pin_flex, INPUT);

   return 0;
}

void _flex_loop()
{
  switch(config_flex_mode) {
    case FLEX_INSTANT: _flex_instant(); break;
    case FLEX_AVERAGE: _flex_average(); break;
  }
}
