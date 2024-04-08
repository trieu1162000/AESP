/*
 * actions_api.c
 *
 *  Created on: Nov 17, 2023
 *      Author: trieu
 */

#include "../inc/actions_api.h"
#include "../inc/caculator_api.h"
#include <math.h>

char display_buffer[MAX_LENGTH_DISPLAY_LCD] = {'\0'};
char alpha_display_buffer[MAX_ALPHA_BUFFER] = {'\0'};
char whole_chars_buffer[MAX_LENGTH_SUPPORTED] = {'\0'};
uint8_t current_length_buffer = 0U;
int8_t current_pos_char_in_buffer = -1;

static char result_buffer[MAX_LENGTH_LCD] = {'\0'};
static char double_string[MAX_DOUBLE_STRING] = "0.";
static char decimal_part[MAX_DOUBLE_STRING/2] = {'\0'};
static struct find_equation_parms sF_equation_parms;
static uint8_t changed_sign_index = INVALID_VALUE;
static bool is_finding_equation = false;

static bool isOver16Digits(int64_t num) {
    uint8_t count = 0U;
    // Counting the number of digits
    while (num != 0U) {
        num /= 10U;
        ++count;
    }
    // If count is greater than 16, return true, else return false
    return (count > 16U) ? true : false;
}

static void deleteSubstring(char *des_buffer, int start, int length) {
    int buffer_length = strlen(des_buffer);
    if (start < 0 || start >= buffer_length || length <= 0 || start + length > buffer_length) {
        printf("Invalid parameters\n");
        return;
    }

    memmove(des_buffer + start, des_buffer + start + length, buffer_length - start - length + 1);
}

static void insertBuffer(char *des_buffer, const char *insert_string, int8_t position) {
    int buffer_length = strlen(des_buffer);
    int insert_length = strlen(insert_string);

    // Adjusting position if it's negative
    position ++;
    
    // Adjusting position if it's greater than the buffer length
    if (position > buffer_length) {
        position = buffer_length;
    }

    // Shift characters to make space for insert_string
    memmove(des_buffer + position + insert_length, 
            des_buffer + position, 
            buffer_length - position + 1);

    // Copy insert_string into the buffer
    memcpy(des_buffer + position, insert_string, insert_length);
}

static void insertChar(char *des_buffer, char insert_char, int position) {
    int buffer_length = strlen(des_buffer);
    
    // Adjusting position if it's negative
    position ++;
    
    // Adjusting position if it's greater than the buffer length
    if (position > buffer_length) {
        position = buffer_length;
    }

    // Shift elements to make space for the insert character
    memmove(&des_buffer[position + 1], &des_buffer[position], buffer_length - position + 1);
    
    // Insert character into buffer
    des_buffer[position] = insert_char;
}

