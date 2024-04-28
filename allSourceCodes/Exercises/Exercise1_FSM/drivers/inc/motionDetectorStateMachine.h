/*
 * motionDetectorStateMachine.h
 *
 *  Created on: Apr 9, 2024
 *      Author: Trieu Huynh Pham Nhat
 */

#ifndef MOTIONDETECTORSTATEMACHINE_H_
#define MOTIONDETECTORSTATEMACHINE_H_

#include <stdint.h>

typedef enum
{
    S_NOMOTION = 0,
    S_NOMOTION_WAIT,
    S_MOTION,
    S_MOTION_WAIT,
} motionDetectorState_t;

extern uint8_t sensorTimer;
extern uint8_t mntSensorVal;

extern uint8_t getMotionSensorValue(void);
extern void motionDetectorStateMachineUpdate(void);

#endif /* MOTIONDETECTORSTATEMACHINE_H_ */
