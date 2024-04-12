/*
 * debug.h
 *
 *  Created on: Apr 9, 2024
 *      Author: Trieu Huynh Pham Nhat
 */

#ifndef MY_LIBS_INC_DEBUG_H_
#define MY_LIBS_INC_DEBUG_H_

#include <stdint.h>
#include "utils/uartstdio.h"

// #define DEBUG // Turn on when you need debugging mode
#undef DEBUG

#ifdef DEBUG
    #define DBG(fmt,...) UARTprintf("%s:  %s:  %d: "fmt, __FILE__, __FUNCTION__, __LINE__,##__VA_ARGS__)
#else
  #define DBG(fmt, args...)
#endif


#endif /* MY_LIBS_INC_DEBUG_H_ */
