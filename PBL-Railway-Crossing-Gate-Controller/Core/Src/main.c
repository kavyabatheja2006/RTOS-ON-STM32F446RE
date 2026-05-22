/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Railway Crossing Gate Controller — Complete Implementation
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* ── Thread handles & attributes ─────────────────────────────────────────── */

/* Definitions for Gatecontrol */
osThreadId_t GatecontrolHandle;
const osThreadAttr_t Gatecontrol_attributes = {
  .name       = "Gatecontrol",
  .stack_size = 128 * 4,
  .priority   = (osPriority_t) osPriorityNormal,
};

/* Definitions for TrainDetect */
osThreadId_t TrainDetectHandle;
const osThreadAttr_t TrainDetect_attributes = {
  .name       = "TrainDetect",
  .stack_size = 128 * 4,
  .priority   = (osPriority_t) osPriorityHigh,
};

/* Definitions for Warninglight */
osThreadId_t WarninglightHandle;
const osThreadAttr_t Warninglight_attributes = {
  .name       = "Warninglight",
  .stack_size = 128 * 4,
  .priority   = (osPriority_t) osPriorityLow,
};

/* Definitions for alarmHorn */
osThreadId_t alarmHornHandle;
const osThreadAttr_t alarmHorn_attributes = {
  .name       = "alarmHorn",
  .stack_size = 128 * 4,
  .priority   = (osPriority_t) osPriorityLow,
};

/* ── Semaphore handles & attributes ──────────────────────────────────────── */

/*
 * FIX #1 — initialCount = 0 (fail-safe boot state).
 *   Starting with initialCount = 1 lets actuator tasks run immediately at
 *   power-on before any train is detected — unsafe. With 0 every actuator
 *   task blocks right away; the crossing stays safe until the sensor fires.
 *
 * FIX #2 — maxCount = 3 (one token per actuator task).
 *   Three tasks (GateControl, WarningLight, AlarmHorn) all block on the same
 *   semaphore. TrainDetect must release it 3 times to wake all three.
 *   maxCount = 1 would only ever unblock one task per train event.
 */

/* Definitions for xSemApproach */
osSemaphoreId_t xSemApproachHandle;
const osSemaphoreAttr_t xSemApproach_attributes = {
  .name = "xSemApproach"
};

/* Definitions for xSemDepart */
osSemaphoreId_t xSemDepartHandle;
const osSemaphoreAttr_t xSemDepart_attributes = {
  .name = "xSemDepart"
};

/* USER CODE BEGIN PV */
/*
 * FIX #4 — No extern re-declarations here.
 *   xSemApproachHandle and xSemDepartHandle are already defined as globals
 *   in this translation unit. Redundant extern declarations removed.
 */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void gatecontrol(void *argument);
void Traindetect(void *argument);
void warninglight(void *argument);
void alarmhorn(void *argument);

/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
 * @brief  Retargets printf() to the SWV ITM Data Console (port 0).
 *         Enables printf() output visible in STM32CubeIDE during debug.
 */
int _write(int file, char *ptr, int len)
{
  for (int i = 0; i < len; i++) {
    ITM_SendChar(*ptr++);
  }
  return len;
}

/* USER CODE END 0 */

