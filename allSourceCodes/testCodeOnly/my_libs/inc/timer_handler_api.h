/*
 * timer_handler_api.h
 *
 *  Created on: Nov 19, 2023
 *      Author: trieu
 */

#ifndef MY_LIBS_INC_TIMER_HANDLER_API_H_
#define MY_LIBS_INC_TIMER_HANDLER_API_H_

typedef enum{
    TIMER_NONE = 0,
    TIMER_WARNED,
    TIMER_UNLOCKED,
}timerRaised_t;

extern timerRaised_t tRaised_;

#endif /* MY_LIBS_INC_TIMER_HANDLER_API_H_ */