static bool parseWholeBuffer(char *buffer_to_parse)
{
    uint8_t i = 0U;
    uint8_t len = strlen(buffer_to_parse);
    for(; i < strlen(buffer_to_parse); i++)
    {
        // Case operands (0-9)
        if( (buffer_to_parse[i] >= '0') && (buffer_to_parse[i] <= '9') )
        {
            if (sCaculator_->is_new_operand)
            {
                if (sCaculator_->current_numberOfOperands <= MAX_OPERANDS)
                {
                    sCaculator_->operands[sCaculator_->current_numberOfOperands] = (double)(buffer_to_parse[i] - '0');
                    sCaculator_->is_new_operand = false;
                    sCaculator_->current_numberOfOperands++;
                }
                else
                {
                    // Handle error here when exceeding the maximum operands which the system support
                    return false;
                }

            }
            else
            {
                sCaculator_->operands[sCaculator_->current_numberOfOperands - 1] = sCaculator_->operands[sCaculator_->current_numberOfOperands - 1] * 10.0 + (double)(buffer_to_parse[i] - '0');
            }
        }

        // Case operators (+, -, *, :, sin, cos, log)
        else
        {
            switch(buffer_to_parse[i])
            {
            // Plus
            case '+':
                if(sCaculator_->is_new_operand)
                {
                    // Skip adding the operator
                }
                else
                {
                    sCaculator_->operators[sCaculator_->current_numberOfOperators] = O_ADD;
                    sCaculator_->current_numberOfOperators++;
                    sCaculator_->is_new_operand = true;
                }
                break;
            // Subtract
            case '-':
                if(sCaculator_->is_new_operand)
                {
                    // Subtract positive
                    if('+' == buffer_to_parse[i-1U])
                    {
                        sCaculator_->operands[sCaculator_->current_numberOfOperands] = 0.0;
                        sCaculator_->operators[sCaculator_->current_numberOfOperators] = O_SUBTRACT;
                    }
                    // Subtract negative
                    else if('-' == buffer_to_parse[i-1U])
                    {
                        sCaculator_->operands[sCaculator_->current_numberOfOperands] = 0.0;
                        sCaculator_->operators[sCaculator_->current_numberOfOperators] = O_ADD;

                        // These lines to handle the case for greater than 3 subtract signs (> '---')
                        buffer_to_parse[i-1] = '+';
                        buffer_to_parse[i] = '+';
                    }
                    else if('x' == buffer_to_parse[i-1U])
                    {
                        // do nothing
                        sCaculator_->current_numberOfOperands--;

                    }
                    else
                    {
                        sCaculator_->operands[sCaculator_->current_numberOfOperands] = -1.0;
                        sCaculator_->operators[sCaculator_->current_numberOfOperators] = sCaculator_->operators[sCaculator_->current_numberOfOperators - 1];
                    }
                    sCaculator_->current_numberOfOperands++;
                }
                else
                    sCaculator_->operators[sCaculator_->current_numberOfOperators] = O_SUBTRACT;

                sCaculator_->current_numberOfOperators++;
                sCaculator_->is_new_operand = true;
                break;
            // Multiply
            case '*':
                sCaculator_->operators[sCaculator_->current_numberOfOperators] = O_MULTIPLY;
                sCaculator_->current_numberOfOperators++;
                sCaculator_->is_new_operand = true;
                break;
            // Divide
            case ':':
                sCaculator_->operators[sCaculator_->current_numberOfOperators] = O_DIVIDE;
                sCaculator_->current_numberOfOperators++;
                sCaculator_->is_new_operand = true;
                break;
            // Sine
            case 's':
                // Padding operand with value 1
                if (sCaculator_->is_new_operand)
                {

                    sCaculator_->operands[sCaculator_->current_numberOfOperands] = 1;
                    sCaculator_->current_numberOfOperands++;

                }
                sCaculator_->operators[sCaculator_->current_numberOfOperators] = O_SIN;
                sCaculator_->current_numberOfOperators++;
                sCaculator_->is_new_operand = true;
                i = i + 2;
                break;
            // Cosine
            case 'c':
                // Padding operand with value 1
                if (sCaculator_->is_new_operand)
                {

                    sCaculator_->operands[sCaculator_->current_numberOfOperands] = 1;
                    sCaculator_->current_numberOfOperands++;

                }
                sCaculator_->operators[sCaculator_->current_numberOfOperators] = O_COS;
                sCaculator_->current_numberOfOperators++;
                sCaculator_->is_new_operand = true;
                i = i + 2;
                break;
            // Logarith
            case 'l':
                // Handle for log with base value is 1
                if (1.0 == sCaculator_->operands[sCaculator_->current_numberOfOperands])
                {
                    return false;
                }
                sCaculator_->operators[sCaculator_->current_numberOfOperators] = O_LOG;
                sCaculator_->current_numberOfOperators++;
                sCaculator_->is_new_operand = true;
                i = i + 2;
                break;
            // Floating point
            case '.':
                sCaculator_->operators[sCaculator_->current_numberOfOperators] = O_DOUBLE;
                sCaculator_->current_numberOfOperators++;
                sCaculator_->is_new_operand = true;
                break;
            // Power
            case '^':
                sCaculator_->operators[sCaculator_->current_numberOfOperators] = O_POWER;
                sCaculator_->current_numberOfOperators++;
                sCaculator_->is_new_operand = true;
                break;
            // Ans
            case 'A':
                sCaculator_->operands[sCaculator_->current_numberOfOperands] = sCaculator_->current_result;
                sCaculator_->current_numberOfOperands++;
                i = i + 2;
                break;
            case 'r':
                // Default is square root (sqrt)
                // Padding operand with value 2
                if (sCaculator_->is_new_operand)
                {

                    sCaculator_->operands[sCaculator_->current_numberOfOperands] = 2;
                    sCaculator_->current_numberOfOperands++;

                }
                sCaculator_->operators[sCaculator_->current_numberOfOperators] = O_ROOT;
                sCaculator_->current_numberOfOperators++;
                sCaculator_->is_new_operand = true;
                break;
            // Equation with level of 1
            case 'x':
                // Set flag indicate that finding the equation
                if(!is_finding_equation)
                    is_finding_equation = true;

                if (sCaculator_->is_new_operand)
                {

                    sCaculator_->operands[sCaculator_->current_numberOfOperands] = 1;
                    sCaculator_->current_numberOfOperands++;

                }
                sCaculator_->operators[sCaculator_->current_numberOfOperators] = O_EQUATION;
                sCaculator_->current_numberOfOperators++;

                // Padding with 1 if needed
                if ( ('0' >= buffer_to_parse[i+1]) && ('9' <= buffer_to_parse[i]) )
                {
                    sCaculator_->operands[sCaculator_->current_numberOfOperands] = 1;
                    sCaculator_->current_numberOfOperands++;
                    sCaculator_->is_new_operand = false;
                }
                else
                    sCaculator_->is_new_operand = true;
                break;
            case '=':
                sCaculator_->operators[sCaculator_->current_numberOfOperators] = O_SUBTRACT;
                sCaculator_->current_numberOfOperators++;
                sCaculator_->is_new_operand = true;
                changed_sign_index = sCaculator_->current_numberOfOperators;
                break;
            default:
                break;
            }
        }
    }
    return true;
}

