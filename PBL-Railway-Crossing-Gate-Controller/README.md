# 🚦 Railway Crossing Gate Controller
### Problem-Based Learning (PBL) — RTOS Application and Implementation

---


---

## 🔗 Project Links

| Deliverable | Link |
|---|---|
| **GitHub Repository** | [RTOS-ON-STM32F446RE](https://github.com/kavyabatheja2006/RTOS-ON-STM32F446RE) |


---

## 🚨 Problem Statement

Railway level crossings are one of the most accident-prone locations in the Indian railway network. Collisions occur due to delayed gate closure, missing warning signals, or absent alarm systems. The challenge is compounded when multiple actuators — the gate barrier, warning lights, and alarm horn — must respond **simultaneously and deterministically** the instant a train is detected.

A conventional bare-metal (sequential) approach cannot guarantee simultaneous response across all three actuators because tasks execute one at a time in a single thread. In a safety-critical system, any delay in lowering the gate or activating the alarm could be catastrophic.

This project implements a **Railway Crossing Gate Controller** on the **STM32F446RE Nucleo-64** microcontroller using **FreeRTOS (CMSIS-RTOS v2)**. Counting semaphores ensure that when a train is detected, all three actuator tasks — `GateControl`, `WarningLight`, and `AlarmHorn` — unblock and respond **concurrently and deterministically**, with a fail-safe default state at power-on.

### Why RTOS is Essential

| Requirement | RTOS Solution |
|---|---|
| Concurrent response | All three actuators activate within the same scheduler tick — impossible with a polling loop |
| Priority-based scheduling | `TrainDetect` runs at `osPriorityHigh`, pre-empting lower-priority actuators for fast detection |
| Fail-safe design | Semaphores initialized with `count = 0` — system cannot accidentally activate before train detection |
| Determinism | FreeRTOS guarantees bounded response times, critical in safety systems |

---

## 🧠 RTOS Concepts Applied

### 1. Task Management and Priority Scheduling

Four concurrent tasks are created using `osThreadNew()` and managed by the FreeRTOS preemptive scheduler. Each task has an independent 512-byte stack (`128 × 4`) and runs in an infinite loop.

| Task Name | Priority | CMSIS-RTOS | Behaviour |
|---|---|---|---|
| `TrainDetect` | HIGH | `osPriorityHigh` | Polls PC13 every 50 ms; edge detection; releases `xSemApproach × 3` on approach and `xSemDepart × 3` on departure |
| `GateControl` | NORMAL | `osPriorityNormal` | Acquires `xSemApproach` → PA5 HIGH (gate closes); acquires `xSemDepart` → PA5 LOW (gate opens) |
| `WarningLight` | LOW | `osPriorityLow` | Acquires `xSemApproach` → flashes PB5 every 500 ms using timed semaphore acquire; stops on `xSemDepart` |
| `AlarmHorn` | LOW | `osPriorityLow` | Acquires `xSemApproach` → PC0 HIGH (buzzer ON); acquires `xSemDepart` → PC0 LOW (buzzer OFF) |

### 2. Counting Semaphores for Multi-Task Synchronization

Two counting semaphores form the core synchronization mechanism:

| Semaphore | maxCount | initialCount | Purpose |
|---|---|---|---|
| `xSemApproach` | 3 | 0 (fail-safe) | Released 3× on train approach; each actuator task consumes one token to unblock simultaneously |
| `xSemDepart` | 3 | 0 (fail-safe) | Released 3× on train departure; unblocks all three actuators to deactivate |

> **Critical Design Insight:** `maxCount = 3` because three actuator tasks all block on the same semaphore. If `maxCount` were 1, only the first waiting task would receive a token; the remaining two would remain blocked forever, leaving the crossing partially unsafe.

**Semaphore Token Flow:**
```
BOOT:            xSemApproach = 0, xSemDepart = 0   → all tasks BLOCKED (fail-safe)
TRAIN DETECTED:  TrainDetect releases xSemApproach ×3 → GateControl, WarningLight, AlarmHorn UNBLOCK
DURING CROSSING: All 3 actuators active; tasks re-block on xSemDepart
TRAIN DEPARTED:  TrainDetect releases xSemDepart ×3  → All 3 actuators deactivate
```

### 3. Timed Semaphore Acquire — LED Flash Without a Timer Task

The `WarningLight` task achieves 500 ms LED flashing without any dedicated timer or `osDelay()` call. It uses a timed `osSemaphoreAcquire()`:
- **Timeout (`osErrorTimeout`)** → no depart signal yet — toggle LED and try again
- **Success (`osOK`)** → train departed — exit loop and extinguish light

This eliminates an extra task and demonstrates an elegant dual-purpose use of semaphore timeout as a timing mechanism.

### 4. Edge Detection for Reliable Sensor Events

`TrainDetect` uses a `trainPresent` flag to implement software edge detection. The sensor is polled every 50 ms (providing inherent debouncing), but semaphore releases occur only on state transitions — once when button is pressed (approach) and once when released (departure). This prevents runaway semaphore accumulation causing spurious actuator cycles.

---

## 🔧 Hardware Requirements

### Bill of Materials

| S.No | Component | Purpose | Qty |
|---|---|---|---|
| 1 | STM32F446RE Nucleo-64 Board | Microcontroller + debugger | 1 |
| 2 | IR Proximity Sensor Module | Train detection (simulated via user button PC13) | 1 |
| 3 | Red LED (5 mm) | Warning crossing light on PB5 | 1 |
| 4 | Green LED (5 mm) | Gate status indicator (on-board PA5) | 1 |
| 5 | Active Buzzer (5V) | Alarm horn on PC0 | 1 |
| 6 | NPN Transistor (BC547) | Buzzer driver — GPIO current limiting | 1 |
| 7 | Resistor 220 Ω | Current limiter for warning LED | 1 |
| 8 | Resistor 1 kΩ | Base resistor for transistor | 1 |
| 9 | Breadboard (half-size) | Circuit prototyping | 1 |
| 10 | Jumper wires | Connections | — |
| 11 | USB-A to Micro-B cable | Power + ST-LINK debug interface | 1 |

### GPIO Pin Assignments

| GPIO Pin | Direction | Component | Function |
|---|---|---|---|
| PC13 | INPUT | IR Sensor / User Button | Train detection (Active LOW, internal pull-up) |
| PA5 | OUTPUT | Gate Barrier LED (on-board LD2) | HIGH = gate CLOSED |
| PB5 | OUTPUT | Warning / Crossing Light LED | Flashing warning signal |
| PC0 | OUTPUT | Alarm Buzzer | Audible alarm via NPN transistor driver |

### Circuit Connection Details

**Warning LED (PB5):**
```
PB5 → 220 Ω resistor → LED anode → LED cathode → GND
```

**Alarm Buzzer (PC0):**
```
PC0 → 1 kΩ resistor → Base of BC547 NPN transistor
Collector of BC547 → Negative terminal of active buzzer
Positive terminal of buzzer → 5V (CN6 header)
Emitter of BC547 → GND (common with Nucleo board)
```

**Train Sensor (PC13):**
```
Simulated by on-board blue user button — no external wiring required.
External IR sensor: VCC → 3.3V, GND → GND, OUT → PC13
```

---

## 💻 Software Tools

| Tool | Purpose |
|---|---|
| STM32CubeIDE v1.14 | IDE, build toolchain (`arm-none-eabi-gcc`), ST-LINK flash utility |
| STM32CubeMX | Peripheral configuration and FreeRTOS middleware code generation |
| CMSIS-RTOS v2 / FreeRTOS v10.x | Real-time kernel (integrated into CubeIDE) |
| SWV ITM Data Console (Port 0) | Real-time `printf()` debug output via SWO pin — no UART required |
| HAL (Hardware Abstraction Layer) | GPIO, RCC, TIM6 (HAL timebase for `osDelay`) |

---

## 📁 Repository Structure

```
PBL-Railway-Crossing-Gate-Controller/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── FreeRTOSConfig.h
│   │   └── stm32f4xx_hal_conf.h
│   └── Src/
│       ├── main.c              ← Application entry point
│       ├── freertos.c          ← Task definitions and semaphore logic
│       └── stm32f4xx_it.c      ← Interrupt handlers
├── Drivers/
│   ├── CMSIS/                  ← ARM CMSIS headers
│   └── STM32F4xx_HAL_Driver/   ← ST HAL source and headers
├── Middlewares/
│   └── Third_Party/FreeRTOS/   ← FreeRTOS kernel source
├── ProjectTrain.ioc            ← STM32CubeMX configuration file
├── STM32F446RETX_FLASH.ld      ← Linker script (Flash)
└── README.md
```

---

## ✅ Key Results and Testing

### Verified Test Sequence

| # | Action | Observed Output |
|---|---|---|
| 1 | System power-on | All outputs OFF. All 3 actuator tasks immediately block on `xSemApproach` — **fail-safe state confirmed** |
| 2 | User button PRESSED (train approaching) | Gate CLOSES (PA5 HIGH), warning lights FLASH at 500 ms (PB5), alarm SOUNDS (PC0 HIGH) |
| 3 | Button held (train at crossing) | Gate stays closed, lights continue flashing, buzzer sounds continuously. No spurious re-triggers |
| 4 | User button RELEASED (train departed) | Gate OPENS (PA5 LOW), lights OFF, alarm SILENCED (PC0 LOW) |
| 5 | Cycle repeated | System returns to blocked state. Subsequent cycles produce **identical, deterministic behaviour** |

### Challenges Faced and Solutions

| Challenge | Root Cause | Solution |
|---|---|---|
| Only one of three actuator tasks woke on train detection | `maxCount = 1`; extra releases were silently dropped | Set `maxCount = 3`; release semaphore exactly 3 times per event |
| All actuators fired immediately at power-on | `initialCount = 1` gave tasks a token before any sensor event | Set `initialCount = 0` so all tasks block at boot |
| Buzzer never activated despite correct code | PC0 was never initialized as GPIO output | Added `GPIO_InitTypeDef` block for PC0 in `MX_GPIO_Init()` |
| Linker errors: duplicate semaphore handle definitions | Redundant `extern` declarations conflicting with global definitions | Removed `extern` re-declarations |
| WarningLight kept flashing after train departed | Flash loop had no mechanism to detect departure while waiting | Used timed semaphore acquire (500 ms) as combined toggle interval + exit signal |

---


---

## 🏁 Conclusion

This project successfully demonstrates a safety-critical embedded system implemented using **FreeRTOS on the STM32F446RE Nucleo-64** microcontroller. Four RTOS concepts were applied and verified in hardware:

1. **Task management** with priority-based preemptive scheduling
2. **Counting semaphores** for multi-task synchronization
3. **Timed semaphore acquisition** as a combined timing and synchronization mechanism
4. **Edge-detection logic** for reliable sensor event handling

The design patterns demonstrated here — particularly the **multi-token counting semaphore** and the **timed-acquire flash loop** — are directly applicable to production embedded systems in transportation, industrial automation, and safety-critical IoT applications.

---

