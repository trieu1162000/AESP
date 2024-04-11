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

// This is used sFSM
extern SemaphoreHandle_t acceptEventSemaphore_;
extern SemaphoreHandle_t dispatchEventSemaphore_;
extern SemaphoreHandle_t displayEventSemaphore_;
extern SemaphoreHandle_t buttonEventSemaphore_;

extern void buttonTask(void *);
extern void displayTask(void *);
extern void mainTask(void *);

extern bool is_alpha_character;
extern alphaValue_t alpha_character_val;
extern char pressed_key;
extern bool check_result;
extern displayTaskValue_t display_task_val;

#endif /* MY_LIBS_INC_SYSTEM_TASK_H_ */
