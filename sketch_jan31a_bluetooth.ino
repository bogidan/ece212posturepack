
#include "esp_log.h"

#include "core.h"
#include "ringbuf.h"
#include "_main.h"

static const char* TAG = "MAIN";

//        ESP32 Adafruit Pin Layout
//           +------------------+
//           | RST              |
//           | 3V               |
//           | NC               |
//           | GND              |
//  A0    26 | A0/DAC2       BAT|
//  A1    25 | A1/DAC1        EN|
//  A2       | A2/34         USB|
//  A3       | A3/39      13/A12| 13 T4 A12
//  A4       | A4/36      12/A11| 12 T5 A11
//  A5 T0  4 | A5/4       27/A10| 27 T7 A10
//           | SCK/5       33/A9| 33 T8 A9
//           | MOSI/18     15/A8| 15 T3 A8
//           | MISO/19     32/A7| 32 T9 A7
//           | RX/16       14/A6| 14 T6 A6
//           | TX/17      SCL/22| 22
//           | 21         SDA/23| 23
//           +------------------+
const int pin_vibrate      = register_pin(32, "notify");
const int pin_poke         = register_pin(14, "notify");
const int pin_pwnEnable    = register_pin(15, "notify");
const int pin_scl          = register_pin(22, "imu");
const int pin_sda          = register_pin(23, "imu");
int register_pin(int pin, const char* module) {
  return pin;
}

// Config
notify_mode_e config_notify = NOTIFY_VIBRATE;


// Control
NotifyL293D control_notify(pin_pwnEnable, pin_poke, pin_vibrate);

void handlePosture()
{
  
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);
  // Setup Begin

  Serial.begin(115200);
  while(!Serial);

  Serial.println("Serial Started");
  
  _bt_init();
  _flex_init();

  digitalWrite(BUILTIN_LED, LOW);
}


void loop()
{
  getCommandSerial();
  
  handleVibrate();
  handlePoke();

  handlePosture();
  _flex_loop();
}

// NEEDED for CORE.h
void PrintContinue() {
  Serial.println();
}
