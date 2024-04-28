/*
 * doorControlStateMachine.c
 *
 *  Created on: Apr 9, 2024
 *      Author: Trieu Huynh Pham Nhat
 */
#include "doorControlStateMachine.h"
#include "led.h"
#include "motionDetectorStateMachine.h"

uint32_t doorTimer = 0U;

static doorState_t doorState = S_CLOSE;

static void doorControl(enum doorValue val)
{
    // Blue LED
    ledControl(LEDBLUE, (enum ledValue) val);
}

void doorControlStateMachineUpdate(void)
{
    switch (doorState)
    {
    case S_CLOSE:
        if (getMotionSensorValue() == 1)
        {
            doorState = S_OPEN;
        }
        break;
    case S_OPEN:
        if (getMotionSensorValue() == 0)
        {
            doorState = S_OPEN_WAIT;
            // Set the timer to 10s
            doorTimer = 10000;
        }
        break;
    case S_OPEN_WAIT:
        if (getMotionSensorValue() == 1)
        {
            doorState = S_OPEN;
        }
        else
        {
            if (doorTimer == 0U)
            {
                doorState = S_CLOSE;
            }
        }
        break;
    }

    switch (doorState)
    {
    case S_CLOSE:
        // Close the door
        doorControl(CLOSE);
        break;
    case S_OPEN:
    case S_OPEN_WAIT:
        // Open the door
        doorControl(OPEN);
    }
}
