/*
 * states.h
 *
 *  Created on: Nov 17, 2023
 *      Author: Trieu Huynh Pham Nhat
 */

#ifndef MY_LIBS_INC_STATES_H_
#define MY_LIBS_INC_STATES_H_

typedef enum {
    S_STOPPED = 0U,
    S_CONVERTING,
    S_CACULATING,
} systemState_t;

#endif /* MY_LIBS_INC_STATES_H_ */
