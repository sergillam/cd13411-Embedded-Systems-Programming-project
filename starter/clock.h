#ifndef CLOCK_H
#define CLOCK_H

#include "alarm_tone.h"
#include "tm1637.h"
#include <Arduino.h>
#include <cstdint>

enum ClockState {
  STATE_CLOCK = 0,
  STATE_MENU_SET = 1,
  STATE_MENU_ALARM = 2,
  STATE_SET_CLOCK = 3,
  STATE_SET_ALARM = 4,
  STATE_ALARM_OFF = 5,
  STATE_ALARM = 6,
};

enum DigitState { DIGITS_LEFT = 0b10, DIGITS_RIGHT = 0b01, POINT = 0b100 };

enum ButtonType {
  BUTTON_MENU,
  BUTTON_PLUS,
  BUTTON_MINUS,
  BUTTON_OK,
};

class Clock {
private:
  TM1637 *display = NULL;
  hw_timer_t *timer = NULL;
  AlarmTone *alarm_tone;
  uint32_t time = 0;
  uint32_t alarm = 0;
  uint32_t *time_to_set = nullptr;
  uint32_t temp_time = 0;
  uint32_t timestamp = 0;
  uint8_t state = STATE_CLOCK;
  uint8_t set_digit = DIGITS_LEFT;
  bool alarm_enabled = 0;
  uint8_t blink_state = POINT;
  uint8_t display_state = DIGITS_LEFT | POINT | DIGITS_RIGHT;
  uint8_t alarm_off_counter;
  uint8_t alarm_counter;

public:
  Clock();
  void init(TM1637 *display, uint8_t buzzer_pin);
  void set_time(uint8_t hours, uint8_t minutes, uint8_t seconds);
  void set_alarm(uint8_t hours, uint8_t minutes);
  void check_alarm();
  void show();
  void run();
  void setup_timer();
  void update_time();
  void set_temp_time(int8_t offset);
  void commit_temp_time();
  void handleButtonMenuPress();
  void handleButtonOkPress();
  void handleButtonPlusPress();
  void handleButtonMinusPress();
  void handleSwitchAlarmChange(bool alarm_pin);
};

extern Clock clk;

#endif
