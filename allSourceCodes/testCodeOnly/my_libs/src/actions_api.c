/*
 * actions_api.c
 *
 *  Created on: Nov 17, 2023
 *      Author: trieu
 */

#include "../inc/actions_api.h"

int8_t detectedFlag = MI_NOTAGERR;
unsigned char str[MAX_LEN] = {0};
unsigned char cardUUID[CARD_LENGTH] = {0};
uint32_t cardUUID4Bytes[CARD_LENGTH] = {0};
unsigned char funnctionalCode = 0; 
uint32_t authorizedCardUUIDs[MAX_CARDS][CARD_LENGTH] = {0};
card verifiedCard;
char receivedFrame[MAX_FRAME_SIZE] = {0};
size_t receivedFrameLength = 0;
uint8_t idBytes[4] = {'\0'};
//uint8_t passWd[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t passWd[6] =  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

void bVerifyAction(void)
{
    // Read the UID of card
    int count, i, j;


    for(j = 0; j < CARD_LENGTH; j++)
        cardUUID4Bytes[j] = (uint32_t) cardUUID[j];

    getAuthorizedCardsUUID(&cardQueueForEEPROM, authorizedCardUUIDs);
    DBG("Verify ID: \n\r");
    dumpHex((unsigned char *)cardUUID, CARD_LENGTH);
    for (i = 0; i < cardQueueForEEPROM.numCards; ++i) {
        count = 0;
        for (j = 0; j < CARD_LENGTH; j++){
            if(cardUUID4Bytes[j] == authorizedCardUUIDs[i][j])
                count++;
        }
        if (count == 5) {
            verifiedCard = *getCardFromUUID(&cardQueueForEEPROM, cardUUID4Bytes);
            bFSM_.event_ = E_AUTHORIZED;
            xSemaphoreGive(dispatchEventSemaphore_);
            return;
        }
    }
    bFSM_.event_ = E_DENIED;
    xSemaphoreGive(dispatchEventSemaphore_);

}

int8_t bPollingAction(void)
{
    // initRC522();
    int8_t status;
    status = rc522Request(PICC_REQIDL, str);
    if(status == MI_OK){
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, HIGH_PIN);
        DBG("Card Detected! \r\n"); //Card Detected
    }

    status = rc522Anticoll(&str[2]);
    memcpy(cardUUID, &str[2], 5);

    if(status == MI_OK){
        DBG("ID: \n\r");
        dumpHex((unsigned char *)cardUUID, CARD_LENGTH);
        SysCtlDelay(SysCtlClockGet()/300); //Delay 10 ms
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, LOW_PIN);
    }
    return status;

}

void bPassAction(void)
{
    // Here, We will unlock the door immediately

    // Simultaneously, enable the unlocked timer 0 and wait for it to expire
    // At this time, it will point to the Timer Int Handler named timerUIntHandler
    // Disable the timer before enabling it again to reset the count
    TimerDisable(TIMER0_BASE, TIMER_A);
    HWREG(TIMER0_BASE+0x50) = 3*SysCtlClockGet() - 1;
    TimerEnable(TIMER0_BASE, TIMER_A);

    lockControl(ON); // ON->Lock

    // Subsequently, We may display something to the LCD
    passDisplay();

    // Then, we may put some data, means that raise an TX ISR
    verifiedSending(&verifiedCard);

    // We may also turn on LED Green, Buzzer but shorter seconds
    ledControl(LEDGREEN, ON);

}

void bFailAction(void)
{
    // Here, We will lock the door immediately
    lockControl(OFF); // ON->Lock

    // Simultaneously, enable the unlocked timer 1 and wait for it to expire
    // At this time, it will point to the Timer Int Handler named timerUIntHandler
    // Disable the timer before enabling it again to reset the count
    TimerDisable(TIMER1_BASE, TIMER_A);
    HWREG(TIMER1_BASE+0x50) = 3*SysCtlClockGet() - 1;
    TimerEnable(TIMER1_BASE, TIMER_A);

    // Then, we may put turn on LED Red, Buzzer for longer seconds
    ledControl(LEDRED, ON);
    buzzerControl(ON);

    // Subsequently, We may want to warning something to the LCD}
    warningDisplay();
}

