#include "clock.h"
#include "stdio.h"
#include <Arduino.h>

extern "C" void onTimer();

void ARDUINO_ISR_ATTR onTimer() {
  clk.update_time();
  clk.check_alarm();
  clk.show();
}

Clock::Clock() {}

void Clock::init(TM1637 *display, uint8_t buzzer_pin) {
  this->display = display;
  this->alarm_tone = new AlarmTone();
  alarm_tone->init(buzzer_pin);
}

void Clock::set_time(uint8_t hours, uint8_t minutes, uint8_t seconds) {
  time = hours << 12 | minutes << 6 | seconds;
  timestamp = (hours * 3600 + minutes * 60 + seconds) * 1000;
}

void Clock::set_alarm(uint8_t hours, uint8_t minutes) {
  alarm = hours << 12 | minutes << 6;
}

void Clock::set_temp_time(int8_t offset) {
  int8_t hours = temp_time >> 12;
  int8_t minutes = (temp_time >> 6) & 0x3F;
  int8_t seconds = temp_time & 0x3F;

  if (set_digit == DIGITS_LEFT)
    hours = (hours + 24 + offset) % 24;
  else
    minutes = (minutes + 60 + offset) % 60;

  temp_time = hours << 12 | minutes << 6 | seconds;
}

void Clock::commit_temp_time() {
  int8_t hours = temp_time >> 12;
  int8_t minutes = (temp_time >> 6) & 0x3F;
  int8_t seconds = temp_time & 0x3F;

  if (time_to_set == &time)
    set_time(hours, minutes, seconds);
  else
    set_alarm(hours, minutes);
}

void Clock::handleButtonMenuPress() {
  display_state = DIGITS_LEFT | POINT | DIGITS_RIGHT;
  switch (state) {
  case STATE_CLOCK:
  case STATE_MENU_SET:
  case STATE_MENU_ALARM:
    state = (state + 1) % 3;
    break;
  case STATE_SET_CLOCK:
  case STATE_SET_ALARM:
    state = STATE_CLOCK;
    set_digit = DIGITS_LEFT;
    break;
  }
}

void Clock::handleButtonOkPress() {
  display_state = DIGITS_LEFT | POINT | DIGITS_RIGHT;
  switch (state) {
  case STATE_MENU_SET:
    temp_time = time;
    time_to_set = &time;
    state = STATE_SET_CLOCK;
    break;
  case STATE_MENU_ALARM:
    temp_time = alarm;
    time_to_set = &alarm;
    if (alarm_enabled)
      state = STATE_SET_ALARM;
    else {
      state = STATE_ALARM_OFF;
      alarm_off_counter = 6;
    }
    break;
  case STATE_SET_CLOCK:
  case STATE_SET_ALARM:
    if (set_digit == DIGITS_RIGHT) {
      state = STATE_CLOCK;
      commit_temp_time();
    }
    set_digit ^= DIGITS_LEFT | DIGITS_RIGHT;
    break;
  case STATE_ALARM:
    state = STATE_CLOCK;
    break;
  }
}

void Clock::handleButtonPlusPress() { set_temp_time(+1); }

void Clock::handleButtonMinusPress() { set_temp_time(-1); }

void Clock::handleSwitchAlarmChange(bool alarm_pin) {
  alarm_enabled = alarm_pin;
}

void Clock::show() {
  uint32_t *time_on_display = nullptr;

  switch (state) {
  case STATE_CLOCK:
    blink_state = POINT;
    time_on_display = &time;
    break;
  case STATE_SET_CLOCK:
  case STATE_SET_ALARM:
    blink_state = set_digit;
    time_on_display = &temp_time;
    break;
  case STATE_ALARM:
    time_on_display = &alarm;
    blink_state = DIGITS_LEFT | POINT | DIGITS_RIGHT;
    alarm_tone->play();
    if (--alarm_counter == 0) {
      state = STATE_CLOCK;
      display_state = DIGITS_LEFT | POINT | DIGITS_RIGHT;
      blink_state = POINT;
    }
    break;
  }

  int8_t hours, minutes, seconds;
  int8_t data[4];
  display->point(0);

  switch (state) {
  case STATE_MENU_SET:
    display->displayStr("SET", 0);
    return;
  case STATE_MENU_ALARM:
    display->displayStr("AL", 0);
    return;
  case STATE_ALARM_OFF:
    display->displayStr("OFF", 0);
    if (--alarm_off_counter == 0)
      state = STATE_CLOCK;
    return;
  case STATE_CLOCK:
  case STATE_ALARM:
  case STATE_SET_CLOCK:
  case STATE_SET_ALARM:
    display_state ^= blink_state;
    hours = *time_on_display >> 12;
    minutes = (*time_on_display >> 6) & 0x3F;
    seconds = *time_on_display & 0x3F;

    if ((display_state & DIGITS_LEFT) >> 1) {
      data[0] = hours / 10;
      data[1] = hours % 10;
    } else {
      data[0] = data[1] = 0x7f;
    }

    display->point((display_state & POINT) >> 2);

    if (display_state & DIGITS_RIGHT) {
      data[2] = minutes / 10;
      data[3] = minutes % 10;
    } else {
      data[2] = data[3] = 0x7f;
    }

    display->display(data);
    break;
  }
}

void Clock::check_alarm() {
  if (alarm_enabled && time == alarm) {
    state = STATE_ALARM;
    alarm_counter = 60;
    display_state = DIGITS_LEFT | POINT | DIGITS_RIGHT;
  }
}

void Clock::setup_timer() {
  timer = timerBegin(0);
  timerAttachInterrupt(timer, onTimer);
  timerAlarm(timer, 500000, true, 0);
}

void Clock::update_time() {
  timestamp = (timestamp + 500) % (24 * 60 * 60 * 1000);
  uint8_t hour = timestamp / 3600000;
  uint8_t minutes = (timestamp % 3600000) / 60000;
  uint8_t seconds = (timestamp % 60000) / 1000;

  time = hour << 12 | minutes << 6 | seconds;
}

void Clock::run() {
  show();
  setup_timer();
}
