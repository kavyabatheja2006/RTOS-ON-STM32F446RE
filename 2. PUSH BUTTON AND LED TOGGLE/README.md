# Experiment 02 — GPIO Digital Input (Push Button LED Toggle)

> **Board:** Nucleo-F446RE &nbsp;|&nbsp; **MCU:** STM32F446RET6 &nbsp;|&nbsp; **Toolchain:** STM32CubeIDE

---

## 🎯 Aim

Interface a push button as digital input and demonstrate LED control by toggling its state on each valid button press.

---

## 📋 Concepts Covered

| Concept | Description |
|---------|-------------|
| GPIO Input | Configuring a pin as digital input with pull-up |
| GPIO Output | Driving an LED from a GPIO pin |
| Debouncing | Filtering mechanical switch bounce in software |
| Edge Detection | Detecting falling edge of button press |

---

## 🔌 Hardware Configuration

| Pin | Role | Notes |
|-----|------|-------|
| PC13 | GPIO Input | User push button (active LOW — pulled HIGH internally) |
| PA5 | GPIO Output | Onboard LED LD2 |

> ℹ️ **Note:** The Nucleo-F446RE user button (B1) is connected to **PC13** and is active LOW. The pin goes LOW when the button is pressed.

---

## ⚙️ CubeMX Configuration

- **PC13** → `GPIO_Input`, Pull-up enabled
- **PA5** → `GPIO_Output`, Push-Pull, No Pull
- **System Clock:** Default HSI or configured PLL

---

## 💻 Key Code

```c
/* USER CODE BEGIN WHILE */
while (1)
{
    // Wait for button press (PC13 goes LOW when pressed)
    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET)
    {
        HAL_Delay(50);  // Debounce delay

        // Confirm button still pressed after debounce
        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET)
        {
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);  // Toggle LED

            // Wait for button release before next toggle
            while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET);
        }
    }
}
/* USER CODE END WHILE */
```

---

## 📊 Expected Output

- LED **LD2 (PA5)** toggles ON↔OFF on every button press
- LED state is held until the next valid press
- No false toggles due to debounce handling

---

## 🔍 Debounce Explanation

```
Button Press (Mechanical bounce):
─────┐  ┌──┐ ┌────────────
     └──┘  └─┘
           ↑ Bounce

With 50 ms debounce filter:
─────┐            ┌──────
     └────────────┘
         50 ms wait → stable LOW confirmed → toggle
```

| Debounce Delay | Effect |
|---------------|--------|
| 0 ms | False triggers from bounce |
| 20–50 ms | Reliable single toggle |
| > 200 ms | May miss fast presses |

---

## ⚠️ Important Notes

- The onboard B1 button on Nucleo-F446RE is **active LOW** — PC13 reads `0` when pressed
- Always wait for button **release** to prevent repeated toggling on a single press
- Debounce delay of **50 ms** is typical for tactile switches; may need tuning for different buttons
- For interrupt-driven approach, see **Experiment 08** (EXTI with Binary Semaphore)

---

## 📁 Project Structure

```
Exp02_PushButton_LED_Toggle/
├── Core/
│   ├── Inc/
│   │   └── main.h
│   └── Src/
│       └── main.c          ← Button read and LED toggle logic
├── Drivers/
│   └── STM32F4xx_HAL_Driver/
├── .ioc
└── README.md
```

---

## 🚀 How to Run

1. Import project into STM32CubeIDE
2. Build (`Ctrl + B`) and Flash (`F11`)
3. Press the **blue B1 button** on the Nucleo board
4. Observe LED LD2 toggling with each press

---

## 🔗 References

- [STM32F446RE Datasheet](https://www.st.com/resource/en/datasheet/stm32f446re.pdf)
- [Nucleo-F446RE User Manual UM1724](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
