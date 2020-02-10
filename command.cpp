
#include "arduino.h"
#include "esp_log.h"

#include "core.h"
#include "_main.h"

static const char* TAG = "COMMAND";


const char * checkCommand(const char *command, const char *line)
{
  size_t len = strlen(command);
  if(strncmp(line, command, len)) {
    return NULL;
  }
  return line + len;
}

void handleVibrate(bool enable)
{
  ulong now = millis();

  static ulong ts = now;
  static bool enabled = false;

  if(enable) {
    ts = now;
    enabled = true;

    // Enable
    ESP_LOGI(TAG, "Start Vibrate");
    control_notify.Vibrate();
    
  } else if(enabled && now - ts > 1000) {
    enabled = false;

    // Disable
    ESP_LOGI(TAG, "End Vibrate");
    control_notify.Stop();
  }
}

void handlePoke(bool enable)
{
  ulong now = millis();

  static ulong ts = now;
  static bool enabled = false;

  if(enable) {
    ts = now;
    enabled = true;
    control_notify.Poke();

    // Enable
    ESP_LOGI(TAG, "Start Poke");
    
  } else if(enabled && now - ts > 1000) {
    enabled = false;

    // Disable
    ESP_LOGI(TAG, "End Poke");
    control_notify.Stop();
  }
}

void handleNotify()
{
  switch(config_notify) {
    case NOTIFY_NONE:
      return;
    case NOTIFY_VIBRATE:
      handleVibrate(true);
      return;
    case NOTIFY_POKE:
      handlePoke(true);
      return;
    case NOTIFY_AUDIO:
      //handleAudio(true);
      return;
  }
}

// handleCommandSet handles set commands
void handleCommandSet(const char *line)
{
  const char *rest;
  if(rest = checkCommand("notify ", line)) {
    if(0 == strcmp(rest, "none")) {
      config_notify = NOTIFY_NONE;
      return;
    }
    if(0 == strcmp(rest, "vibrate")) {
      config_notify = NOTIFY_VIBRATE;
      return;
    }
    if(0 == strcmp(rest, "poke")) {
      config_notify = NOTIFY_POKE;
      return;
    }
    if(0 == strcmp(rest, "audio")) {
      config_notify = NOTIFY_AUDIO;
      return;
    }
  }
  else if(rest = checkCommand("flex mode ", line)) {
    if(0 == strcmp(rest, "none")) {
      config_flex_mode = FLEX_NONE;
      return;
    }
    if(0 == strcmp(rest, "instant")) {
      config_flex_mode = FLEX_INSTANT;
      return;
    }
    if(0 == strcmp(rest, "average")) {
      config_flex_mode = FLEX_AVERAGE;
      return;
    }
  }
  else if(rest = checkCommand("flex limit ", line)) {
    int value = atoi(rest);
    if(value > 0 && value < 90) {
      config_flex_limit = value;
    }
  }
  else if(rest = checkCommand("flex instant reset ", line)) {
    int value = atoi(rest);
    if(value > 5000 && value < 30000) {
      config_flex_instant_reset = value;
    }
  }
  else if(rest = checkCommand("flex instant check ", line)) {
    int value = atoi(rest);
    if(value > 500 && value < 10000) {
      config_flex_instant_check = value;
    }
  }
  else if(rest = checkCommand("flex average check ", line)) {
    int value = atoi(rest);
    if(value > 500 && value < 10000) {
      config_flex_average_check = value;
    }
  }
  else if(rest = checkCommand("flex average weight ", line)) {
    int value = atof(rest);
    if(value > 0.05 && value < 0.2) {
      config_flex_average_weight = value;
    }
  }
  
  ESP_LOGE(TAG, "Unhandled Set Command \"%s\"", line);
}

// handleCommand handles local arduino text based commands.
void handleCommand(const char *line)
{
  const char *rest;
  if(rest = checkCommand("set ", line)) {
    handleCommandSet(rest);
    return;
  }
  if(rest = checkCommand("notify", line)) {
    handleNotify();
    return;
  }
  if(rest = checkCommand("vibrate", line)) {
    handleVibrate(true);
    return;
  }
  if(rest = checkCommand("poke", line)) {
    handlePoke(true);
    return;
  }
  if(rest = checkCommand("send ", line)) {
    // Send data over Bluetooth
    _bt_write(rest, strlen(rest) + 1);
    return;
  }
}

bool getCommandSerial()
{
  static int  idx = 0;
  static char buffer[256];

  int c;
  while((c = Serial.read()) != -1) {
    if(c == '\n') {
      buffer[idx] = '\0'; 
      Serial.print(">");
      Serial.println(buffer);

      // handle command
      handleCommand(buffer);
      
      idx = 0;
      return 1;
    }
    if(idx >= (lengthOf(buffer) - 1)) {
      Serial.println("{Error \"Buffer Full\"}");
      continue;
    }
    if(isprint(c)) {
      buffer[idx++] = c;
      continue;
    }
    else {
      
    }
  }
  return 0;
}
