/*
 * actions_api.c
 *
 *  Created on: Nov 17, 2023
 *      Author: trieu
 */

#include "../inc/actions_api.h"
#include "../inc/caculator_api.h"
#include <math.h>

char display_buffer[MAX_LENGTH_LCD] = {'\0'};
char alpha_display_buffer[MAX_ALPHA_BUFFER] = {'\0'};
char whole_chars_buffer[MAX_LENGTH_SUPPORTED] = {'\0'};
uint8_t current_length_buffer = 0U;

static char result_buffer[MAX_LENGTH_LCD] = {'\0'};
static char double_string[MAX_DOUBLE_STRING] = "0.";
static char decimal_part[MAX_DOUBLE_STRING/2] = {'\0'};
static struct find_equation_parms sF_equation_parms;
static uint8_t changed_sign_index = INVALID_VALUE;
static bool is_finding_equation = false;

static void parseWholeBuffer(char *buffer_to_parse)
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
                    // TODO: Handle error here when exceeding the maximum operands which the system support
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
                sCaculator_->operators[sCaculator_->current_numberOfOperators] = O_ADD;
                sCaculator_->current_numberOfOperators++;
                sCaculator_->is_new_operand = true;
                break;
            // Subtract
            case '-':
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
                // TODO: Handle for log with base value is 1
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
        // '=' character for finding equation feature
        strcpy(alpha_display_buffer, "=");
        break;
    case 'A':
        // Up
    case 'B':
        // Down
    case 'C':
        // Left
    case 'D':
        // Right
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
            // In the top of 2 cases, check if the buffer exceed the LCD length supported
            if(current_length_buffer > MAX_LENGTH_LCD - 1)
            {
                memcpy(display_buffer, whole_chars_buffer + current_length_buffer - MAX_LENGTH_LCD + 1, MAX_LENGTH_LCD - 1);

                lcdClearDisplay(lcd_todo);
                lcd_todo->dirty = true;
                lcdPrint(lcd_todo, display_buffer);
            }
            else
            {
                // If previous pressed key is alpha key
                if(is_alpha_character)
                {
                    // Display the alpha buffer
                    lcdPrint(lcd_todo, alpha_display_buffer);
                    prev_col = lcd_todo->pos.col;

                    // Clear the alpha character also
                    lcdGotoXY(lcd_todo, 1, 0);
                    lcdClearLine(lcd_todo);

                    // Then, return back to previous position
                    lcdGotoXY(lcd_todo, 0, prev_col);

                    alpha_character_val = ALPHA_VALUE_FALSE;
                    is_alpha_character = false;

                }
                else
                {
                    // If previous pressed key is not alpha key
                    char temp_str[2] = {'\0'};
                    temp_str[0] = pressed_key;
                    lcdPrint(lcd_todo, temp_str);
                }
            }
        }
    }     
        
}

void clearDisplay(struct lcd_i2c *lcd_todo)
{
    lcdClearDisplay(lcd_todo);
    DBG("clear display\n");

}

void resultDisplay(struct lcd_i2c *lcd_todo)
{
    if(check_result)
    {
        // Display the result to the lcd
        uint8_t col = MAX_LENGTH_LCD - 1 - strlen(result_buffer);
        lcdGotoXY(lcd_todo, 1, col);
        lcdPrint(lcd_todo, result_buffer);
        lcdGotoXY(lcd_todo, 0, 0);
    }
    else
    {
        lcdClearDisplay(lcd_todo);
        lcdPrint(lcd_todo, "Syntax ERROR");
    }
}

void appendAction(void)
{

    // Check the alpha character first and convert if it really is
    if (is_alpha_character)
    {
        convertAlphaCharacter(pressed_key);

        // Append to the buffer with the alpha char buffer
        if(NULL == whole_chars_buffer[MAX_LENGTH_SUPPORTED - 2])
        {
            strcat(whole_chars_buffer, alpha_display_buffer);
            current_length_buffer = current_length_buffer + strlen(alpha_display_buffer);

        }
        else
            DBG("Unknown ERROR: Exceed the maximum buffer\n");


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
            //default option
        }
        // Append to the buffer with the current character (pressed key)
        if(NULL == whole_chars_buffer[MAX_LENGTH_SUPPORTED - 2])
        {
            whole_chars_buffer[strlen(whole_chars_buffer)] = pressed_key;
            current_length_buffer++;

            DBG("append action\n");
        }
        else
            DBG("Unknown ERROR: Exceed the maximum buffer\n");
    }
}

bool giveResultAction(void)
{

    // Parse the string to get operators and operands
    parseWholeBuffer(whole_chars_buffer);

    if((0U != sCaculator_->current_numberOfOperators) && (sCaculator_->current_numberOfOperators >= sCaculator_->current_numberOfOperands))
    {
        DBG("Syntax ERROR\n");
        return false;
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
                return false;
            }
            sCaculator_->operands[i+1] = powerFunc(sCaculator_->operands[i], (int64_t) sCaculator_->operands[i+1]);
            sCaculator_->operands[i] = 0.0;
            break;
        case O_ROOT:
            if(0.0 == sCaculator_->operands[i])
            {
                // Syntax ERROR, not support exponent with double type
                DBG("Syntax ERROR\n");
                return false;
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
                return false;
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
        if((sCaculator_->current_result - ((int64_t)sCaculator_->current_result)) > THRESH_HOLD)
        {
            DBG("Float\n");

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

    // Reset the buffer
    memset(whole_chars_buffer, '\0', MAX_LENGTH_SUPPORTED);
    memset(display_buffer, '\0', MAX_LENGTH_LCD);
    current_length_buffer = 0U;
    DBG("Buffer is reset\n");

    return true;

}

void clearAction(void)
{
    clearParamsCaculator(sCaculator_);
    // is_alpha_character = false;
    // alpha_character_val = ALPHA_VALUE_FALSE;

    memset(whole_chars_buffer, '\0', MAX_LENGTH_SUPPORTED);
    memset(display_buffer, '\0', MAX_LENGTH_LCD);
    current_length_buffer = 0;

    DBG("clear action\n");

}
