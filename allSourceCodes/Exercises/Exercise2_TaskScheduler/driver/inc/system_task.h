/*
 * system_task.h
 *
 *  Created on: Jan 29, 2024
 *      Author: trieu
 */

#ifndef MY_LIBS_INC_SYSTEM_TASK_H_
#define MY_LIBS_INC_SYSTEM_TASK_H_

#include <FreeRTOS.h>
#include <semphr.h>
#include <stdbool.h>

// Minimum stack size for FreeRTOS tasks.
#define STACK_SIZE          (200)
#define LED_GREEN_TIME      ( (TickType_t) (15000U / portTICK_PERIOD_MS) )
#define LED_YELLOW_TIME     ( (TickType_t) (3000U / portTICK_PERIOD_MS) )
#define LED_RED_TIME        ( (TickType_t) (15000U / portTICK_PERIOD_MS) )

typedef enum
{
    NONE_SW_TIME = 0U,
    SW1_TIME = 1U,
    SW2_TIME = 2U,
} swTime_t;

extern volatile uint32_t period_led_time;

// Binary Semaphores
extern xSemaphoreHandle SW1PressedSemaphore_;
extern xSemaphoreHandle SW2PressedSemaphore_;

// Single-element Message Queue
extern xQueueHandle timeEventQueue_;

extern void SW1Task(void *);
extern void SW2Task(void *);
extern void ledTask(void *);

#endif /* MY_LIBS_INC_SYSTEM_TASK_H_ */
