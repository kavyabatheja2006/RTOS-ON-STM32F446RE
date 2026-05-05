# 🔧 RTOS-ON-STM32F446RE

> Embedded systems experiments on **STM32F446RET6** using **FreeRTOS** and **STM32CubeIDE**  
> From bare-metal GPIO to multi-task RTOS scheduling — all in one place.

---

## 🛠️ Hardware & Tools

| Item | Details |
|------|---------|
| MCU | STM32F446RET6 |
| Core | ARM Cortex-M4 @ 180 MHz |
| Flash | 512 KB |
| RAM | 128 KB |
| RTOS | FreeRTOS (via STM32CubeIDE middleware) |
| Toolchain | STM32CubeIDE + STM32CubeMX |
| Board | Nucleo-F446RE |

---

## 📁 Experiments Overview

| # | Title | Concepts |
|---|-------|----------|
| 01 | GPIO Digital Output — LED Blink | GPIO Output, Software Delay |
| 02 | GPIO Digital Input — Push Button LED Toggle | GPIO Input, Debouncing |
| 03 | HC-SR04 Ultrasonic Sensor — Distance Classification | GPIO, Timer, USART, Sensor Interfacing |
| 04 | PWM LED Brightness Control | Timer, PWM, Duty Cycle |
| 05 | FreeRTOS — LED Blink via Single Task | FreeRTOS, Task Creation, vTaskDelay |
| 06 | FreeRTOS — Dual Task Priority Analysis | FreeRTOS, Task Priority, Preemption |
| 07 | FreeRTOS — Dual Task Priority with SWV ITM Tracing | FreeRTOS, CMSIS-V2, SWV ITM Console, Priority Scheduling |
| 08 | FreeRTOS — Binary Semaphore with EXTI Button Interrupt | FreeRTOS, Binary Semaphore, EXTI, Deferred Interrupt Processing |
| 09 | FreeRTOS — Inter-Task Communication via Queue | FreeRTOS, Queue, FIFO, Sensor Data Transfer |
| 10 | FreeRTOS — Counting Semaphore Resource Access Control | FreeRTOS, Counting Semaphore, Resource Sharing, Task Interleaving |

---

## 🔬 Experiment Details

---

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

### 05 — FreeRTOS LED Blink via Single Task
**AIM:** Develop a basic FreeRTOS-based project on STM32F446RE in STM32CubeIDE and validate LED blinking using a single RTOS task.

- FreeRTOS enabled via STM32CubeMX middleware
- Single task created using `osThreadNew()` / `xTaskCreate()`
- LED toggled inside task using `vTaskDelay()` instead of `HAL_Delay()`
- Validates basic RTOS task creation and scheduling

---

### 06 — FreeRTOS Dual Task Priority Analysis
**AIM:** Create and execute two FreeRTOS tasks with different priorities and analyze their effect on LED blinking behaviour.

- Two tasks created with **different priorities**
- Higher priority task preempts lower priority task
- LED blinking rates differ based on task priority
- Demonstrates FreeRTOS **preemptive scheduling** behaviour

---

### 07 — FreeRTOS Dual Task Priority with SWV ITM Tracing
**AIM:** Create and execute two FreeRTOS tasks with different priorities and analyze their effect on LED blinking behaviour using SWV ITM Data Console.

- Two tasks (`LED_1` and `LED_2`) created with **CMSIS-RTOS v2** interface (`CMSIS_V2`)
- Both tasks use `osDelay(500)` with **different priority levels** assigned via task attributes
- Priority levels tested: `osPriorityLow`, `osPriorityNormal`, `osPriorityHigh`, `osPriorityRealtime`
- `printf` retargeted to **SWV ITM Console** via `ITM_SendChar()` for real-time trace output
- Demonstrates **CPU starvation** of lower-priority tasks as priority difference increases
- LED blink rates and ITM console message frequency observed on **Port 0**

**Key Code Snippets:**
```c
// Retarget printf to ITM
int _write(int file, char *ptr, int len) {
    for (int i = 0; i < len; i++) {
        ITM_SendChar(*ptr++);
    }
    return len;
}

// Task 1 — toggles PA5
void Task1_function(void *argument) {
    for(;;) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        printf("Task_1 Executing for LED Toggle \n");
        osDelay(500);
    }
}

// Task 2 — toggles PA6
void StartLED_2(void *argument) {
    for(;;) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
        printf("Task_2 Executing for LED Toggle \n");
        osDelay(500);
    }
}
```

---

### 08 — FreeRTOS Binary Semaphore with EXTI Button Interrupt
**AIM:** Configure an external interrupt (EXTI) for a user button and use a binary semaphore to synchronize an LED control task.

- Implements **Deferred Interrupt Processing** — ISR stays short, heavy logic moved to a task
- **PC13** configured as EXTI with **Falling Edge Trigger Detection** (NVIC priority: 7)
- **PA5** configured as GPIO Output for LED
- Binary semaphore initialized to **0** (unavailable); released by ISR on button press
- `LED_Control` task blocks on `osSemaphoreAcquire()` and blinks LED **5 times** (250 ms ON/OFF) on each button press
- `printf` retargeted to **SWV ITM Console** for task execution trace

