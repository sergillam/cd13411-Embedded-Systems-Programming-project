#include "clock.h"

// Hardware pins for buttons, alarm switch and buzzer pin
#define MENU_PIN   16
#define PLUS_PIN   4
#define MINUS_PIN  2
#define OK_PIN     0
#define ALARM_PIN  15
#define BUZZER_PIN 12

TM1637 display(5, 18);
Clock clk;

// ISRs for buttons
static void button_menu_pressed(void) {
  clk.handleButtonMenuPress();
}

static void button_ok_pressed(void) {
  clk.handleButtonOkPress();
}

static void button_plus_pressed(void) {
  clk.handleButtonPlusPress();
}

static void button_minus_pressed(void) {
  clk.handleButtonMinusPress();
}

static void alarm_status_changed(void) {
  clk.handleSwitchAlarmChange(digitalRead(ALARM_PIN));
}

void setup() {
  
  // Configure buttons as inputs with pull-up
  pinMode(MENU_PIN, INPUT_PULLUP);
  pinMode(PLUS_PIN, INPUT_PULLUP);
  pinMode(MINUS_PIN, INPUT_PULLUP);
  pinMode(OK_PIN, INPUT_PULLUP);
  pinMode(ALARM_PIN, INPUT_PULLUP);

  // Attach interrupt for the buttons
  attachInterrupt(digitalPinToInterrupt(MENU_PIN), button_menu_pressed, FALLING);
  attachInterrupt(digitalPinToInterrupt(OK_PIN), button_ok_pressed, FALLING);
  attachInterrupt(digitalPinToInterrupt(ALARM_PIN), alarm_status_changed, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PLUS_PIN), button_plus_pressed, FALLING);
  attachInterrupt(digitalPinToInterrupt(MINUS_PIN), button_minus_pressed, FALLING);

  display.init();
  display.set(BRIGHT_TYPICAL);
  
  // Clock class init
  clk.init(&display, BUZZER_PIN);
  clk.handleSwitchAlarmChange(digitalRead(ALARM_PIN));;
  clk.run();
}

void loop() {
  // Delay to help with simulation running
  delay(100);
}
