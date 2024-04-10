/*
 * ledControlStateMachine.h
 *
 *  Created on: Apr 9, 2024
 *      Author: trieu
 */

#ifndef DRIVERS_INC_LEDCONTROLSTATEMACHINE_H_
#define DRIVERS_INC_LEDCONTROLSTATEMACHINE_H_

typedef enum
{
    S_LEDOFF = 0,
    S_LEDON,
} ledState_t;

extern enum ledValue led_val;

void ledControlStateMachineUpdate(void);

#endif /* DRIVERS_INC_LEDCONTROLSTATEMACHINE_H_ */