void bStopAction(void)
{
    // Here, We will lock the door immediately
    lockControl(OFF);

    // Turn off all leds
    ledControl(LEDGREEN, OFF);
    ledControl(LEDRED, OFF);

    // Turn off buzzer
    buzzerControl(OFF);

    // Turn the timer to none
    tRaised_ = TIMER_NONE;

    // And, We may display something to the LCD
    normalDisplay();
}

void bReceiveAction(void)
{
    int i;
    bool result;
    size_t rawDataLength  = 0;

    DBG("Received Frame: ");
    for(i = 0; i<MAX_FRAME_LENGTH; i++)
        UARTprintf("%x ", mainFrame[i]);
    DBG("\n");

    rawDataLength = sizeof(mainFrame) / sizeof(mainFrame[0]);    // Get the data into the frame from the raw data
    result = parseFirstFrameInRawData((uint8_t *) mainFrame, rawDataLength);
    if(result)
    {
        // Parsing the data in receive frame into the card
        parseDataInFrame(receivedFrame, &cardNeedToDo);
    }
//    else
//    {
//        currentEvent = E_GUI_FINISHED; // Handle error when ISR_RECEIVE is still raise so then STOP state may polling forever
//    }

    // Need to reset the frame before receiving again
    memset(rawReceivedFrame, 0, MAX_FRAME_LENGTH);
    memset(mainFrame, 0, MAX_FRAME_LENGTH);
    receivedFrameIndex = 0;

}

void bSyncAction(cardQueue *queue)
{
    // Sync the database from Tiva C to the GUI
    int i;

    if (queue->numCards == 0) {
        DBG("No cards in the queue.\n");
        return;
    }

    sendLength(queue->numCards * 46);

    for (i = 0; i < queue->numCards; i++) {
        sync1Card(&queue->authorizedCards[i]);
    }

}

// This action is used for adding new card
bool bWriteAction(card *writeCard)
{
    int i;

    // We need to disable the timeout if a card already been detected
    TimerDisable(TIMER2_BASE, TIMER_A);

    for (i = 0; i < CARD_LENGTH; i++)
        writeCard->uuid[i] = (uint32_t) cardUUID[i];

    // Reset for next use
    memset(cardUUID, 0, CARD_LENGTH);

    // Write for existing card. These codes below is using to write for only a writeable card
    // We dont need them at this time. So skip here, uncomment them in case you need to use.
    // writeName((uint8_t *) cardNeedToDo.name);
    // writeID(cardNeedToDo.id);

    if(checkCardIsDuplicated(&cardQueueForEEPROM, writeCard->uuid))
    {

        bNACKAction();
        return false;
    }
    else
    {
        // Sync to the database in Tiva C
        enqueueCard(&cardQueueForEEPROM, writeCard->name, writeCard->id, writeCard->uuid);
        
        // Save back to EEPROM
        saveCardsToEEPROM(&cardQueueForEEPROM);
        printAllCards(&cardQueueForEEPROM);

        // Send Back the UUID to the GUI
        bACKAdded();

        return true;
    }

}

bool bRemoveAction(uint32_t id)
{
    if(removeCard(&cardQueueForEEPROM, id))
    {
        saveCardsToEEPROM(&cardQueueForEEPROM);
        return true;
    }
    else 
        return false;
}

bool bUpdateAction(card *updateCard)
{
    int i;
    // We need to disable the timeout if a card already been detected
    TimerDisable(TIMER2_BASE, TIMER_A);

    for (i = 0; i < CARD_LENGTH; i++)
        updateCard->uuid[i] = (uint32_t) cardUUID[i];
    
    // Reset for next use
    memset(cardUUID, 0, CARD_LENGTH);

    // Send an ACK to the GUI
    bACKUpdated();
    // Write for existing card. These codes below is using to write for only a writeable card
    // We dont need them at this time. So skip here, uncomment them in case you need to use.
    // writeName((uint8_t *) updateCard->name);
    // writeID(updateCard->id);

    if(updateCardBaseOnUUID(&cardQueueForEEPROM, updateCard->uuid, updateCard->name, updateCard->id))
    {
        saveCardsToEEPROM(&cardQueueForEEPROM);
        printAllCards(&cardQueueForEEPROM);
        return true;
    }
    else
        return false;
}

