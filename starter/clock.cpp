
#include "clock.h"
#include "stdio.h"
#include "esp_timer.h"

esp_timer_handle_t esp_timer;




// Static function: Update time, show things on display
//                  and check alarm trigger
// static void update_time(void* clock)
// {
// }

// Empty constructor
Clock::Clock() {}

// Clock::init(): Initialize internal variables,
//                set display to use and buzzer pin
void Clock::init(TM1637* disp, uint8_t buzzer_pin)
{
    display = disp;
    alarm_tone.init(buzzer_pin);
}

// Clock::set_time(): Set the time hour, minutes and seconds
//                    to internal binary representation
void Clock::set_time(uint8_t hour, uint8_t minutes, uint8_t seconds)
{
    current_time = ((hour & 0x1F) << 12) | ((minutes & 0x3F) << 6) | (seconds & 0x3F);
}

// Clock::set_alarm(): Set the alarm hour, minutes and seconds
//                     to internal binary representation
void Clock::set_alarm(uint8_t hour, uint8_t minutes)
{
    alarm_time = ((hour & 0x1F) << 12) | ((minutes & 0x3F) << 6); // segundos = 0
}

// Clock::menu_pressed(): Tell the internal clock that a button was pressed
//                        type: The button that was pressed
void Clock::button_pressed(ButtonType type)
{
    // Se o alarme estiver tocando e o botão OK for pressionado
    if (menu_state == ALARM_ACTIVE && type == BUTTON_OK) {
        alarm_tone.stop();
        menu_state = SHOW_TIME;
        alarm_dismissed = true;  // ✅ MARCA como dispensado
        Serial.println("Alarm stopped manually");
        return;
    }


    if (showing_off) return;

    // ---- MENU control ----
    if (type == BUTTON_MENU) {
        if (menu_state == SHOW_TIME) {
            menu_state = SET_TIME;
        } else if (menu_state == SET_TIME) {
            menu_state = SET_ALARM;

            if (!alarm_enabled) {
                showing_off = true;
                off_display_time = millis();
            }
        } else {
            menu_state = SHOW_TIME;
        }
        return;
    }

    // ---- SET_ALARM control ----
    if (menu_state == SET_ALARM && alarm_enabled) {
        if (type == BUTTON_OK) {
            if (edit_state == NONE) {
                edit_state = EDIT_HOUR;
                edit_hour = (alarm_time >> 12) & 0x1F;
                edit_minute = (alarm_time >> 6) & 0x3F;
            }
            else if (edit_state == EDIT_HOUR) {
                edit_state = EDIT_MINUTE;
            }
            else if (edit_state == EDIT_MINUTE) {
                set_alarm(edit_hour, edit_minute);
                edit_state = NONE;
                menu_state = SHOW_TIME;
            }
        }

        else if (type == BUTTON_PLUS) {
            if (edit_state == EDIT_HOUR) {
                edit_hour = (edit_hour + 1) % 24;
            } else if (edit_state == EDIT_MINUTE) {
                edit_minute = (edit_minute + 1) % 60;
            }
        }

        else if (type == BUTTON_MINUS) {
            if (edit_state == EDIT_HOUR) {
                edit_hour = (edit_hour == 0) ? 23 : edit_hour - 1;
            } else if (edit_state == EDIT_MINUTE) {
                edit_minute = (edit_minute == 0) ? 59 : edit_minute - 1;
            }
        }

        return;
    }

    // ---- SET_TIME control ----
    if (menu_state == SET_TIME) {
        if (type == BUTTON_OK) {
            if (edit_state == NONE) {
                edit_state = EDIT_HOUR;
                edit_hour = (current_time >> 12) & 0x1F;
                edit_minute = (current_time >> 6) & 0x3F;
            }
            else if (edit_state == EDIT_HOUR) {
                edit_state = EDIT_MINUTE;
            }
            else if (edit_state == EDIT_MINUTE) {
                set_time(edit_hour, edit_minute, 0);
                edit_state = NONE;
                menu_state = SHOW_TIME;
            }
        }

        else if (type == BUTTON_PLUS) {
            if (edit_state == EDIT_HOUR) {
                edit_hour = (edit_hour + 1) % 24;
            } else if (edit_state == EDIT_MINUTE) {
                edit_minute = (edit_minute + 1) % 60;
            }
        }

        else if (type == BUTTON_MINUS) {
            if (edit_state == EDIT_HOUR) {
                edit_hour = (edit_hour == 0) ? 23 : edit_hour - 1;
            } else if (edit_state == EDIT_MINUTE) {
                edit_minute = (edit_minute == 0) ? 59 : edit_minute - 1;
            }
        }

        return;
    }
}

// Clock::turn_alarm(): Enable or disable alarm
void Clock::turn_alarm(bool on_off)
{
    alarm_enabled = on_off;
    if (!alarm_enabled && menu_state == ALARM_ACTIVE) {
        alarm_tone.stop();
        menu_state = SHOW_TIME;
    }
}

