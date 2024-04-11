/*
 * lcd_i2c_api.h
 *
 *  Created on: Oct 29, 2023
 *      Author: Trieu Huynh Pham Nhat
 */

#include "config_peripherals_api.h"
#include "system_FSM_api.h"

#define MAX_FRAME_LENGTH 60

extern uint8_t receivedFrameIndex;
extern uint8_t rawReceivedFrame[MAX_FRAME_LENGTH];
extern uint8_t mainFrame[MAX_FRAME_LENGTH];

extern void UARTStringPut(uint32_t, const char*);
