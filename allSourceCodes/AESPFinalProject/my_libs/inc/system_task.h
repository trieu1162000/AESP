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
#include "../inc/states.h"
#include "../inc/events.h"
#include "../inc/actions_api.h"
#include "../inc/system_FSM_api.h"
#include "../inc/keypad_api.h"

// Minimum stack size for FreeRTOS tasks.
#define STACK_SIZE 200

typedef enum
{
    DISPLAY_NONE,
    DISPLAY_APPEND,
    DISPLAY_RESULT,
    DISPLAY_CLEAR,
} displayTaskValue_t;

typedef enum
{
    ALPHA_VALUE_TRUE,
    ALPHA_VALUE_FALSE,
} alphaValue_t;

extern uint32_t task_idle_count;

// Binary Semaphores
extern xSemaphoreHandle acceptEventSemaphore_;
extern xSemaphoreHandle dispatchEventSemaphore_;
extern xSemaphoreHandle buttonEventSemaphore_;

// Single-element Queue
extern xQueueHandle displayEventQueue_;

extern void buttonTask(void *);
extern void displayTask(void *);
extern void mainTask(void *);

extern bool is_alpha_character;
extern alphaValue_t alpha_character_val;
extern char pressed_key;
extern bool check_result;

#endif /* MY_LIBS_INC_SYSTEM_TASK_H_ */
