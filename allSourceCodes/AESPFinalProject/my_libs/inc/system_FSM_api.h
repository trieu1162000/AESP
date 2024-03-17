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
#include "debug.h"

typedef struct{
    systemEvents_t event_;
    systemState_t state_;
} sFSM_t;

extern sFSM_t sFSM_;

#endif /* MY_LIBS_INC_SYSTEM_FSM_API_H_ */
