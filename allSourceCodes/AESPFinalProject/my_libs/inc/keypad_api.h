/*
 * keypad_api.h
 *
 *  Created on: Jan 29, 2024
 *      Author: trieu
 */

#ifndef MY_LIBS_INC_KEYPAD_API_H_
#define MY_LIBS_INC_KEYPAD_API_H_

#include "config_peripherals_api.h"
#include "system_FSM_api.h"

#define KEYPAD_PORT_BASE    (GPIO_PORTB_BASE)

// Rows From Px0 to Px3, currently x is portB
#define KEYPAD_ROW_1ST      (GPIO_PIN_0)
#define KEYPAD_ROW_2ND      (GPIO_PIN_1)
#define KEYPAD_ROW_3RD      (GPIO_PIN_2)
#define KEYPAD_ROW_4TH      (GPIO_PIN_3)
#define KEYPAD_ROWS         (KEYPAD_ROW_1ST | KEYPAD_ROW_2ND | KEYPAD_ROW_3RD | KEYPAD_ROW_4TH)

// Rows From Px4 to Px7, currently x is portB
#define KEYPAD_COL_1ST      (GPIO_PIN_4)
#define KEYPAD_COL_2ND      (GPIO_PIN_5)
#define KEYPAD_COL_3RD      (GPIO_PIN_6)
#define KEYPAD_COL_4TH      (GPIO_PIN_7)
#define KEYPAD_COLS         (KEYPAD_COL_1ST | KEYPAD_COL_2ND | KEYPAD_COL_3RD | KEYPAD_COL_4TH)

#define KEYPAD_ROW_COUNT 4U ///< The number of rows on the keypad.
#define KEYPAD_COL_COUNT 4U ///< The number of columns on the keypad.

#define LED_RED GPIO_PIN_1
#define LED_BLUE GPIO_PIN_2
#define LED_GREEN GPIO_PIN_3

extern char getKeyOnKeypad(void);
extern void initKeypad(uint32_t);
void keypadIntHandler(void);

#endif /* MY_LIBS_INC_KEYPAD_API_H_ */
