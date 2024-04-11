/*
 * config_peripherals_api.h
 *
 *  Created on: Oct 27, 2023
 *      Author: Trieu Huynh Pham Nhat
 */

#ifndef MY_LIBS_INC_CONFIG_PERIPHERALS_API_H_
#define MY_LIBS_INC_CONFIG_PERIPHERALS_API_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"

extern void initConsole(void);
extern void initPeriphs(void);
extern void initI2C(void);
extern void initHibernateMode(void);

#endif /* MY_LIBS_INC_CONFIG_PERIPHERALS_API_H_ */
