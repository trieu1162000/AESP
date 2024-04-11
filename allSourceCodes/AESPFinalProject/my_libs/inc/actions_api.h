/*
 * actions_api.h
 *
 *  Created on: Nov 17, 2023
 *      Author: Trieu Huynh Pham Nhat
 */

#ifndef MY_LIBS_INC_ACTIONS_API_H_
#define MY_LIBS_INC_ACTIONS_API_H_

#include "lcd_i2c_api.h"
#include "system_task.h"

#define MAX_LENGTH_LCD          (16U)                   // LCD 16x2
#define MAX_LENGTH_DISPLAY_LCD  (17U)
#define MAX_LENGTH_SUPPORTED    (100U)                  // Curently support for 100 character in the buffer
#define MAX_ALPHA_BUFFER        (5U)
#define MAX_DOUBLE_STRING       (30U)
#define INVALID_VALUE           (MAX_OPERATORS + 1U)    // Unreach value

extern char alpha_display_buffer[MAX_ALPHA_BUFFER];
extern char display_buffer[MAX_LENGTH_DISPLAY_LCD];
extern char whole_chars_buffer[MAX_LENGTH_SUPPORTED];
extern uint8_t current_length_buffer;
extern int8_t current_pos_char_in_buffer;

// Display functions for displayTask
extern void appendDisplay(struct lcd_i2c *);
extern void resultDisplay(struct lcd_i2c *);
extern void clearDisplay(struct lcd_i2c *);

// Actions for mainTask
extern giveResultType_t giveResultAction(void);
extern void clearAction(void);
extern void appendAction(void);

#endif /* MY_LIBS_INC_ACTIONS_API_H_ */