// Function to send back the ACK when GUI Requests
void bACKRequestAction(void)
{
    // Frame: 0xFFAA - 'O' - 0xAAFF
    // Start of Frame
    UARTCharPut(UART1_BASE, 0xFF);
    UARTCharPut(UART1_BASE, 0xAA);

    // Identifier 'O' for OK to send back to the GUI
    UARTCharPut(UART1_BASE, 'O');

    // End of Frame
    UARTCharPut(UART1_BASE, 0xAA);
    UARTCharPut(UART1_BASE, 0xFF);
}

void bACKAdded(void)
{
    int i = 0;
    // After adding, the UUID in the card will be sent back to the GUI
    // Frame: 0xFFAA - 'A' - data (UUID) - 0xAAFF
    // Start of Frame
    UARTCharPut(UART1_BASE, 0xFF);
    UARTCharPut(UART1_BASE, 0xAA);

    // Identifier 'A' for indicating the data is from the "card" structure
    UARTCharPut(UART1_BASE, 'A');

    // Send back UUID to the GUI after adding Name and ID in Tiva C
    for (i = 0; i < sizeof(cardNeedToDo.uuid) / sizeof(uint32_t); i++) {
        UARTCharPut(UART1_BASE, (uint8_t) cardNeedToDo.uuid[i]);
    }

    // End of Frame
    UARTCharPut(UART1_BASE, 0xAA);
    UARTCharPut(UART1_BASE, 0xFF);
}

void bACKUpdated(void)
{
    // After adding, the UUID in the card will be sent back to the GUI
    // Frame: 0xFFAA - 'A' - data (UUID) - 0xAAFF
    // Start of Frame
    UARTCharPut(UART1_BASE, 0xFF);
    UARTCharPut(UART1_BASE, 0xAA);

    // Identifier 'U' for indicating the data is from the "card" structure
    UARTCharPut(UART1_BASE, 'U');

    // End of Frame
    UARTCharPut(UART1_BASE, 0xAA);
    UARTCharPut(UART1_BASE, 0xFF);
}

void bNACKAction(void)
{
    // After adding, the card is not scanned
    // Frame: 0xFFAA - 'N' - - 0xAAFF
    // Start of Frame
    UARTCharPut(UART1_BASE, 0xFF);
    UARTCharPut(UART1_BASE, 0xAA);

    // Identifier 'N' for indicating the NACK
    UARTCharPut(UART1_BASE, 'N');

    // End of Frame
    UARTCharPut(UART1_BASE, 0xAA);
    UARTCharPut(UART1_BASE, 0xFF);
}

// Function to send Length of data when synchronizing
void sendLength(uint16_t length)
{
    int i;
    // Frame: 0xFFAA - 'L' - length (2 bytes) - 0xAAFF
    // Start of Frame
    UARTCharPut(UART1_BASE, 0xFF);
    UARTCharPut(UART1_BASE, 0xAA);

    // Identifier 'L' for Length to send back to the GUI
    UARTCharPut(UART1_BASE, 'L');

    // Send length of frame
    uint8_t* lengthBytes = (uint8_t*)&length;
    for (i = 0; i < sizeof(length); i++) {
        UARTCharPut(UART1_BASE, *(lengthBytes + i));
    }

    // End of Frame
    UARTCharPut(UART1_BASE, 0xAA);
    UARTCharPut(UART1_BASE, 0xFF);
}

void bStartTimeOut(void)
{
    TimerDisable(TIMER2_BASE, TIMER_A);
    HWREG(TIMER2_BASE+0x50) = 6*SysCtlClockGet() - 1;
    TimerEnable(TIMER2_BASE, TIMER_A);
}