**Application Flow:**
```
Button Press → EXTI ISR → osSemaphoreRelease()
                              ↓
                    LED_Control task unblocks
                              ↓
                    LED blinks 5× (250 ms each)
                              ↓
                    Task blocks again (waits for next press)
```

**Key Code Snippets:**
```c
// LED_Control task — blinks LED 5 times per button press
void StartDefaultTask(void *argument) {
    uint8_t i;
    for(;;) {
        if (osSemaphoreAcquire(myBinarySem01Handle, 100) == osOK) {
            printf("Inside LEDControl Task\n");
            i = 0;
            while(i < 10) {
                HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
                HAL_Delay(250);
                i = i + 1;
            }
        }
    }
}

// EXTI ISR — releases semaphore on button press
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    osSemaphoreRelease(myBinarySem01Handle);
}
```

> ⚠️ **Note:** After every CubeMX code regeneration, manually change the binary semaphore initial count from `1` to `0`:
> ```c
> myBinarySem01Handle = osSemaphoreNew(1, 0, &myBinarySem01_attributes);
> ```

---

### 09 — FreeRTOS Inter-Task Communication via Queue
**AIM:** Implement inter-task communication using a FreeRTOS queue where one task generates or acquires sensor data and another task receives the data.

- FreeRTOS **Queue** used as a thread-safe **FIFO buffer** between tasks
- **Producer task** reads sensor data (e.g. ultrasonic HC-SR04) and sends to queue
- **Consumer task** receives data from queue and processes/displays it via SWV ITM Console
- Queue uses **Copy by Value** — data safely copied into queue memory, no shared variable risks
- **Blocking behaviour:**
  - Consumer blocks when queue is empty — zero CPU cycles consumed
  - Unblocks instantly when producer sends data
- Prevents **race conditions** — no two tasks access the same memory simultaneously
- Demonstrates the difference between semaphore (signal only) vs queue (signal + data)

**Key Concepts:**
```
Producer Task                    Queue (FIFO)             Consumer Task
─────────────                    ────────────             ─────────────
Read sensor data   ──Send──►  [ slot ][ slot ]  ──Receive──►  Process & print
osDelay()                                                   osDelay()
```

---

### 10 — FreeRTOS Counting Semaphore — Shared Resource Access Control
**AIM:** Model a limited shared resource using a FreeRTOS counting semaphore and study access control when multiple tasks request the resource simultaneously.

- **3 tasks** created: `TaskA`, `TaskB`, `TaskC` using CMSIS-RTOS V2
- **Counting semaphore** initialized with count = **2** (max 2 tasks can hold resource at once)
- Each task calls `osSemaphoreAcquire()` before accessing the ITM trace resource and `osSemaphoreRelease()` after
- `printf` retargeted to **SWV ITM Console** via `ITM_SendChar()`
- Demonstrates **task interleaving** — with count=2, two tasks print simultaneously causing garbled output
- Difference between **counting semaphore** (quantity control) vs **mutex** (mutual exclusion) clearly observed

**Key Concept:**
```
Counting Semaphore (count=2):
  TaskA acquires → count becomes 1
  TaskB acquires → count becomes 0
  TaskC tries    → BLOCKED (count = 0, waits)
  TaskA releases → count becomes 1 → TaskC unblocks
```

**Key Code Snippets:**
```c
// TaskA — acquires semaphore, prints 10 'A' chars, releases
void func_TaskA(void *argument) {
    char ch = 'A';
    for(;;) {
        osSemaphoreAcquire(myCountingSem01Handle, osWaitForever);
        printf("1");
        for(int i = 0; i < 10; i++) {
            printf("%c", ch);
            HAL_Delay(50);
        }
        osSemaphoreRelease(myCountingSem01Handle);
        osDelay(5);
    }
}
// TaskB and TaskC follow same pattern with 'B' and 'C'
```

> ⚠️ **Observation:** ITM output shows interleaved characters (e.g. `ACACAC`, `BABABAB`) because counting semaphore allows 2 tasks simultaneously — it controls **quantity** but NOT **atomicity** of the shared channel.

---

## 🚀 Getting Started

### Prerequisites
- [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) v1.13+
- ST-Link V2 or onboard debugger
- STM32F446RET6 Nucleo board

### Clone the Repo
```bash
git clone https://github.com/kavyabatheja2006/RTOS-ON-STM32F446RE.git
```

### Opening a Project in STM32CubeIDE
1. Open **STM32CubeIDE**
2. Go to `File` → `Import` → `General` → `Existing Projects into Workspace`
3. Browse to any experiment folder
4. Click **Finish**
5. Build: `Ctrl+B` → Flash & Debug: `F11`

---

## 📌 Pin Reference

| Pin | Function |
|-----|----------|
| PA5 | Onboard LED (LD2) |
| PA6 | External LED (Exp 07) |
| PC13 | User Push Button / EXTI Input |
| PA2 | USART2 TX (Serial Monitor) |
| PA3 | USART2 RX |

---

## 📄 License

MIT License — free to use, modify, and share.

---

## 🙋 Author

**Kavya Batheja**  
GitHub: [@kavyabatheja2006](https://github.com/kavyabatheja2006)
