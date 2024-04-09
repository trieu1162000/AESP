/*
 * doorControlStateMachine.h
 *
 *  Created on: Apr 9, 2024
 *      Author: Trieu Huynh Pham Nhat
 */

#ifndef DOORCONTROLSTATEMACHINE_H_
#define DOORCONTROLSTATEMACHINE_H_

#include <stdint.h>

typedef enum
{
    S_CLOSE = 0,
    S_OPEN,
    S_OPEN_WAIT,
} doorState_t;

extern uint32_t doorTimer;

void doorControlStateMachineUpdate(void);

#endif /* DOORCONTROLSTATEMACHINE_H_ */
