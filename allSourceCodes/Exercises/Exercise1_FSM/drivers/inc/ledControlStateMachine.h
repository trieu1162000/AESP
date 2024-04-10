/*
 * ledControlStateMachine.h
 *
 *  Created on: Apr 9, 2024
 *      Author: trieu
 */

#ifndef DRIVERS_INC_LEDCONTROLSTATEMACHINE_H_
#define DRIVERS_INC_LEDCONTROLSTATEMACHINE_H_

#include <stdint.h>

typedef enum
{
    S_LEDOFF = 0,
    S_LEDON,
} ledControlState_t;

extern uint32_t ledTimer;
extern enum ledValue led_val;

void ledControlStateMachineUpdate(void);

#endif /* DRIVERS_INC_LEDCONTROLSTATEMACHINE_H_ */
