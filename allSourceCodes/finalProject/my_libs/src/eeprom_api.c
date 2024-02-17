/*
 * eeprom_api.c
 *
 *  Created on: Nov 26, 2023
 *      Author: trieu
 */
#include "../inc/eeprom_api.h"
#include <stdlib.h>

char result[32] = {'\0'};
// Function to load cards from EEPROM into the queue
void loadCardsFromEEPROM(cardQueue *queue) {
    // EEPROMRead((uint32_t *)queue->authorizedCards, 0x400, sizeof(card) * MAX_CARDS);

    uint32_t rawName[8];
    uint32_t rawID;
    uint32_t rawUUID[5];


    uint32_t pui32Read[sizeof(card) * MAX_CARDS / sizeof(uint32_t)];
    // Read data from EEPROM into a buffer
//    uint32_t *pui32Read = (uint32_t *)malloc(sizeof(card) * MAX_CARDS );
    memset(pui32Read, 0, sizeof(card) * MAX_CARDS );

    EEPROMRead(pui32Read, 0x400, sizeof(card)*MAX_CARDS);

    // Initialize variables and counter
    int i, j;

    // Convert to 2D array
    for (i = 0; i < (sizeof(card) * MAX_CARDS / sizeof(uint32_t)); i = i+14) {

        // Handle NULL card
        // Check the first char of name is NULL or not
        if(!pui32Read[i])
            break;

        for(j = 0; j < 8; j++)
            rawName[j] = pui32Read[i+j];
        rawID = pui32Read[i+8];
        for(j = 0; j < 5; j++)
            rawUUID[j] = pui32Read[i+j+9];
        // Handle the string name
        mergeAndPrint(rawName, sizeof(rawName)/sizeof(uint32_t));
        enqueueCard(queue, result, rawID, rawUUID);
    }

}

// Function to save cards from the queue into EEPROM
void saveCardsToEEPROM(const cardQueue *queue) {
    EEPROMProgram((uint32_t *)queue->authorizedCards, 0x400, sizeof(card) * MAX_CARDS);
}

void reverseBytes(uint32_t *value) {
    *value = ((*value >> 24) & 0xFF) | ((*value >> 8) & 0xFF00) | ((*value << 8) & 0xFF0000) | ((*value << 24) & 0xFF000000);
}

void mergeAndPrint(const uint32_t *data, size_t size) {
    size_t resultIndex = 0;
    size_t i;
    int j;

    for (i = 0; i < size; i++) {
        uint32_t reversed = data[i];
        reverseBytes(&reversed);

        for (j = sizeof(uint32_t) - 1; j >= 0; j--) {
            // Extract each byte and convert it to a character
            result[resultIndex++] = (char)((reversed >> (j * 8)) & 0xFF);
        }
    }

    // Null-terminate the string
    result[resultIndex] = '\0';
//    DBG("Merged String: %s\n", result);

}