/* ── main() ─────────────────────────────────────────────────────────────── */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* Reset of all peripherals, Initializes the Flash interface and Systick */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();

  /* USER CODE BEGIN 2 */
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* USER CODE END RTOS_MUTEX */

  /* ── Create semaphores ──────────────────────────────────────────────────
   *
   * osSemaphoreNew(maxCount, initialCount, attr)
   *
   *   maxCount    = 3  → holds one token per actuator task; TrainDetect
   *                       releases it 3× so all three tasks wake together.
   *   initialCount = 0 → semaphore starts empty → every actuator blocks
   *                       immediately (fail-safe: crossing safe at boot).
   */

  /* creation of xSemApproach */
  xSemApproachHandle = osSemaphoreNew(3, 0, &xSemApproach_attributes);

  /* creation of xSemDepart */
  xSemDepartHandle   = osSemaphoreNew(3, 0, &xSemDepart_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* USER CODE END RTOS_QUEUES */

  /* ── Create threads ─────────────────────────────────────────────────── */

  /* creation of Gatecontrol */
  GatecontrolHandle  = osThreadNew(gatecontrol, NULL, &Gatecontrol_attributes);

  /* creation of TrainDetect */
  TrainDetectHandle  = osThreadNew(Traindetect, NULL, &TrainDetect_attributes);

  /* creation of Warninglight */
  WarninglightHandle = osThreadNew(warninglight, NULL, &Warninglight_attributes);

  /* creation of alarmHorn */
  alarmHornHandle    = osThreadNew(alarmhorn, NULL, &alarmHorn_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler — control never returns from here */
  osKernelStart();

  /* Infinite loop (unreachable) */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/* ── SystemClock_Config ──────────────────────────────────────────────────── */

/**
  * @brief  System Clock Configuration
  *         HSI → PLL → 180 MHz SYSCLK (STM32F446RE max)
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /* Configure the main internal regulator output voltage */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Initialise RCC Oscillators */
  RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM            = 8;
  RCC_OscInitStruct.PLL.PLLN            = 180;
  RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ            = 2;
  RCC_OscInitStruct.PLL.PLLR            = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /* Activate Over-Drive mode to reach 180 MHz */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /* Initialise CPU, AHB and APB bus clocks */
  RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK  | RCC_CLOCKTYPE_SYSCLK
                                   | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* ── MX_GPIO_Init ────────────────────────────────────────────────────────── */

/**
  * @brief  GPIO Initialization
  *
  *   PC13 — INPUT  : Train sensor (on-board user button, active LOW)
  *   PA5  — OUTPUT : Gate barrier indicator LED (HIGH = gate closed)
  *   PB5  — OUTPUT : Warning / crossing light LED
  *   PC0  — OUTPUT : Alarm buzzer (FIX #3 — was missing in original)
  *
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /*
   * FIX #3 — PC0 (buzzer) was never initialised in the original skeleton.
   *   alarmhorn() calls HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, ...) but
   *   without GPIO init that call is a no-op and the buzzer never sounds.
   *   PC0 is now configured as push-pull output below.
   */
  /* USER CODE END MX_GPIO_Init_1 */

  /* Enable GPIO port clocks */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* Set initial output levels — everything OFF at boot (fail-safe) */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);  /* Gate LED   : OFF (gate open)  */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);  /* Warning LED: OFF              */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);  /* Buzzer     : OFF (silent)     */

  /* PC13 — Train sensor input (on-board user button, active LOW) */
  GPIO_InitStruct.Pin  = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;  /* External pull-up on Nucleo board */
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* PA5 — Gate barrier indicator LED (active HIGH = gate closed) */
  GPIO_InitStruct.Pin   = GPIO_PIN_5;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* PB5 — Warning / crossing light LED */
  GPIO_InitStruct.Pin   = GPIO_PIN_5;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* PC0 — Alarm buzzer output (FIX #3) */
  GPIO_InitStruct.Pin   = GPIO_PIN_0;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

/* ── Task implementations ────────────────────────────────────────────────── */

/* USER CODE BEGIN Header_gatecontrol */
/**
  * @brief  GateControl task — controls the gate barrier (PA5).
  *
  *  Flow:
  *    1. Block on xSemApproach  (released by TrainDetect × 3 on approach)
  *    2. Drive PA5 HIGH         → gate CLOSES
  *    3. Block on xSemDepart    (released by TrainDetect × 3 on departure)
  *    4. Drive PA5 LOW          → gate OPENS
  *    5. Repeat forever
  *
  * @param  argument  Not used
  * @retval None
  */
/* USER CODE END Header_gatecontrol */
void gatecontrol(void *argument)
{
  /* USER CODE BEGIN 5 */
  for (;;)
  {
    /* ── Wait for train approaching ─────────────────────────────────────── */
    osSemaphoreAcquire(xSemApproachHandle, osWaitForever);

    /* Close the gate barrier */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
    printf("[GateControl] CLOSING gate barrier\n");
    osDelay(100);  /* Brief settle delay */

    /* ── Wait for train departed ────────────────────────────────────────── */
    osSemaphoreAcquire(xSemDepartHandle, osWaitForever);

    /* Open the gate barrier */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    printf("[GateControl] RAISING gate barrier\n");
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_Traindetect */
/**
  * @brief  TrainDetect task — highest-priority sensor polling task.
  *
  *  Polls PC13 (IR sensor, simulated by on-board user button) every 50 ms.
  *  Uses edge detection via trainPresent flag to fire only once per event.
  *
  *  Rising edge  (button pressed  = train approaching):
  *    → Release xSemApproach THREE times so GateControl, WarningLight,
  *      and AlarmHorn all unblock simultaneously.
  *      (FIX #2: single release only woke one task; all three need a token)
  *
  *  Falling edge (button released = train departed):
  *    → Release xSemDepart THREE times for the same reason.
  *
  * @param  argument  Not used
  * @retval None
  */
/* USER CODE END Header_Traindetect */
void Traindetect(void *argument)
{
  /* USER CODE BEGIN Traindetect */
  uint8_t trainPresent = 0;  /* Edge-detection flag: 0=no train, 1=train */

  for (;;)
  {
    /*
     * PC13 is active LOW:
     *   GPIO_PIN_RESET (0) = button pressed  = train detected
     *   GPIO_PIN_SET   (1) = button released = no train
     */
    uint8_t sensor = (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET);

    if (sensor && !trainPresent)
    {
      /* ── Rising edge: train just appeared ──────────────────────────── */
      trainPresent = 1;
      printf("[TrainDetect] Train APPROACHING — signalling all actuators\n");

      /*
       * FIX #2 — Release xSemApproach 3 times.
       * Each actuator task consumes exactly one token when it unblocks.
       * One release would only wake a single task; the other two would
       * remain blocked forever.
       */
      osSemaphoreRelease(xSemApproachHandle);  /* Token for GateControl  */
      osSemaphoreRelease(xSemApproachHandle);  /* Token for WarningLight */
      osSemaphoreRelease(xSemApproachHandle);  /* Token for AlarmHorn    */
    }
    else if (!sensor && trainPresent)
    {
      /* ── Falling edge: train has cleared the crossing ──────────────── */
      trainPresent = 0;
      printf("[TrainDetect] Train DEPARTED — signalling all actuators\n");

      /* Release xSemDepart 3 times — same reasoning as above */
      osSemaphoreRelease(xSemDepartHandle);    /* Token for GateControl  */
      osSemaphoreRelease(xSemDepartHandle);    /* Token for WarningLight */
      osSemaphoreRelease(xSemDepartHandle);    /* Token for AlarmHorn    */
    }

    /* Poll every 50 ms — provides debounce and yields CPU to other tasks */
    osDelay(50);
  }
  /* USER CODE END Traindetect */
}

/* USER CODE BEGIN Header_warninglight */
/**
  * @brief  WarningLight task — flashes the crossing warning LED (PB5).
  *
  *  Flow:
  *    1. Block on xSemApproach
  *    2. Flash PB5 every 500 ms by using a timed semaphore acquire:
  *         - Try xSemDepart with 500 ms timeout
  *         - osErrorTimeout → toggle LED, try again
  *         - osOK           → train departed; exit flash loop
  *    3. Extinguish PB5
  *    4. Repeat forever
  *
  *  This eliminates the need for a separate software timer task — the
  *  500 ms acquire timeout doubles as the LED toggle interval.
  *
  * @param  argument  Not used
  * @retval None
  */
/* USER CODE END Header_warninglight */
void warninglight(void *argument)
{
  /* USER CODE BEGIN warninglight */
  for (;;)
  {
    /* ── Wait for train approaching ─────────────────────────────────────── */
    osSemaphoreAcquire(xSemApproachHandle, osWaitForever);

    printf("[WarningLight] RED lights FLASHING\n");

    /*
     * Flash loop — toggle PB5 every 500 ms until the depart token arrives.
     * osSemaphoreAcquire returns:
     *   osOK           → token received → train gone → exit loop
     *   osErrorTimeout → no token yet   → toggle LED → loop again
     */
    while (osSemaphoreAcquire(xSemDepartHandle, 500) != osOK)
    {
      HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
    }

    /* Train departed — turn warning lights off */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
    printf("[WarningLight] Lights OFF — crossing clear\n");
  }
  /* USER CODE END warninglight */
}

/* USER CODE BEGIN Header_alarmhorn */
/**
  * @brief  AlarmHorn task — drives the alarm buzzer (PC0).
  *
  *  Flow:
  *    1. Block on xSemApproach
  *    2. Drive PC0 HIGH  → buzzer ON
  *    3. Block on xSemDepart (no timeout — buzzer must sound for the
  *       entire danger window regardless of duration)
  *    4. Drive PC0 LOW   → buzzer OFF
  *    5. Repeat forever
  *
  *  Note: PC0 requires GPIO initialisation (FIX #3) — without it,
  *        HAL_GPIO_WritePin is a no-op and the buzzer never activates.
  *
  * @param  argument  Not used
  * @retval None
  */
/* USER CODE END Header_alarmhorn */
void alarmhorn(void *argument)
{
  /* USER CODE BEGIN alarmhorn */
  for (;;)
  {
    /* ── Wait for train approaching ─────────────────────────────────────── */
    osSemaphoreAcquire(xSemApproachHandle, osWaitForever);

    /* Activate buzzer */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
    printf("[AlarmHorn] Alarm SOUNDING\n");

    /* ── Wait for train departed ────────────────────────────────────────── */
    osSemaphoreAcquire(xSemDepartHandle, osWaitForever);

    /* Silence buzzer */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
    printf("[AlarmHorn] Alarm SILENCED\n");
  }
  /* USER CODE END alarmhorn */
}

/* ── HAL callbacks & error handler ──────────────────────────────────────── */

/**
  * @brief  Period elapsed callback in non-blocking mode.
  *         Called from HAL_TIM_IRQHandler() when TIM6 fires.
  *         Increments HAL time base (uwTick) used by osDelay etc.
  * @param  htim  TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */
  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
  /* USER CODE END Callback 1 */
}

/**
  * @brief  Error handler — disables interrupts and halts the system.
  *         In a production safety system this would trigger a watchdog
  *         reset and drive all outputs to the safe (gate closed) state.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the source file and line number of a failed assert_param.
  * @param  file  Pointer to the source file name
  * @param  line  assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  printf("Assert failed: file %s, line %lu\r\n", file, (unsigned long)line);
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
