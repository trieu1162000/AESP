/*
 * caculator_api.c
 *
 *  Created on: Feb 3, 2024
 *      Author: trieu
 */
#include "../inc/caculator_api.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

double result_x1 = 0.0;
double result_x2 = 0.0;

double sumAllOperands(caculator_t *calc)
{
    double sum = 0.0;
    uint8_t i = 0U;
    for(; i < calc->current_numberOfOperands; i++)
    {
        sum += calc->operands[i];
    }
    return sum;
}

eResultType_t solveQuadraticEquation(double a, double b, double c)
{
    if (0.0 == a) {
        result_x1 = -c / b;
        return SAME_ROOT;
    }

    // Calculate the discriminant
    double discriminant = b * b - 4.0 * a * c;
    
    // Check if the discriminant is non-negative
    if (discriminant >= 0.0) {
        // Calculate the square root of the discriminant
        double sqrt_discriminant = sqrt(discriminant);
        
        // Calculate the two roots
         result_x1 = (-b + sqrt_discriminant) / (2.0 * a);
         result_x2 = (-b - sqrt_discriminant) / (2.0 * a);
        
        // Print the roots
        // DBG("The roots are: %f and %f\n", result_x1, result_x2);
        return DIFF_ROOT;
    }
    else
    {
        // If the discriminant is negative, print no real roots
        // DBG("The equation has no real roots.\n");
        return NO_ROOT;
    }
}

double sineFunc(double x_angle) {
    double x_radian = x_angle * M_PI / 180.0; // Convert angle to radian
    double result = 0.0;
    double term = x_radian;
    uint8_t i = 0U;
    int32_t sign = 1;

    for (i = 1U; i <= 10U; i += 2U) {
        result += ( (double) sign ) * term;
        sign *= -1;
        term *= (x_radian * x_radian) / ((i + 1) * (i + 2));
    }

    return result;
}

double cosineFunc(double x_angle) {
    double x_radian = x_angle * M_PI / 180.0; // Convert angle to radian
    double result = 1.0; // First term of the series
    double term = 1.0; // Initialize the current term
    uint8_t i = 0U;
    int32_t sign = -1;

    for (i = 2U; i <= 10U; i += 2U) {
        term *= (x_radian * x_radian) / ((i - 1U) * i); // Calculate the next term
        result += ( (double) sign ) * term; // Add the term to the result
        sign *= -1; // Change the sign for the next term
    }

    return result;
}

double logFunc(double base_val, double calc_val) {
    return log(calc_val) / log(base_val);
}

double powerFunc(double base, int64_t exponent) {
    double result = 1.0;
    int64_t i = 0;

    // Handling negative exponents
    if (0 > exponent) {
        base = 1.0 / base;
        exponent = -exponent;
    }

    // Compute the result by multiplying base 'exponent' times
    for (i = 0; i < exponent; i++) {
        result *= base;
    }

    return result;
}

void clearParamsCaculator(caculator_t *calc)
{
    calc->current_numberOfOperands = 0U;
    calc->current_numberOfOperators = 0U;
    calc->is_new_operand = true;
    memset(calc->operands, 0.0, sizeof(double) * MAX_OPERANDS);
    memset(calc->operators, O_NONE, sizeof(operator_t) * MAX_OPERATORS);
}

void initParamsCaculator(caculator_t *calc)
{
    calc->current_result = 0.0;
    memset(calc->whole_buffer, '\0', MAX_BUFFER_LENGTH);
    clearParamsCaculator(calc);
}
