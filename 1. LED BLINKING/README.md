# Experiment 01 — GPIO Digital Output (LED Blink)

> **Board:** Nucleo-F446RE &nbsp;|&nbsp; **MCU:** STM32F446RET6 &nbsp;|&nbsp; **Toolchain:** STM32CubeIDE

---

## 🎯 Aim

Configure a GPIO pin of STM32F446RE as digital output and verify LED blinking operation using software delay routines.

---

## 📋 Concepts Covered

| Concept | Description |
|---------|-------------|
| GPIO Output | Configuring a pin as push-pull digital output |
| HAL Abstraction | Using STM32 HAL library functions |
| Software Delay | Blocking delay with `HAL_Delay()` |
| Clock Configuration | System clock setup via STM32CubeMX |

---

## 🔌 Hardware Configuration

| Pin | Role | Notes |
|-----|------|-------|
| PA5 | GPIO Output | Onboard LED LD2 (active HIGH) |

---

## ⚙️ CubeMX Configuration

- **PA5** → `GPIO_Output`, Push-Pull, No Pull, Speed: Low
- **System Clock:** HSI (16 MHz) or HSE → PLL → 180 MHz (optional for this experiment)
- **Timebase Source:** TIM6 (recommended, frees SysTick for FreeRTOS later)

---

## 💻 Key Code

```c
/* USER CODE BEGIN WHILE */
while (1)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);   // LED ON
    HAL_Delay(500);                                        // 500 ms delay
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);  // LED OFF
    HAL_Delay(500);                                        // 500 ms delay
}
/* USER CODE END WHILE */
```

> **Alternative:** `HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);` followed by `HAL_Delay(500);`

---

## 📊 Expected Output

- Onboard LED **LD2 (PA5)** blinks at **1 Hz** (500 ms ON, 500 ms OFF)
- LED state alternates continuously in a blocking loop

---

## 🔍 Observations

| Delay Value | Blink Frequency |
|------------|-----------------|
| 100 ms | 5 Hz |
| 500 ms | 1 Hz |
| 1000 ms | 0.5 Hz |

---

## ⚠️ Important Notes

- `HAL_Delay()` is a **blocking** function — the CPU does nothing else during the wait
- In later RTOS experiments, `vTaskDelay()` replaces `HAL_Delay()` to allow task switching during delays
- Avoid using `HAL_Delay()` inside ISRs — it relies on SysTick interrupts which may be blocked

---

## 📁 Project Structure

```
Exp01_GPIO_LED_Blink/
├── Core/
│   ├── Inc/
│   │   └── main.h
│   └── Src/
│       └── main.c          ← LED blink logic here
├── Drivers/
│   └── STM32F4xx_HAL_Driver/
├── .ioc                    ← CubeMX configuration file
└── README.md
```

---

## 🚀 How to Run

1. Open **STM32CubeIDE** → `File` → `Import` → `Existing Projects into Workspace`
2. Browse to this folder and click **Finish**
3. Build: `Ctrl + B`
4. Flash & Debug: `F11`
5. Observe LED LD2 blinking on the Nucleo board

---

## 🔗 References

- [STM32F446RE Datasheet](https://www.st.com/resource/en/datasheet/stm32f446re.pdf)
- [Nucleo-F446RE User Manual](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
- UM1725 — STM32 HAL and Low-Layer Drivers Reference Manual