// Static functions
void dumpHex(unsigned char* buffer, int len){

    int i;
    UARTprintf(" ");
    for(i=0; i < len; i++) {
        UARTprintf("0x%x, ", buffer[i]);
    }
    UARTprintf("  FIM! \r\n"); //End

}
// Function to send the data of the verified card
void verifiedSending(card* myCard)
{
    int i;
    // After passinng, the data in the card will be sent to the GUI
    // Frame: 0xFFAA - 'V' - data (Name, ID) - 0xAAFF
    // Start of Frame
    UARTCharPut(UART1_BASE, 0xFF);
    UARTCharPut(UART1_BASE, 0xAA);

    // Identifier 'V' for indicating the data is from the "card" structure
    UARTCharPut(UART1_BASE, 'V');

    // Send ID
    uint8_t* idBytes = (uint8_t*)&myCard->id;
    for (i = 0; i < sizeof(myCard->id); i++) {
        UARTCharPut(UART1_BASE, idBytes[i]);
    }

    // Send Name
    for (i = 0; i < sizeof(myCard->name); i++) {
        UARTCharPut(UART1_BASE, myCard->name[i]);
    }

    // End of Frame
    UARTCharPut(UART1_BASE, 0xAA);
    UARTCharPut(UART1_BASE, 0xFF);

    // Clear the card
    initCard(&verifiedCard);
}

void normalDisplay()
{
    // Display normally
    // Line 2: ACCESS CONTROL SYSTEM
    // Line 3: Please verify your card!
    lcdPrint(lcd, "\n ACCESS CTRL SYSTEM\nPlease scan ur card!");
    lcd->dirty = true;


}

void warningDisplay()
{
    // Display a warning
    // Line 2: ACCESS IS DENIED!
    // Line 3: Please contact the
    // Line 4: admin to help check.
    lcdPrint(lcd, "  ACCESS IS DENIED  \n Please contact the \nadmin to help check.");
    lcd->dirty = true;

}

void passDisplay()
{
    // Display a passing
    // Line 2: ID:
    // Line 3: Name:
    char buffer[80];
    memset(buffer, 0, sizeof(buffer));
    if(strlen(verifiedCard.name) < 20)
        sprintf(buffer, " ACCESS IS ACCEPTED\nName: %s\n\nID: %d", verifiedCard.name, verifiedCard.id);
    else
        sprintf(buffer, " ACCESS IS ACCEPTED\nName: %s\nID: %d", verifiedCard.name, verifiedCard.id);
    lcdPrint(lcd, buffer);
    lcd->dirty = true;
}

// Function to parse the first frame in raw data
// Function to parse the first frame in raw data
bool parseFirstFrameInRawData(const uint8_t *data_stream, size_t stream_length) {
    size_t i;
    size_t frame_start = 0;
    size_t frame_end = 0;

    // Find the start marker
    for (i = 0; i < stream_length - 1; ++i) {
        if ((data_stream[i] == 0xFF) && (data_stream[i + 1] == 0xAA)) {
            frame_start = i;
            break;
        }
    }

    // Find the end marker
    for (i = frame_start; i < stream_length - 1; ++i) {
        if ((data_stream[i] == 0xAA) && (data_stream[i + 1] == 0xFF)) {
            frame_end = i + 2;
            break;
        }
    }

    // Check if both markers were found
    if (frame_start < frame_end) {
        // Extract the frame and store it in the global variable
        size_t frame_length = frame_end - frame_start;
        memcpy(receivedFrame, &data_stream[frame_start], frame_length);
        receivedFrameLength = frame_length;
        return true;
        // If there is more data after the frame, you can process it here
    } else {
        DBG("Incomplete frame received.\n");
        return false;
    }
}

// Function to modify the data based on the functional code in the frame
void parseDataInFrame(char *frame, card *dataCard)
{
    char functionalCode = frame[2];
    int i;

    // Reset the card first, then parse it later
    initCard(dataCard);

    // Extract data based on the functional code
    switch (functionalCode) {
        case 'A':
            gFSM_.event_ = GUI_E_ADD;
            DBG("Add code\n");
            strncpy(dataCard->name, frame + 3, 32);
            for (i = 0; i < 4; i++){
                idBytes[i] = (uint32_t)frame[38-i];
            }
            dataCard->id = combineBytes(idBytes);
            break;

        case 'D':
            gFSM_.event_ = GUI_E_REMOVE;
            DBG("Remove code\n");
            for (i = 3; i >= 0; i--){
                idBytes[i] = (uint32_t)frame[i+3];
            }
            dataCard->id = combineBytes(idBytes);
            DBG("Remove ID: %d\n", dataCard->id);
            break;

        case 'S':
            gFSM_.event_ = GUI_E_SYNC;
            DBG("Sync code\n");
            break;

        case 'F':
            gFSM_.event_ = GUI_E_FINISHED;
            DBG("GUI finished code\n");
            break;

        case 'R':
            bFSM_.event_ = E_GUI_REQUEST;
            DBG("Request code\n");
            break;

        case 'U':
            gFSM_.event_ = GUI_E_UPDATE;
            strncpy(dataCard->name, frame + 8, 32);
            for (i = 0; i < 4; i++){
                idBytes[i] = (uint32_t)frame[43-i];
            }
            dataCard->id = combineBytes(idBytes);
            for (i = 0; i < 5; i++){
                dataCard->uuid[i] = (uint32_t)frame[i+3];
            }
            break;
        default:
            // Handle invalid functional code
//            currentEvent = GUI_E_FINISHED;
            DBG("Invalid functional code\n");
            break;
    }
}

