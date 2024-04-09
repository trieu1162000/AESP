/*
 * motionDetectorMachine.c
 *
 *  Created on: Apr 9, 2024
 *      Author: Trieu Huynh Pham Nhat
 */

#include "motionDetectorMachine.h"
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "switches.h"
#include "led.h"
#include "utils/uartstdio.h"
#include "debug.h"

uint8_t sensorTimer = 0U;
uint8_t mntSensorVal = 0U;
static motionDetectorState_t motionState = S_NOMOTION;

#ifdef DEBUG
static const char *stateName[4] = {
    "S_NOMOTION",
    "S_NOMOTION_WAIT",
    "S_MOTION",
    "S_MOTION_WAIT"};

#endif

uint8_t getMotionSensorValue(void)
{
    return mntSensorVal;
}

void motionDetectorStateMachineUpdate(void)
{
    switch (motionState)
    {
    case S_NOMOTION:
        if (getSensorState(1) == ON)
        {
            motionState = S_NOMOTION_WAIT;
            // Set the timer to 50 ms
            sensorTimer = 50;
            DBG("motionState = %s\n", stateName[motionState]);
        }
        break;
    case S_NOMOTION_WAIT:
        if (getSensorState(1) == OFF)
        {
            motionState = S_NOMOTION;
            DBG("motionState = %s\n", stateName[motionState]);
        }
        else
        {
            if (sensorTimer == 0U)
            {
                motionState = S_MOTION;
                DBG("motionState = %s\n", stateName[motionState]);
            }
        }
        break;
    case S_MOTION:
        if (getSensorState(1) == ON)
        {
            motionState = S_MOTION_WAIT;
            // Set the timer to 50 ms
            sensorTimer = 50;
            DBG("motionState = %s\n", stateName[motionState]);
        }
        break;
    case S_MOTION_WAIT:
        if (getSensorState(1) == OFF)
        {
            motionState = S_MOTION;
            DBG("motionState = %s\n", stateName[motionState]);
        }
        else
        {
            if (sensorTimer == 0U)
            {
                motionState = S_NOMOTION;
                DBG("motionState = %s\n", stateName[motionState]);
            }
        }
        break;
    }

    switch (motionState)
    {
    case S_NOMOTION:
    case S_NOMOTION_WAIT:
        // Set the motion value to 0
        mntSensorVal = 0U;
        break;
    case S_MOTION:
    case S_MOTION_WAIT:
        // Set the motion value to 1
        mntSensorVal = 1U;
    }
}