static void convertAlphaCharacter(char input_character)
{
    switch (input_character)
    {
    case '1':
        strcpy(alpha_display_buffer, "sin");
        break;
    case '2':
        strcpy(alpha_display_buffer, "cos");
        break;
    case '3':
        strcpy(alpha_display_buffer, "log");
        break;
    case '4':
        // 'x' charracter along with coefficient of equation
        strcpy(alpha_display_buffer, "x");
        break;
    case '5':
        // Floating Point
        strcpy(alpha_display_buffer, ".");
        break;
    case '6':
        // Power
        strcpy(alpha_display_buffer, "^");
        break;
    case '7':
        // Previous Result
        strcpy(alpha_display_buffer, "Ans");
        break;
    case '8':
        // AC (Reset)
        // Do nothing. Already checked in parseKey()
        break;
    case '9':
        // root character
        strcpy(alpha_display_buffer, "r");
        break;
    case '0':
        // 'd' means delete the character
        strcpy(alpha_display_buffer, "d");
        break;
    case '#':
        // '=' character for finding equation feature
        strcpy(alpha_display_buffer, "=");
        break;
    // A, B, C, D which are equivalent to Up, Down, Left, Right respectively
    // will be ignore. Handle them will be done later 
    default:
        break;
    }
}

void appendDisplay(struct lcd_i2c *lcd_todo)
{
    // If press alpha key
    if(is_alpha_character && ( ALPHA_VALUE_FALSE == alpha_character_val ))
    {
        uint8_t prev_col = lcd_todo->pos.col;
        // Just display the alpha character
        lcdGotoXY(lcd_todo, 1, 0);
        lcdPrintAlphaCharacter(lcd_todo, LCD_ALPHA_CHAR_ADDR);

        // Then, return back to the previous position
        lcdGotoXY(lcd_todo, 0, prev_col);
        alpha_character_val = ALPHA_VALUE_TRUE;

    }
    // If press another key or press alpha key again
    else
    {
        uint8_t prev_col = lcd_todo->pos.col;

        // If alpha key is pressed again
        if(!is_alpha_character && ( ALPHA_VALUE_TRUE == alpha_character_val ))
        {
            // Just clear the alpha character
            lcdGotoXY(lcd_todo, 1, 0);
            lcdClearLine(lcd_todo);

            // Then, return back to previous position
            lcdGotoXY(lcd_todo, 0, prev_col);
            alpha_character_val = ALPHA_VALUE_FALSE;

        }
        else
        {
            // If another key is pressed, there are 2 sub-cases below

            // Check move up or down first
            if( ('A' == pressed_key) || ('B' == pressed_key) )
            {
                // Copy the buffer
                memcpy(display_buffer, whole_chars_buffer, MAX_LENGTH_LCD);

                // Clear the buffer before printing
                lcdClearDisplay(lcd);
                lcdGotoXY(lcd_todo, 0, 0);

                // Print the buffer
                lcdPrint(lcd_todo, display_buffer);
                lcdGotoXY(lcd_todo, 0, 0);
            }

            // In the top of 2 cases, check if the buffer exceed the LCD length supported
            if( (MAX_LENGTH_LCD - 1U) < current_length_buffer)
            {
                // Unless from 'A' to 'D'
                if( ('A' > pressed_key) || ('D' < pressed_key) )
                {
                    if(('0' == pressed_key) && is_alpha_character)
                    {

                        // Only need to update the buffer
                        // Save the column
                        prev_col = lcd_todo->pos.col;

                        // Copy the buffer
                        memcpy(display_buffer, whole_chars_buffer + current_pos_char_in_buffer - lcd_todo->pos.col + 1, MAX_LENGTH_LCD);

                        // Clear the LCD first
                        lcdClearDisplay(lcd_todo);
                        lcdGotoXY(lcd_todo, 0, 0);

                        // Print the buffer
                        lcdPrint(lcd_todo, display_buffer);

                        // Return back to the previous column
                        lcdGotoXY(lcd_todo, 0, prev_col);

                    }
                    else
                    {
                        if((current_length_buffer -1) != current_pos_char_in_buffer)
                        {

                            // Save the previous column
                            prev_col = lcd_todo->pos.col;

                            // Copy the buffer
                            memcpy(display_buffer, whole_chars_buffer + current_pos_char_in_buffer + 1 - lcd_todo->pos.col, MAX_LENGTH_LCD);

                            // Print the buffer
                            lcdGotoXY(lcd_todo,  0,  0);
                            lcdPrint(lcd_todo, display_buffer);

                        }
                        else
                        {
                            // Copy the buffer
                            memcpy(display_buffer, whole_chars_buffer + current_pos_char_in_buffer - MAX_LENGTH_LCD + 2, MAX_LENGTH_LCD);

                            // Print the buffer
                            lcdGotoXY(lcd_todo, 0, 0);
                            lcdPrint(lcd_todo, display_buffer);

                            // Save the previous column
                            prev_col = lcd_todo->pos.col;
                        }

                        // If previous pressed key is alpha key
                        if(is_alpha_character)
                        {
                            // Clear the alpha character
                            lcdGotoXY(lcd_todo, 1, 0);
                            lcdClearLine(lcd_todo);
                            
                        }

                        // Return back to the cursor
                        lcdGotoXY(lcd_todo, 0, prev_col);
                    }

                }
                // If from 'A' to 'D'
                else
                {
                    // Move left
                    if('C' == pressed_key)
                    {
                        // Move the cursor if it is not in [0]
                        if(-1 < lcd_todo->pos.col)
                        {
                            prev_col = lcd_todo->pos.col;

                            // Clear the alpha character also
                            lcdGotoXY(lcd_todo, 1, 0);
                            lcdClearLine(lcd_todo);

                            lcdGotoXY(lcd_todo, 0, prev_col);
                        }
                        else
                        {
                            // Only move the display_buffer when the cursor is already in [0]
                            if(-1 <= current_pos_char_in_buffer)
                            {
                                // Copy the buffer
                                memcpy(display_buffer, whole_chars_buffer + current_pos_char_in_buffer + 1, MAX_LENGTH_LCD);

                                // Print the buffer
                                lcdGotoXY(lcd_todo, 0, 0);
                                lcdPrint(lcd_todo, display_buffer);

                                // Clear the alpha character and some abnormal characters
                                lcdGotoXY(lcd_todo, 1, 0);
                                lcdClearLine(lcd_todo);

                                // Return back to the (0; 0) position
                                lcdGotoXY(lcd_todo, 0, 0);

                            }
                        }
                    }
                    // Move right
                    else if('D' == pressed_key)
                    {
                        // Move the cursor if it is not in [end]
                        if( MAX_LENGTH_LCD > lcd_todo->pos.col)
                        {
                            prev_col = lcd_todo->pos.col;
                        }
                        else
                        {
                            if((current_length_buffer - 1) >= current_pos_char_in_buffer)
                            {
                                //Copy the buffer`
                                int8_t  temp = current_length_buffer - (current_pos_char_in_buffer - MAX_LENGTH_LCD + 2);
                                if(MAX_LENGTH_LCD > temp)
                                {
                                    // Clear the line of lcd first
                                    lcdGotoXY(lcd_todo, 0, 0);
                                    lcdClearLine(lcd_todo);

                                    memcpy(display_buffer, whole_chars_buffer + current_pos_char_in_buffer - MAX_LENGTH_LCD + 2, temp);
                                }
                                else
                                    memcpy(display_buffer, whole_chars_buffer + current_pos_char_in_buffer - MAX_LENGTH_LCD + 2, MAX_LENGTH_LCD);

                                // Print the buffer
                                lcdGotoXY(lcd_todo, 0, 0);
                                lcdPrint(lcd_todo, display_buffer);

                                // Save the previous column
                                prev_col = lcd_todo->geometry->cols - 1;
                            }
                        }
                        
                        // Clear the alpha character also
                        lcdGotoXY(lcd_todo, 1, 0);
                        lcdClearLine(lcd_todo);

                        // Return back to previous position
                        lcdGotoXY(lcd_todo, 0, prev_col);
                    }

                }
            }
            else
            {
                // If previous pressed key is alpha key
                if(is_alpha_character)
                {
                    // Save the position in term of col
                    prev_col = lcd_todo->pos.col;

                    // Clear the alpha character also
                    lcdGotoXY(lcd_todo, 1, 0);
                    lcdClearLine(lcd_todo);

                    // Then, return back to previous position
                    lcdGotoXY(lcd_todo, 0, prev_col);

                    if( ('A' > pressed_key) || ('D' < pressed_key) )
                    {
                        if('0' == pressed_key)
                        {
                            // Clear the LCD first
                            lcdClearDisplay(lcd_todo);
                            lcdGotoXY(lcd_todo, 0, 0);

                            // Print the buffer
                            memcpy(display_buffer, whole_chars_buffer, MAX_LENGTH_LCD);
                            lcdPrint(lcd_todo, display_buffer);

                            // Return back to the previous column
                            lcdGotoXY(lcd_todo, 0, current_pos_char_in_buffer + 1);
                        }
                        else
                        {
                            // Display the alpha buffer
                            if(current_pos_char_in_buffer != (current_length_buffer - 1))
                            {
                                lcd_todo->pos.col = lcd_todo->pos.col - 3;
                                lcdGotoXY(lcd_todo, 0, lcd_todo->pos.col);
                            }

                            lcdPrint(lcd_todo, alpha_display_buffer);

                            // Reset it imemediately
                            memset(alpha_display_buffer, '\0', MAX_ALPHA_BUFFER);

                            // Save the col
                            prev_col = lcd_todo->pos.col;

                            // Shift and display the rest
                            if(current_pos_char_in_buffer != (current_length_buffer - 1))
                            {
                                // Print the buffer
                                memcpy(display_buffer, whole_chars_buffer + current_pos_char_in_buffer + 1, MAX_LENGTH_LCD - lcd_todo->pos.col);
                                lcdPrint(lcd_todo, display_buffer);

                                // Return back to previous col
                                lcdGotoXY(lcd_todo, 0, prev_col);

                            }
                        }
                    }
                    else
                    {
                        // Just move the cursor. Nothing need to be done
                    }

                }
                // If previous pressed key is not alpha key
                else
                {
                    // Print the char
                    char temp_str[2] = {'\0'};
                    temp_str[0] = pressed_key;
                    if(current_pos_char_in_buffer != (current_length_buffer - 1))
                    {
                        lcd_todo->pos.col--;
                        lcdGotoXY(lcd_todo, 0, lcd_todo->pos.col);
                    }
                    lcdPrint(lcd_todo, temp_str);


                    // Save the col
                    prev_col = lcd_todo->pos.col;

                    // Shift and display the rest
                    if(current_pos_char_in_buffer != (current_length_buffer - 1))
                    {
                        // Display the buffer
                        memcpy(display_buffer, whole_chars_buffer + current_pos_char_in_buffer + 1, MAX_LENGTH_LCD - lcd_todo->pos.col);
                        lcdPrint(lcd_todo, display_buffer);

                        // Return back to previous col
                        lcdGotoXY(lcd_todo, 0, prev_col);
                    }
                }
            }
            alpha_character_val = ALPHA_VALUE_FALSE;
            is_alpha_character = false;
        }
    }     
        
}