uint32_t combineBytes(uint8_t bytes[4]) {
    // Combine the bytes into a uint32_t value
    uint32_t result = 0;
    result |= ((uint32_t)bytes[0] << 24);
    result |= ((uint32_t)bytes[1] << 16);
    result |= ((uint32_t)bytes[2] << 8);
    result |= bytes[3];
    return result;
}

void sync1Card(card *syncCard)
{
    int i = 0;
    // After passinng, the data in the card will be sent to the GUI
    // Frame: 0xFFAA - 'S' - data (Name, ID, UUID) - 0xAAFF
    // Start of Frame
    UARTCharPut(UART1_BASE, 0xFF);
    UARTCharPut(UART1_BASE, 0xAA);

    // Identifier 'V' for indicating the data is from the "card" structure
    UARTCharPut(UART1_BASE, 'S');

    // Send ID
    uint8_t* idBytes = (uint8_t*)&syncCard->id;
    for (i = 0; i < sizeof(syncCard->id); i++) {
        UARTCharPut(UART1_BASE, idBytes[i]);
    }

    // Send Name
    for (i = 0; i < sizeof(syncCard->name); i++) {
        UARTCharPut(UART1_BASE, syncCard->name[i]);
    }

    // Send UUID
    for (i = 0; i < sizeof(syncCard->uuid) / sizeof(uint32_t); i++) {
        UARTCharPut(UART1_BASE, (uint8_t) syncCard->uuid[i]);
    }

    // End of Frame
    UARTCharPut(UART1_BASE, 0xAA);
    UARTCharPut(UART1_BASE, 0xFF);

}

int8_t writeID(uint8_t id)
{
    int8_t i, status;
    uint8_t uuidTemp[4];

    for(i = 0; i < CARD_LENGTH - 1; i++)
        uuidTemp[i] = (uint8_t) cardNeedToDo.uuid[i];
        
	status = rc522SelectTag(uuidTemp);
	if(status!=MI_OK)
	{
		DBG("select card: no card.\n");
		return status;            
	}

    status = rc522Auth(PICC_AUTHENT1A, WRITE_ID_BLOCK, passWd, uuidTemp);
    if(status!=MI_OK)
    {
        DBG("write authrioze err.\n");
        return status;
    }
    status = rc522WriteBlock(WRITE_ID_BLOCK, &id);
    if(status!=MI_OK)
        DBG("write data err.\n");
    else
        DBG("write data sucess.\n");
    return status;
}

int8_t writeName(uint8_t *name)
{
    int8_t i, status;
    uint8_t uuidTemp[4];

    for(i = 0; i < CARD_LENGTH - 1; i++)
        uuidTemp[i] = (uint8_t) cardNeedToDo.uuid[i];

	status = rc522SelectTag(uuidTemp);
	if(status!=MI_OK)
	{
		DBG("select card: no card.\n");
		return status;            
	}

	for(i = 0; i<64; i++ )
    {
	    status = rc522Auth(PICC_AUTHENT1A, i, passWd, uuidTemp);
        if(status!=MI_OK)
        {
            DBG("%d: write authrioze err.\n", i);
    //        return status;
        }
    }
    status = rc522WriteBlock(WRITE_NAME_BLOCK, &name[0]);
    if(status!=MI_OK)
        DBG("write data err.\n");
    else
        DBG("write data sucess.\n");
    return status;
}
