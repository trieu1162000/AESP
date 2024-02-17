/*
 * actions_api.h
 *
 *  Created on: Nov 17, 2023
 *      Author: Trieu Huynh Pham Nhat
 */

#ifndef MY_LIBS_INC_ACTIONS_API_H_
#define MY_LIBS_INC_ACTIONS_API_H_

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "leds_buzzer_lock.h"
#include "rc522_api.h"
#include "uart_api.h"
#include "events.h"
#include "queue_cards_api.h"
#include "eeprom_api.h"
#include "system_FSM_api.h"
#include "lcd_i2c_api.h"

#define MAX_FRAME_SIZE        60
#define WRITE_NAME_BLOCK      1
#define WRITE_ID_BLOCK        2
#define MAX_LEN               16

#define VERIFY_PASS           1
#define VERIFY_FAIL           (-1)
#define YET_VERIFY            0

extern uint8_t passWd[6];
extern unsigned char str[MAX_LEN];
extern unsigned char cardUUID[CARD_LENGTH];
extern uint32_t cardUUID4Bytes[CARD_LENGTH];
extern char receivedFrame[MAX_FRAME_SIZE];
extern size_t receivedFrameLength;
extern uint8_t idBytes[4];

// This var is used for both
extern char functionalCode;
extern int8_t detectedFlag;
extern uint32_t authorizedCardUUIDs[MAX_CARDS][CARD_LENGTH];
extern card verifiedCard;

// These functions only be used in actions_api
static uint32_t combineBytes(uint8_t bytes[4]);
static void dumpHex(unsigned char *buffer, int len);
static void verifiedSending(card *myCard);
static void sync1Card(card *syncCard);
static int8_t writeID(uint8_t id);
static int8_t writeName(uint8_t *name);
static void sendLength(uint16_t length);

// Actions for base system. These will be used
extern bool parseFirstFrameInRawData(const uint8_t *data_stream, size_t stream_length);
extern void parseDataInFrame(char *frame, card *dataCard);
extern void warningDisplay(void);
extern void passDisplay(void);
extern void normalDisplay(void);
extern void bStartTimeOut(void);
extern void bNACKAction(void);
extern void bVerifyAction(void);
extern int8_t bPollingAction(void);
extern void bPassAction(void);
extern void bFailAction(void);
extern void bStopAction(void);
extern void bReceiveAction(void);
extern void bSyncAction(cardQueue *queue);
extern bool bWriteAction(card *writeCard);
extern bool bUpdateAction(card *updateCard);
extern bool bRemoveAction(uint32_t id);
extern void bACKRequestAction(void);
extern void bACKAdded(void);
extern void bACKUpdated(void);

#endif /* MY_LIBS_INC_ACTIONS_API_H_ */
