
#ifndef INCLUDED__MAIN_H
#define INCLUDED__MAIN_H


class NotifyL293D {
  const int pin_en34pwm, pin_3A, pin_4A;
  int pwmChannel;
  int dir;
public:
  NotifyL293D(int pin_en34, int pin_3A_poke, int pin_4A_vib)
    : pin_en34pwm (pin_en34)
    , pin_3A (pin_3A_poke)
    , pin_4A (pin_4A_vib)
    , pwmChannel(0)
    , dir (0)
  {
    pinMode(pin_3A, OUTPUT);
    pinMode(pin_4A, OUTPUT);
    ledcSetup(pwmChannel, 500, 8);
    ledcAttachPin(pin_en34pwm, pwmChannel);

    Stop();
  }

  void Vibrate() {
    ledcWrite(pwmChannel, 255);
    digitalWrite(pin_3A, LOW);
    digitalWrite(pin_4A, HIGH);
    
  }

  void Poke() {
    ledcWrite(pwmChannel, 255);
    digitalWrite(pin_3A, HIGH);
    digitalWrite(pin_4A, LOW);
  }

  void Stop() {
    ledcWrite(pwmChannel, 0);
    digitalWrite(pin_3A, LOW);
    digitalWrite(pin_4A, LOW);
  }
};



enum notify_mode_e {
  NOTIFY_NONE, NOTIFY_VIBRATE, NOTIFY_POKE, NOTIFY_AUDIO
};

enum flex_mode_e {
  FLEX_NONE, FLEX_INSTANT, FLEX_AVERAGE,
};


// Configuration
extern notify_mode_e config_notify;

extern flex_mode_e config_flex_mode;
extern int config_flex_instant_check;
extern int config_flex_instant_reset;
extern int config_flex_limit;
extern int config_flex_average_check;
extern float config_flex_average_weight;

// Controls
extern NotifyL293D   control_notify;

// Bluetooth Module
int    _bt_init();
size_t _bt_avail();
size_t _bt_write(const void* buffer, size_t bytes);
size_t _bt_read (const void* buffer, size_t bytes);

// Command Module
int   _cm_init();


void handleCommand(const char *line);
bool getCommandSerial();

void handleNotify();
void handleVibrate(bool enable = false);
void handlePoke(bool enable = false);

void handlePosture();

// Flex Module
int  _flex_init();
void _flex_loop();

#endif
