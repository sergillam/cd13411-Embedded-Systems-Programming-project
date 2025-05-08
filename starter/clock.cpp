
#include "clock.h"
#include "stdio.h"

// Static function: Update time, show things on display
//                  and check alarm trigger
static void update_time(void *clock) {
  Clock *c = (Clock *)clock;
  c->time += 1;
  c->show();
  c->check_alarm();
}

// Empty constructor
Clock::Clock() : time(0), alarm_time(0), alarm_enabled(false), current_menu(MENU_TIME) {}

// Clock::init(): Initialize internal variables,
//                set display to use and buzzer pin
void Clock::init(TM1637* display, uint8_t buzzer_pin) {
    this->display = display;
    this->buzzer_pin = buzzer_pin;
    alarm_tone.init(buzzer_pin); 
    this->time = 0;  
    this->alarm_time = 0;  
    this->alarm_enabled = false;
    this->current_menu = MENU_TIME;
    this->display->set(BRIGHT_TYPICAL, 0x40, 0xc0); // Set display brightness and control
    this->display->clearDisplay(); // Clear display
    this->display->point(false); // Turn off the colon point
    this->display->set(0x00, 0x00, 0x00); // Set display to show 0
    this->display->clearDisplay(); // Clear display
    
    timerAttachInterrupt(timer, &update_time, true);
    timerAlarmWrite(timer, 500000, true);  // 500ms para atualização do tempo
    timerAlarmEnable(timer);
}

// Clock::set_time(): Set the time hour, minutes and seconds
//                    to internal binary representation
void Clock::set_time(uint8_t hour, uint8_t minutes, uint8_t seconds) {
    time = (hour << 11) | (minutes << 5) | seconds;
}

// Clock::set_alarm(): Set the alarm hour, minutes and seconds
//                     to internal binary representation
void Clock::set_alarm(uint8_t hour, uint8_t minutes) {
    alarm_time = (hour << 11) | (minutes << 5);
}


// Clock::menu_pressed(): Tell the internal clock that a button was pressed
//                        type: The button that was pressed
void Clock::button_pressed(ButtonType type) {
    switch (type) {
        case BUTTON_MENU:
            if (current_menu == MENU_TIME) {
                current_menu = MENU_ALARM;
                display->displayStr("AL", 500); 
            } else if (current_menu == MENU_ALARM) {
                current_menu = MENU_TIME;
                display->displayStr("TIME", 500);  
            }
            break;
        
        case BUTTON_PLUS:
            if (current_menu == MENU_TIME) {
                time += 1;  // Incrementa o tempo
            } else if (current_menu == MENU_ALARM) {
                alarm_time += 1;  
            }
            break;

        case BUTTON_MINUS:
            if (current_menu == MENU_TIME) {
                time -= 1;  
            } else if (current_menu == MENU_ALARM) {
                alarm_time -= 1;  
            }
            break;

        case BUTTON_OK:
            if (current_menu == MENU_TIME) {
                display->displayNum(time, 0, true);
            } else if (current_menu == MENU_ALARM) {
                display->displayStr("ALARM SET", 500);
                alarm_enabled = true;  
            }
            break;
    }
}


// Clock::turn_alarm(): Enable or disable alarm
void Clock::turn_alarm(bool on_off) {
    alarm_enabled = on_off;
    if (!alarm_enabled) {
        alarm_tone.stop();
    }
}

// Clock::show(): Show time or menu on display
void Clock::show() {
    if (current_menu == MENU_TIME) {
        uint8_t hours = (time >> 11) & 0x1F;
        uint8_t minutes = (time >> 5) & 0x3F;
        uint8_t seconds = time & 0x1F;
        display->displayNum(hours * 100 + minutes, 2, true);  
    } else if (current_menu == MENU_ALARM) {
        uint8_t alarm_hours = (alarm_time >> 11) & 0x1F;
        uint8_t alarm_minutes = (alarm_time >> 5) & 0x3F;
        display->displayNum(alarm_hours * 100 + alarm_minutes, 2, true); 
    }
}

// Clock::check_alarm(): Check if alarm needs to be triggered
void Clock::check_alarm() {
    if (alarm_enabled && time == alarm_time) {
        alarm_tone.play();
        display->displayStr("AL", 500); 
    }
}

// Clock::run(): Start running the clock
//               This function MUST not block, everything should be handled
//               by interrupts
void Clock::run() {
    check_alarm();
}
