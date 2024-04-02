/*
 * caculator_api.h
 *
 *  Created on: Feb 3, 2024
 *      Author: trieu
 */

#ifndef MY_LIBS_INC_CACULATOR_API_H_
#define MY_LIBS_INC_CACULATOR_API_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_OPERANDS            10U
#define MAX_OPERATORS           9U
#define POSITIVE_THRESH_HOLD    0.00000001F
#define NEGATIVE_THRESH_HOLD    -0.00000001F
#define MAX_SUPPORT_ELEMENTS    10U
#define MAX_BUFFER_LENGTH       100U

struct find_equation_parms
{
    double coefficient_x2;
    double coefficient_x1;
    double coefficient_x0;
};

typedef enum
{
    SAME_ROOT,
    DIFF_ROOT,
    NO_ROOT
} eResultType_t;

typedef enum {
    O_ADD,
    O_SUBTRACT,
    O_MULTIPLY,
    O_DIVIDE,
    O_SIN,
    O_COS,
    O_LOG,
    O_DOUBLE,
    O_POWER,
    O_EQUATION,
    O_ROOT,
    O_ANS,
    O_NONE,
} operator_t;

typedef enum {
    IS_OPERATOR,
    IS_OPERAND,
} typed_char_t;

typedef struct {
    bool is_new_operand;
    uint8_t current_numberOfOperands;
    uint8_t current_numberOfOperators;
    double current_result;
    double operands[MAX_OPERANDS];
    operator_t operators[MAX_OPERATORS];
    char whole_buffer[MAX_BUFFER_LENGTH];
} caculator_t;

extern double result_x1;
extern double result_x2;
extern caculator_t *sCaculator_;
extern caculator_t sCaculator_list[MAX_SUPPORT_ELEMENTS];
extern uint8_t current_caculator_element;
extern void initParamsCaculator(caculator_t *);
extern void clearParamsCaculator(caculator_t *);
extern double sineFunc(double);
extern double cosineFunc(double);
extern double logFunc(double, double);
extern double powerFunc(double, int64_t);
extern double sumAllOperands(caculator_t *);
extern eResultType_t solveQuadraticEquation(double, double, double);

#endif /* MY_LIBS_INC_CACULATOR_API_H_ */
