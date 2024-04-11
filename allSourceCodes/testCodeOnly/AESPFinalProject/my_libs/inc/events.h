/*
 * events.h
 *
 *  Created on: Nov 17, 2023
 *      Author: Trieu Huynh Pham Nhat
 */

#ifndef MY_LIBS_INC_EVENTS_API_H_
#define MY_LIBS_INC_EVENTS_API_H_

typedef enum {
    E_NONE = 0,                 // Handle alpha char
    E_TYPED,                    // Other buttons are pressed 
    E_RESET,                    // Reset button is pressed
    E_CACULATE,                 // Caculate button is pressed
} systemEvents_t;

#endif /* MY_LIBS_INC_EVENTS_H_ */