// Clock::show(): Show time or menu on display
void Clock::show()
{
    if (showing_off) {
        display->point(false);
        display->displayStr("OFF");

        // Mostrar "OFF" por 2 segundos e voltar para o modo normal
        if (millis() - off_display_time >= 2000) {
            showing_off = false;
            menu_state = SHOW_TIME;
        }
        return;
    }

  if (menu_state == ALARM_ACTIVE) {
    alarm_tone.play();

    unsigned long now = millis();
    if (now - last_blink_time >= 500) {
        display_visible = !display_visible;
        last_blink_time = now;
    }

    if (display_visible) {
        uint8_t hour = (current_time >> 12) & 0x1F;
        uint8_t minute = (current_time >> 6) & 0x3F;
        char time_str[5];
        snprintf(time_str, sizeof(time_str), "%02d%02d", hour, minute);
        display->displayStr(time_str);
    } else {
        display->displayStr("   ");
    }

    display->point(false);

    // Nova lógica com contador
    if (alarm_counter > 0) {
        alarm_counter--;
    }

    if (alarm_counter == 0) {
        alarm_tone.stop();
        menu_state = SHOW_TIME;
        alarm_dismissed = true;  // ✅ Impede reativação imediata
        Serial.println("Alarm stopped automatically after 30s");
    }
    return;
  }

    if (menu_state == SET_TIME && edit_state == NONE) {
        display->point(false);
        display->displayStr("SET");
        return;
    }

    // Mostrar "AL" quando entra em modo SET_ALARM e o alarme está ativado
    if (menu_state == SET_ALARM && edit_state == NONE && alarm_enabled && !showing_off) {
        display->point(false);
        display->displayStr("AL");
        return;
    }

    if (menu_state == SHOW_TIME) {
        uint8_t hour = (current_time >> 12) & 0x1F;
        uint8_t minute = (current_time >> 6) & 0x3F;

        display->point(blink_colon);
        char time_str[5];
        snprintf(time_str, sizeof(time_str), "%02d%02d", hour, minute);
        display->displayStr(time_str);
        return;
    }

    if (menu_state == SET_TIME) {
      char time_str[5];
      snprintf(time_str, sizeof(time_str), "%02d%02d", edit_hour, edit_minute);

      display->point(true);

      if (edit_state == EDIT_HOUR && blink_colon) {
          time_str[0] = ' ';
          time_str[1] = ' ';
      }
      else if (edit_state == EDIT_MINUTE && blink_colon) {
          time_str[2] = ' ';
          time_str[3] = ' ';
      }

      display->displayStr(time_str);
      return;
    }
    if (menu_state == SET_ALARM && alarm_enabled) {
    char time_str[5];
    snprintf(time_str, sizeof(time_str), "%02d%02d", edit_hour, edit_minute);

    display->point(true);

    if (edit_state == EDIT_HOUR && blink_colon) {
        time_str[0] = ' ';
        time_str[1] = ' ';
    }
    else if (edit_state == EDIT_MINUTE && blink_colon) {
        time_str[2] = ' ';
        time_str[3] = ' ';
    }

    display->displayStr(time_str);
    return;
  }
}


// Clock::check_alarm(): Check if alarm needs to be triggered
void Clock::check_alarm()
{
  uint16_t now = (current_time >> 6);   // HH:MM
  uint16_t alarm = (alarm_time >> 6);   // HH:MM

  if (now != alarm) {
    alarm_dismissed = false;
  }
  if (!alarm_enabled || menu_state == ALARM_ACTIVE){
    return;
  }

    if (now == alarm && !alarm_dismissed) {
        menu_state = ALARM_ACTIVE;
        alarm_counter = 60; // 30 segundos
        alarm_tone.play();
    }
}

// Clock::run(): Start running the clock
//               This function MUST not block, everything should be handled
//               by interrupts
void Clock::run()
{
    const esp_timer_create_args_t timer_args = {
        .callback = &update_time,
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "clock_timer"
    };

    esp_timer_create(&timer_args, &esp_timer);
    esp_timer_start_periodic(esp_timer, 500000); // 0.5s = 500000 µs
}

void IRAM_ATTR update_time(void* arg)
{
    Clock* clk = static_cast<Clock*>(arg);

    // Atualiza contadores
    clk->timer_counter++;

    // Piscar dois pontos (:) a cada 0.5s
    clk->blink_colon = !clk->blink_colon;

    // Só avança 1 segundo a cada 2 ciclos (1 segundo real)
    if (clk->timer_counter >= 2) {
        clk->timer_counter = 0;

        // Incrementa 1 segundo
        uint8_t hour = (clk->current_time >> 12) & 0x1F;
        uint8_t minute = (clk->current_time >> 6) & 0x3F;
        uint8_t second = clk->current_time & 0x3F;

        second++;
        if (second >= 60) {
            second = 0;
            minute++;
            if (minute >= 60) {
                minute = 0;
                hour = (hour + 1) % 24;
            }
        }

        clk->set_time(hour, minute, second);
    }

    // Atualiza display e checa alarme
    clk->show();
    clk->check_alarm();
}
