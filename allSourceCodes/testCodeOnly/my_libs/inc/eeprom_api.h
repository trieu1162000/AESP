/*
 * eeprom_api.h
 *
 *  Created on: Nov 26, 2023
 *      Author: trieu
 */

#ifndef MY_LIBS_INC_EEPROM_API_H_
#define MY_LIBS_INC_EEPROM_API_H_

#include "stdint.h"
#include "stdbool.h"
#include "driverlib/eeprom.h"
#include "queue_cards_api.h"

void loadCardsFromEEPROM(cardQueue *queue);
void saveCardsToEEPROM(const cardQueue *queue);
static void mergeAndPrint(const uint32_t *data, size_t size);
static void reverseBytes(uint32_t *value);


#endif /* MY_LIBS_INC_EEPROM_API_H_ */
