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
#define LED_GREEN_TIME      (15000U / portTICK_PERIOD_MS)
#define LED_YELLOW_TIME     (3000U / portTICK_PERIOD_MS)
#define LED_RED_TIME        (15000U / portTICK_PERIOD_MS)

extern volatile uint32_t period_led_time;

// Binary Semaphores
extern xSemaphoreHandle SW1PressedSemaphore_;
extern xSemaphoreHandle SW2PressedSemaphore_;

extern void SW1Task(void *);
extern void SW2Task(void *);
extern void ledTask(void *);

#endif /* MY_LIBS_INC_SYSTEM_TASK_H_ */
