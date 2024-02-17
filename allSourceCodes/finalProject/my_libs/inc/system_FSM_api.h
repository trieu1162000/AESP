/*
 * system_FSM_api.h
 *
 *  Created on: Nov 17, 2023
 *      Author: Trieu Huynh Pham Nhat
 */

#ifndef MY_LIBS_INC_SYSTEM_FSM_API_H_
#define MY_LIBS_INC_SYSTEM_FSM_API_H_

#include "actions_api.h"
#include "events.h"
#include "states.h"
#include "timer_handler_api.h"
#include "uart_api.h"
#include "debug.h"

typedef struct{
    systemEvents_t event_;
    systemState_t state_;
} sFSM_t;

extern sFSM_t sFSM_;

// This is used by bFSM
extern SemaphoreHandle_t bTimerEventSemaphore_;
extern SemaphoreHandle_t acceptEventSemaphore_;
extern SemaphoreHandle_t dispatchEventSemaphore_;

// This is used by gFSM
extern SemaphoreHandle_t gTimerEventSemaphore_;
extern SemaphoreHandle_t buttonEventSemaphore_;
extern SemaphoreHandle_t gAcceptEventSemaphore_;
extern SemaphoreHandle_t displayEventSemaphore_;

// This is used by both
extern SemaphoreHandle_t switchGUIEventSemaphore_;
extern SemaphoreHandle_t switchBareEventSemaphore_;

#endif /* MY_LIBS_INC_SYSTEM_FSM_API_H_ */
