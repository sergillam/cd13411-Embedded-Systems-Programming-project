# Alarm clock starter files

Check the README file on the parent folder for instructions on how to set up this starter code in Wokwi.


# ESP32 Alarm Clock Project

This project implements a digital alarm clock using an ESP32 microcontroller, a 4-digit 7-segment display, a buzzer, four push buttons, and a slide switch. It is developed using the Arduino framework and designed to run in the Wokwi simulator.

## ✅ Features

- ⏰ Clock starts at 00:00 and ticks automatically.
- 🧠 Time and alarm can be configured via code or through button interaction.
- 🔘 Alarm can be enabled/disabled via slide switch.
- 🔊 Alarm triggers when the current time matches the alarm time.
- ✅ Alarm can be silenced manually (OK button) or automatically (after 30 seconds).
- 🔁 Feedback on the display with labels: `SET`, `AL`, and `OFF`.
- 💡 Blinking colon every second, digit blinking while editing.

---

## 🛠️ Hardware Components (Simulated in Wokwi)

- ESP32
- 4-digit TM1637 7-segment display
- Buzzer
- 4 push buttons:
  - `MENU`: cycles between modes (`SHOW_TIME` → `SET_TIME` → `SET_ALARM`)
  - `+`, `-`: increment/decrement hour or minute
  - `OK`: confirms edits
- Slide switch: enables/disables the alarm

---

## 🎮 How to Use

### 🕒 Set the Time
1. Press `MENU` once to enter **SET** mode.
2. Press `OK` to start editing hours (hours blink).
3. Use `+` / `-` to adjust the hour.
4. Press `OK` to move to minutes (minutes blink).
5. Use `+` / `-` to adjust the minute.
6. Press `OK` to confirm. Clock resumes with the new time.

### ⏰ Set the Alarm
1. Press `MENU` twice to enter **AL** mode.
2. If the alarm is disabled (switch off), the display shows `OFF`.
3. If enabled:
   - Press `OK` to start editing alarm hour.
   - Use `+` / `-` to adjust the hour.
   - Press `OK` to move to minutes.
   - Use `+` / `-` to adjust the minute.
   - Press `OK` to confirm.

### 🔔 Alarm Behavior
- Alarm triggers when the time matches the configured alarm (hour and minute only).
- The display will blink, and the buzzer will sound.
- Alarm can be stopped:
  - By pressing the `OK` button.
  - Or automatically after 30 seconds.
- If the slide switch is OFF, the alarm will not trigger.

---

## 🧪 Testing Suggestions (in `setup()`)
Uncomment the following lines to test alarm functionality:

```cpp
clk.set_time(23, 59, 55);
clk.set_alarm(0, 0);