void clearDisplay(struct lcd_i2c *lcd_todo)
{
    lcdClearDisplay(lcd_todo);
    is_alpha_character = false;
    alpha_character_val = ALPHA_VALUE_FALSE;
    DBG("clear display\n");

}

void resultDisplay(struct lcd_i2c *lcd_todo)
{
    if(MATH_OK == check_result)
    {
        // Display the result to the lcd
        uint8_t col = MAX_LENGTH_LCD - strlen(result_buffer);
        lcdGotoXY(lcd_todo, 1, col);
        lcdPrint(lcd_todo, result_buffer);
        lcdGotoXY(lcd_todo, 0, 0);
    }
    else if(SYNTAX_ERROR == check_result)
    {
        lcdClearDisplay(lcd_todo);
        lcdPrint(lcd_todo, "Syntax ERROR");
    }
    else if(MATH_ERROR == check_result)
    {
        lcdClearDisplay(lcd_todo);
        lcdPrint(lcd_todo, "Math ERROR");
    }
    else
    {
        // default option to prevent some klocwork warning
    }
}

void appendAction(void)
{

    // Check the alpha character first and convert if it really is
    if (is_alpha_character)
    {
        convertAlphaCharacter(pressed_key);

        // Ignore if from 'A' to 'D'
        if( ('A' <= pressed_key) && ('D' >= pressed_key) )
        {
            switch(pressed_key)
            {
            case 'A':
                // Up
                if(sCaculator_ != &sCaculator_list[0])
                {
                    sCaculator_--;
                    memcpy(whole_chars_buffer, sCaculator_->whole_buffer, MAX_LENGTH_SUPPORTED);
                    current_pos_char_in_buffer = -1;
                    current_length_buffer = strlen(whole_chars_buffer);
                }
                // Else, do nothing

                // Clear the caculator
                clearParamsCaculator(sCaculator_);
                break;
            case 'B':
                // Down
                if( (sCaculator_ != &sCaculator_list[MAX_SUPPORT_ELEMENTS - 1U]) && (sCaculator_ != &sCaculator_list[current_caculator_element - 1U]) )
                {
                    sCaculator_++;
                    memcpy(whole_chars_buffer, sCaculator_->whole_buffer, MAX_LENGTH_SUPPORTED);
                    current_pos_char_in_buffer = -1;
                    current_length_buffer = strlen(whole_chars_buffer);
                }
                // Else, do nothing

                // Clear the caculator
                clearParamsCaculator(sCaculator_);
                break;
            case 'C':
                // Left: Only move when not in [0]
                // Check if the cursor is already in the start
                if( -1 < current_pos_char_in_buffer )
                {
                    // Move the cursor to the left
                    // There are 2 sub-cases
                    switch(whole_chars_buffer[current_pos_char_in_buffer])
                    {
                    // Case 1: sin, cos, log, Ans
                    // co's', An's'    
                    case 's':
                    // si'n'
                    case 'n':
                    // lo'g'
                    case 'g':
                        if( -1 < lcd->pos.col)
                            lcd->pos.col = lcd->pos.col - 3;
                        current_pos_char_in_buffer = current_pos_char_in_buffer -3;
                        break;
                    // Case 2: Others
                    default:
                        if( -1 < lcd->pos.col)
                            lcd->pos.col--;
                        current_pos_char_in_buffer--;
                        break;
                    }
                }
                // Else, do nothing
                break;
            case 'D':
                // Right
                // Check if the cursor is already in the end
                if( (current_length_buffer - 1) > current_pos_char_in_buffer )
                {
                    // Move the cursor to the right
                    // There are 2 sub-cases
                    switch(whole_chars_buffer[current_pos_char_in_buffer + 1])
                    {
                    // Case 1: sin, cos, log, Ans
                    // 's'in
                    case 's':
                    // 'c'os
                    case 'c':
                    // 'l'og
                    case 'l':
                    // 'A'ns
                    case 'A':
                        if( MAX_LENGTH_LCD > lcd->pos.col)
                            lcd->pos.col = lcd->pos.col + 3;
                        current_pos_char_in_buffer = current_pos_char_in_buffer + 3;
                        break;
                    // Case 2: Others
                    default:
                        if( MAX_LENGTH_LCD > lcd->pos.col)
                            lcd->pos.col++;
                        current_pos_char_in_buffer++;
                        break;
                    }
                }
                // Else, do nothing
                break;
            }
        }
        else if('0' == pressed_key)
        {
            if(-1 != current_pos_char_in_buffer)
            {
                // Similar to move left but we delete an sub-string of characters, in addition
                // There are 2 sub-cases
                switch(whole_chars_buffer[current_pos_char_in_buffer])
                {
                // Case 1: sin, cos, log, Ans
                // co's', An's'    
                case 's':
                // si'n'
                case 'n':
                // lo'g'
                case 'g':
                    if( -1 < lcd->pos.col)
                        lcd->pos.col = lcd->pos.col - 3;
                    current_pos_char_in_buffer = current_pos_char_in_buffer - 3;
                    deleteSubstring(whole_chars_buffer, current_pos_char_in_buffer + 1, 1);
                    deleteSubstring(whole_chars_buffer, current_pos_char_in_buffer + 1, 1);
                    deleteSubstring(whole_chars_buffer, current_pos_char_in_buffer + 1, 1);
                    current_length_buffer = current_length_buffer - 3;
                    break;
                // Case 2: Others
                default:
                    if( -1 < lcd->pos.col)
                        lcd->pos.col--;
                    current_pos_char_in_buffer--;
                    deleteSubstring(whole_chars_buffer, current_pos_char_in_buffer + 1, 1);
                    current_length_buffer--;
                    break;
                }
            }
        }
        else
        {
            // Append to the buffer with the alpha char buffer
            if(NULL == whole_chars_buffer[MAX_LENGTH_SUPPORTED - 2])
            {
                // Insert alpha buffer into the whole buffer
                insertBuffer(whole_chars_buffer, alpha_display_buffer, current_pos_char_in_buffer);

                // Increase the column until the end position if needed
                if( (int)(MAX_LENGTH_LCD - 1U) != lcd->pos.col && ( current_pos_char_in_buffer !=  (current_length_buffer - 1) ) )
                {
                    lcd->pos.col = lcd->pos.col + strlen(alpha_display_buffer);
                    if( (int)(MAX_LENGTH_LCD - 1U) < lcd->pos.col )
                        lcd->pos.col = (int) (MAX_LENGTH_LCD - 1U);
                }

                current_pos_char_in_buffer = current_pos_char_in_buffer + strlen(alpha_display_buffer);
                current_length_buffer = current_length_buffer + strlen(alpha_display_buffer);

            }
            else
                // Do nothing here if the buffer is full
                DBG("Unknown ERROR: Exceed the maximum buffer\n");
        }

    }
    else
    {
        // Handle for the operator
        if('A' == pressed_key)
        {
            pressed_key = '+';
        }
        else if('B' == pressed_key)
        {
            pressed_key = '-';
        }
        else if('C' == pressed_key)
        {
            pressed_key = '*';
        }
        else if('D' == pressed_key)
        {
            pressed_key = ':';
        }
        else
        {
            // default option
        }
        // Append to the buffer with the current character (pressed key)
        if(NULL == whole_chars_buffer[MAX_LENGTH_SUPPORTED - 2])
        {
            // Insert the character
            insertChar(whole_chars_buffer, pressed_key, current_pos_char_in_buffer);

            // Increase the column until the end position if needed
            if( (int)(MAX_LENGTH_LCD - 1U) > lcd->pos.col && ( ( current_pos_char_in_buffer !=  (current_length_buffer - 1) ) ) )
                lcd->pos.col++;

            current_pos_char_in_buffer++;
            current_length_buffer++;

            DBG("append action\n");
        }
        else
            DBG("Unknown ERROR: Exceed the maximum buffer\n");
    }
}

