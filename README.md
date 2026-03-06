
# RTOS-ON-STM32F446RE

FreeRTOS projects for STM32F446RET6 using STM32CubeIDE

---

## 🧠 Hardware

| Item | Details |
|---|---|
| MCU | STM32F446RET6 |
| Core | ARM Cortex-M4 @ 180 MHz |
| Flash | 512 KB |
| RAM | 128 KB |
| RTOS | FreeRTOS (via STM32CubeIDE middleware) |
| Toolchain | STM32CubeIDE + STM32CubeMX |

---

## 📁 Experiments

| # | Title | Concepts |
|---|-------|----------|
| 01 | GPIO Digital Output — LED Blink | GPIO Output, Software Delay |
| 02 | GPIO Digital Input — Push Button LED Toggle | GPIO Input, Debouncing |
| 03 | HC-SR04 Ultrasonic Sensor — Distance Classification | GPIO, USART, Sensor Interfacing |
| 04 | PWM LED Brightness Control | Timer, PWM, Duty Cycle |

---

## 🔬 Experiment Details

### 01 — GPIO Digital Output (LED Blink)
**AIM:** Configure a GPIO pin of STM32F446RE as digital output and verify LED blinking operation using software delay routines.

- Configured **PA5** as GPIO Output
- Used `HAL_Delay()` for software delay
- Onboard LED (LD2) blinks at a defined interval

---

### 02 — GPIO Digital Input (Push Button LED Toggle)
**AIM:** Interface a push button as digital input and demonstrate LED control by toggling its state on each valid button press.

- Configured **PC13** as GPIO Input (User Button)
- LED toggles state on each valid button press
- Includes basic debounce handling

---

### 03 — HC-SR04 Ultrasonic Sensor with USART
**AIM:** Interface an HC-SR04 ultrasonic sensor with STM32F446RE and classify distance ranges using visual indication through LEDs, with output on serial monitor via USART.

- Trigger pulse generated via GPIO Output
- Echo pulse duration measured using Timer input capture
- Distance calculated and classified into ranges:
  - 🟢 **Close** → Green LED ON
  - 🟡 **Medium** → Yellow LED ON
  - 🔴 **Far** → Red LED ON
- Distance values printed on serial monitor via **USART2 @ 115200 baud**

---

### 04 — PWM LED Brightness Control
**AIM:** Generate a PWM signal using a timer on STM32F446RE and control the brightness of an onboard LED by varying the duty cycle.

- Timer configured in **PWM mode**
- Duty cycle varied from 0% → 100% → 0% (breathing effect)
- Onboard LED brightness changes smoothly

---

## 🚀 Getting Started

### Prerequisites
- [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) v1.13+
- ST-Link V2 or onboard debugger
- STM32F446RET6 board (Nucleo-F446RE)

### Clone the Repo
```bash
git clone https://github.com/kavyabatheja2006/RTOS-ON-STM32F446RE.git
```

### Opening a Project in STM32CubeIDE
1. Open **STM32CubeIDE**
2. Go to `File → Import → General → Existing Projects into Workspace`
3. Browse to any experiment folder
4. Click **Finish**
5. Build: `Ctrl+B` → Flash & Debug: `F11`

---

## 📌 Pin Reference

| Pin | Function |
|-----|----------|
| PA5 | Onboard LED (LD2) |
| PC13 | User Push Button |
| PA2 | USART2 TX (Serial Monitor) |
| PA3 | USART2 RX |

---

## 📄 License
MIT License — free to use, modify, and share.

---

## 🙋 Author
**Kavya Batheja**
GitHub: [@kavyabatheja2006](https://github.com/kavyabatheja2006)
