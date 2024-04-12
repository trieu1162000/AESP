/*
 * motionDetectorMachine.c
 *
 *  Created on: Apr 9, 2024
 *      Author: Trieu Huynh Pham Nhat
 */

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "switch.h"
#include "led.h"
#include "switch.h"
#include "motionDetectorStateMachine.h"

#define SENSOR_ON     PRESSED
#define SENSOR_OFF    RELEASED

uint8_t sensorTimer = 0U;
uint8_t mntSensorVal = 0U;

static motionDetectorState_t motionState = S_NOMOTION;

static sw_t getSensorState(void)
{
    // Return the state of SW1
    return switchState();
}


uint8_t getMotionSensorValue(void)
{
    return mntSensorVal;
}

void motionDetectorStateMachineUpdate(void)
{
    sw_t ret = getSensorState();

    switch (motionState)
    {
    case S_NOMOTION:
        if (ret == SENSOR_ON)
        {
            motionState = S_NOMOTION_WAIT;
            // Set the timer to 50 ms
            sensorTimer = 50;
        }
        break;
    case S_NOMOTION_WAIT:
        if (ret == SENSOR_OFF)
        {
            motionState = S_NOMOTION;
        }
        else
        {
            if (sensorTimer == 0U)
            {
                motionState = S_MOTION;
            }
        }
        break;
    case S_MOTION:
        if (ret == SENSOR_OFF)
        {
            motionState = S_MOTION_WAIT;
            // Set the timer to 50 ms
            sensorTimer = 50;
        }
        break;
    case S_MOTION_WAIT:
        if (ret == SENSOR_ON)
        {
            motionState = S_MOTION;
        }
        else
        {
            if (sensorTimer == 0U)
            {
                motionState = S_NOMOTION;
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
