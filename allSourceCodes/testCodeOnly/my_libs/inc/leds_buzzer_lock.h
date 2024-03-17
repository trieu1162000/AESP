/*
 * leds_buzzer_lock.h
 *
 *  Created on: Nov 25, 2023
 *      Author: trieu
 */

#ifndef MY_LIBS_INC_LEDS_BUZZER_LOCK_H_
#define MY_LIBS_INC_LEDS_BUZZER_LOCK_H_

#include "config_peripherals_api.h"

enum ledNumber {
                LEDGREEN = 0,
                LEDRED
                };

enum controlState {
               OFF = 0,
               ON = 1
               };

extern void ledControl(enum ledNumber led, enum controlState state);
extern void buzzerControl(enum controlState state);
extern void lockControl(enum controlState state);

#endif /* MY_LIBS_INC_LEDS_BUZZER_LOCK_H_ */
