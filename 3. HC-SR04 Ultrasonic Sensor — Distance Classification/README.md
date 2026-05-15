# Experiment 03 — HC-SR04 Ultrasonic Sensor (Distance Classification)

> **Board:** Nucleo-F446RE &nbsp;|&nbsp; **MCU:** STM32F446RET6 &nbsp;|&nbsp; **Toolchain:** STM32CubeIDE

---

## 🎯 Aim

Interface an HC-SR04 ultrasonic sensor with STM32F446RE and classify distance ranges using visual indication through LEDs, with measurement output on a serial monitor via USART.

---

## 📋 Concepts Covered

| Concept | Description |
|---------|-------------|
| GPIO Output | Generating the trigger pulse |
| Timer Input Capture | Measuring echo pulse width |
| USART Communication | Transmitting distance values over serial |
| Sensor Interfacing | HC-SR04 timing protocol |
| Distance Classification | Conditional LED output based on range |

---

## 🔌 Hardware Configuration

| Pin | Role | Notes |
|-----|------|-------|
| PB6 (or custom) | Trigger Output | 10 µs HIGH pulse to HC-SR04 |
| PB7 (or custom) | Echo Input (Timer IC) | Measures echo pulse width |
| PA5 | Green LED | Close range indicator |
| PA6 | Yellow LED | Medium range indicator |
| PA7 | Red LED | Far range indicator |
| PA2 | USART2 TX | Serial output to PC |
| PA3 | USART2 RX | Serial input (optional) |

> ℹ️ Adjust Trigger/Echo pins to match your `.ioc` configuration.

---

## ⚙️ CubeMX Configuration

- **Timer (e.g. TIM1 or TIM2):** Input Capture mode on Echo pin
  - Prescaler: Set for 1 µs resolution (e.g., `PSC = 179` for 180 MHz clock → 1 MHz timer tick)
  - Auto-Reload: `0xFFFF`
- **USART2:** Asynchronous mode, Baud Rate: **115200**, 8N1
- **GPIO:** Trigger pin as output; LED pins as outputs

---

## 🔧 HC-SR04 Timing Protocol

```
Trigger Pin:
   ┌──────┐
───┘  10µs└────────────────────────────
                    ↓
Echo Pin:
────────────────────┐          ┌───────
                    └──────────┘
                    ←  t_echo  →
                    (proportional to distance)
```

**Distance Calculation:**
```
Distance (cm) = (Echo pulse width in µs) / 58
```
> Speed of sound ≈ 343 m/s → round trip: 58 µs per cm

---

## 💻 Key Code

```c
// Send 10 µs trigger pulse
HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET);
__HAL_TIM_SET_COUNTER(&htim1, 0);
while (__HAL_TIM_GET_COUNTER(&htim1) < 10);   // 10 µs wait
HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);

// Measure echo pulse width using timer input capture
// (use IC callbacks or polling based on setup)
uint32_t distance_cm = echo_pulse_us / 58;

// Classify and display
if (distance_cm < 10) {
    // Close — Green LED ON
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
} else if (distance_cm < 30) {
    // Medium — Yellow LED ON
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
} else {
    // Far — Red LED ON
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
}

// USART output
char msg[50];
sprintf(msg, "Distance: %lu cm\r\n", distance_cm);
HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
```

---

## 📊 Distance Classification

| Range | LED Colour | Meaning |
|-------|-----------|---------|
| < 10 cm | 🟢 Green | **Close** |
| 10 – 30 cm | 🟡 Yellow | **Medium** |
| > 30 cm | 🔴 Red | **Far** |

---

## 🖥️ Serial Monitor Output

Open any terminal (PuTTY / CoolTerm / Arduino Serial Monitor) at **115200 baud, 8N1**:

```
Distance: 7 cm
Distance: 8 cm
Distance: 15 cm
Distance: 42 cm
```

---

## ⚠️ Important Notes

- HC-SR04 operates at **5V logic** — use a voltage divider or logic-level shifter on the Echo line if connecting directly to STM32 (3.3V tolerant but **not 5V tolerant** on most pins)
- Ensure minimum **60 ms** between measurements to allow echo decay
- Timer prescaler must be configured for **1 µs resolution** for accurate distance calculation
- `sprintf()` requires `#include <stdio.h>` and USART handle properly initialized

---

## 📁 Project Structure

```
Exp03_HC-SR04_Ultrasonic/
├── Core/
│   ├── Inc/
│   │   └── main.h
│   └── Src/
│       └── main.c          ← Sensor trigger, echo capture, USART transmit
├── Drivers/
│   └── STM32F4xx_HAL_Driver/
├── .ioc
└── README.md
```

---

## 🔗 References

- [HC-SR04 Datasheet](https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf)
- [STM32 Timer Input Capture — AN4776](https://www.st.com/resource/en/application_note/an4776-general-purpose-timer-cookbook-for-stm32-microcontrollers-stmicroelectronics.pdf)
