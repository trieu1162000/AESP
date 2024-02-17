/*
 * queue_cards_api.c
 *
 *  Created on: Nov 26, 2023
 *      Author: trieu
 */

#include "../inc/queue_cards_api.h"

cardQueue cardQueueForEEPROM;
card cardNeedToDo;

void initCard(card *card)
{
    memset(&card->name, '\0', sizeof(card->name));
    memset(&card->id, 0, sizeof(card->id));
    memset(&card->uuid, 0, sizeof(card->uuid));

}

void initCardQueue(cardQueue *queue) {
    int i = 0;
    queue->front = 0;
    queue->rear = -1;
    queue->numCards = 0;

    for (; i < MAX_CARDS; i++) {
        // Set each element of the authorizedCards array to zero
        memset(&queue->authorizedCards[i].name, '\0', sizeof(queue->authorizedCards[i].name));
        memset(&queue->authorizedCards[i].id, 0, sizeof(queue->authorizedCards[i].id));
        memset(&queue->authorizedCards[i].uuid, 0, sizeof(queue->authorizedCards[i].uuid));

    }
}

// Enqueue a card into the queue
bool enqueueCard(cardQueue *queue, const char *name, uint32_t id, const uint32_t *uuid) {
    if (queue->numCards < MAX_CARDS) {
        queue->rear = (queue->rear + 1) % MAX_CARDS;
        card *newCard = &queue->authorizedCards[queue->rear];
        strncpy(newCard->name, name, sizeof(newCard->name) - 1);
        newCard->name[sizeof(newCard->name) - 1] = '\0';  // Ensure null-terminated string
        newCard->id = id;
        memcpy(newCard->uuid, uuid, sizeof(newCard->uuid));
        queue->numCards++;
        return true;  // Enqueue successful
    }
    else
        return false;  // Queue full
}

// Dequeue a card from the queue
bool dequeueCard(cardQueue *queue, card *dequeuedCard) {
    if (queue->numCards > 0) {
        *dequeuedCard = queue->authorizedCards[queue->front];
        queue->front = (queue->front + 1) % MAX_CARDS;
        initCard(&queue->authorizedCards[queue->numCards - 1]);
        queue->numCards--;
        return true;  // Dequeue successful
    }
    else
        return false;  // Queue empty
}

void printAllCards(const cardQueue *queue) {
    int i, j;
    if (queue->numCards == 0) {
        DBG("No cards in the queue.\n");
        return;
    }

    DBG("Printing all cards:\n");
    for (i = 0; i < queue->numCards; i++) {
        DBG("Card %d:\n", i + 1);
        DBG("Name: %s\n", queue->authorizedCards[i].name);
        DBG("ID: %d\n", queue->authorizedCards[i].id);
        DBG("UUID: ");
        for (j = 0; j < sizeof(queue->authorizedCards[i].uuid)/sizeof(uint32_t); j++) {
            UARTprintf("%02X ", queue->authorizedCards[i].uuid[j]);
        }
        DBG("\n\n");
    }
}

// Function to get a card from the queue based on UUID
card *getCardFromUUID(const cardQueue *queue, const uint32_t *targetUUID) {
    int i, j;

    for (i = 0; i < queue->numCards; i++) {
        int match = 1; // Assume a match by default

        for (j = 0; j < sizeof(queue->authorizedCards[i].uuid) / sizeof(uint32_t); j++) {
            // Compare each 32-bit element of UUID
            if (queue->authorizedCards[i].uuid[j] != targetUUID[j]) {
                match = 0; // UUID does not match
                break;
            }
        }

        // If match is still 1, it means UUID matches for this card
        if (match) {
            return &queue->authorizedCards[i];
        }
    }

    // Here, there is no match card
    return NULL;
}

// Function to get the UUIDs of authorized cards
void getAuthorizedCardsUUID(const cardQueue *queue, uint32_t (*uuidArray)[CARD_LENGTH]) {
    int i, j;

    for (i = 0; i < queue->numCards; i++) {
        for (j = 0; j < 5; j++) {
            uuidArray[i][j] = (uint8_t) queue->authorizedCards[i].uuid[j];
        }
    }
}

// Function to remove a card from the queue based on ID
bool removeCard(cardQueue *queue, uint32_t id) {
    int i, j;
    for (i = 0; i < queue->numCards; i++) {
        if (queue->authorizedCards[i].id == id) {
            // Found the card with the specified ID, remove it and shift other elements
            for (j = i; j < queue->numCards - 1; j++) {
                queue->authorizedCards[j] = queue->authorizedCards[j + 1];
            }
            initCard(&queue->authorizedCards[queue->numCards - 1]);
            queue->numCards--;
            queue->rear = (queue->rear - 1) % MAX_CARDS;
            return true;  // Card removed successfully
        }
    }
    return false;  // Card with the specified ID not found
}

// Function to update the fields of a card based on UUID, and replace name and id
bool updateCardBaseOnUUID(cardQueue *queue, const uint32_t *uuid, const char *newName, uint32_t newId) {
    int i;
    for (i = 0; i < queue->numCards; i++) {
        if (memcmp(queue->authorizedCards[i].uuid, uuid, sizeof(queue->authorizedCards[i].uuid)) == 0) {
            // Found the card with the specified UUID, update its fields
            strncpy(queue->authorizedCards[i].name, newName, sizeof(queue->authorizedCards[i].name) - 1);
            queue->authorizedCards[i].name[sizeof(queue->authorizedCards[i].name) - 1] = '\0';  // Ensure null-terminated string
            queue->authorizedCards[i].id = newId;
            DBG("Update succesfully\n");
            printAllCards(queue);
            return true;  // Card updated successfully
        }
    }
    return false;  // Card with the specified UUID not found
}

bool checkCardIsDuplicated(cardQueue *queue, uint32_t *uuid)
{
    card *cardToCheck = NULL;
    
    cardToCheck = getCardFromUUID(queue, uuid);
    if(cardToCheck)
        return true;
    return false;
}

