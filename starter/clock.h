#ifndef CLOCK_H
#define CLOCK_H

#include <cstdint>
#include <Arduino.h>
#include "tm1637.h"
#include "alarm_tone.h"

enum ButtonType
{
  BUTTON_MENU,
  BUTTON_PLUS,
  BUTTON_MINUS,
  BUTTON_OK,
};

class Clock
{
private:
    // 7-segment Display
    TM1637* display = NULL;

    // Timer variable to count time
    hw_timer_t *timer = NULL;

    // TODO: Add other private variables here
    unsigned long alarm_counter = 0;
    unsigned long off_display_time = 0;
    bool showing_off = false;
    unsigned long last_blink_time = 0;
    bool display_visible = true;

public:
    // Constructor
    Clock();

    // Init function
    void init(TM1637* display, uint8_t buzzer_pin);
    
    // Set time and alarm time
    void set_time(uint8_t hour, uint8_t minutes, uint8_t seconds);
    void set_alarm(uint8_t hour, uint8_t minutes);
    
    // Button pressed function
    void button_pressed(ButtonType type);
    
    // Alarm functions
    void turn_alarm(bool on_off);
    void check_alarm();

    // Clock functions 
    void show();
    void run();
    
    // TODO: Add other public variables/functions here
    // Binary time representation (HHHHH MMMMMM SSSSSS)
    uint32_t current_time = 0;
    uint32_t alarm_time = 0;
    bool alarm_enabled = false;
    // AlarmTone instance
    AlarmTone alarm_tone;

    // Estados de menu
    enum MenuState { SHOW_TIME, SET_TIME, SET_ALARM, ALARM_ACTIVE };
    MenuState menu_state = SHOW_TIME;

    // Estados de edição (se está editando hora ou minuto)
    enum EditState { NONE, EDIT_HOUR, EDIT_MINUTE };
    EditState edit_state = NONE;

    // Flags auxiliares
    bool blink_colon = true;
    bool blink_digit = true;
    uint8_t timer_counter = 0;
    uint8_t edit_hour = 0;
    uint8_t edit_minute = 0;
    bool alarm_dismissed = false;
};

void IRAM_ATTR update_time(void* arg);

#endif