giveResultType_t giveResultAction(void)
{
    bool ret = false;

    // Parse the string to get operators and operands
    ret = parseWholeBuffer(whole_chars_buffer);

    if(false == ret)
        return MATH_ERROR;

    if((0U != sCaculator_->current_numberOfOperators) && (sCaculator_->current_numberOfOperators >= sCaculator_->current_numberOfOperands))
    {
        DBG("Syntax ERROR\n");
        return SYNTAX_ERROR;
    }

    // Reset the value of params related to solveQuadraticEquation function
    sF_equation_parms.coefficient_x0 = 0.0;
    sF_equation_parms.coefficient_x1 = 0.0;
    sF_equation_parms.coefficient_x2 = 0.0;

    // Depend on the operators, we caculate the result on them
    uint8_t i = 0U;
    for(;i < sCaculator_->current_numberOfOperators; i++)
    {
        switch (sCaculator_->operators[i])
        {
        case O_SIN:
            sCaculator_->operands[i+1] = sCaculator_->operands[i] * sineFunc(sCaculator_->operands[i+1]);
            sCaculator_->operands[i] = 0.0;
            break;
        case O_COS:
            sCaculator_->operands[i+1] = sCaculator_->operands[i] * cosineFunc(sCaculator_->operands[i+1]);
            sCaculator_->operands[i] = 0.0;
            break;
        case O_SUBTRACT:
            sCaculator_->operands[i+1] = - sCaculator_->operands[i+1];
            break;
        case O_ADD:
            // Do nothing
            break;
        case O_DIVIDE:
            sCaculator_->operands[i+1] = sCaculator_->operands[i] / sCaculator_->operands[i+1];
            sCaculator_->operands[i] = 0.0;
            break;
        case O_MULTIPLY:
            sCaculator_->operands[i+1] = sCaculator_->operands[i] * sCaculator_->operands[i+1];
            sCaculator_->operands[i] = 0.0;
            break;
        case O_LOG:
            sCaculator_->operands[i+1] = logFunc(sCaculator_->operands[i], sCaculator_->operands[i+1]);
            sCaculator_->operands[i] = 0.0;
            break;
        case O_DOUBLE:
            memset(double_string, 0, MAX_DOUBLE_STRING);
            memset(decimal_part, 0, MAX_DOUBLE_STRING/2);

            strcpy(double_string, "0.");
            sprintf(decimal_part, "%llu", (uint64_t) sCaculator_->operands[i+1]);
            strcat(double_string, decimal_part);
            sscanf(double_string, "%lf", &sCaculator_->operands[i+1]);
            break;
        case O_POWER:
            if(sCaculator_->operators[i+1] == O_DOUBLE)
            {
                // Syntax ERROR, not support exponent with double type
                DBG("Syntax ERROR\n");
                return SYNTAX_ERROR;
            }

            if(0.0 > sCaculator_->operands[i])
            {
                double temp_operand = -sCaculator_->operands[i];
                temp_operand = powerFunc(temp_operand,  (int64_t) sCaculator_->operands[i+1]);
                sCaculator_->operands[i+1] = -temp_operand;
            } 
            else
                sCaculator_->operands[i+1] = powerFunc(sCaculator_->operands[i], (int64_t) sCaculator_->operands[i+1]);
            sCaculator_->operands[i] = 0.0;
            break;
        case O_ROOT:
            if(0.0 == sCaculator_->operands[i])
            {
                // Syntax ERROR, not support exponent with double type
                DBG("Syntax ERROR\n");
                return SYNTAX_ERROR;
            }
            sCaculator_->operands[i+1] = (pow(sCaculator_->operands[i+1], 1.0/sCaculator_->operands[i]));
            sCaculator_->operands[i] = 0.0;
            break;
        case O_EQUATION:
            switch ((int32_t)sCaculator_->operands[i+1])
            {
            case 2:
                sF_equation_parms.coefficient_x2 += sCaculator_->operands[i];
                break;
            case 1:
                sF_equation_parms.coefficient_x1 += sCaculator_->operands[i];
                break;
            // Handle syntax error (Power with base out of range (1, 2))
            default:
                return SYNTAX_ERROR;
            }
            sCaculator_->operands[i] = 0.0;
            sCaculator_->operands[i+1] = 0.0;
            break;
        default:
            break;
        }

        if(changed_sign_index <= sCaculator_->current_numberOfOperators)
        {
            // Need to change O_ADD to O_SUBTRACT
            if(O_ADD == sCaculator_->operators[changed_sign_index])
                sCaculator_->operators[changed_sign_index] = O_SUBTRACT;
        }
    }

    if(is_finding_equation)
    {
        sF_equation_parms.coefficient_x0 = sumAllOperands(sCaculator_);
        eResultType_t result_type = solveQuadraticEquation(sF_equation_parms.coefficient_x2, sF_equation_parms.coefficient_x1, sF_equation_parms.coefficient_x0);
        if(SAME_ROOT == result_type)
            snprintf(result_buffer, MAX_LENGTH_LCD, "x=%.2f", result_x1);
        else if (DIFF_ROOT == result_type)
            snprintf(result_buffer, MAX_LENGTH_LCD, "x=%.2f, x=%.2f", result_x1, result_x2);
        else
            snprintf(result_buffer, MAX_LENGTH_LCD, "No real root");
        is_finding_equation = false;
    }
    else
    {
        sCaculator_->current_result = sumAllOperands(sCaculator_);

        DBG("Result: %d\n", sCaculator_->current_result);
        // Convert double/rational/int to string
        if( ( (sCaculator_->current_result - ((int64_t)sCaculator_->current_result)) > POSITIVE_THRESH_HOLD ) || ( (sCaculator_->current_result - ((int64_t)sCaculator_->current_result)) < NEGATIVE_THRESH_HOLD ))
        {
            snprintf(result_buffer, MAX_LENGTH_LCD, "%2f", sCaculator_->current_result);

            // Filter for rational number
            while('0' == result_buffer[strlen(result_buffer) - 1])
                result_buffer[strlen(result_buffer) - 1UL] = '\0';
        }
        else
        {
            DBG("Int\n");
            sprintf(result_buffer, "%lld", (int64_t)sCaculator_->current_result);
        }

    }

    changed_sign_index = INVALID_VALUE;

    // Save the buffer before reset
    memcpy(sCaculator_->whole_buffer, whole_chars_buffer, MAX_BUFFER_LENGTH);

    // Save the result
    double tmp_result = sCaculator_->current_result;

    // Point back to the current caculator element
    sCaculator_ = &sCaculator_list[current_caculator_element -1];

    // Fix a logic bug here
    if('\0' != sCaculator_->whole_buffer[0])
    {
        // Move to the next element and reset it
        if(sCaculator_ == &sCaculator_list[MAX_SUPPORT_ELEMENTS - 1])
        {
            sCaculator_ = &sCaculator_list[0];
        }
        else
        {
            current_caculator_element++;
            sCaculator_ = &sCaculator_list[current_caculator_element -1];
        }
        initParamsCaculator(sCaculator_);
    }

    // Reset the buffer
    memset(whole_chars_buffer, '\0', MAX_LENGTH_SUPPORTED);
    memset(display_buffer, '\0', MAX_LENGTH_LCD);
    memset(alpha_display_buffer, '\0', MAX_ALPHA_BUFFER);
    current_length_buffer = 0U;
    current_pos_char_in_buffer = -1;

    if(true == isOver16Digits((int64_t) tmp_result))
    {
        // Handle error when exceeding 16 digits
        sCaculator_->current_result = 0.0;
        return MATH_ERROR;
    }
    else
    {
        // Get the previous result
        sCaculator_->current_result = tmp_result;
        return MATH_OK;
    }

}

void clearAction(void)
{
    clearParamsCaculator(sCaculator_);

    memset(whole_chars_buffer, '\0', MAX_LENGTH_SUPPORTED);
    memset(display_buffer, '\0', MAX_LENGTH_LCD);
    memset(alpha_display_buffer, '\0', MAX_ALPHA_BUFFER);

    current_length_buffer = 0U;
    current_pos_char_in_buffer = -1;

}
