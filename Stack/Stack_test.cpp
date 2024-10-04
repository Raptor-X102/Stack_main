#include <TXLib.h>
#include <stdio.h>
#include "Stack_definitions.h"
#include "Stack_Functions.h"
#include "Stack_Debug_Functions.h"
void Printf_Format_Function(const void * value);

int main() {
    const double poison_val = -6.66;

    Stack_t stack = {};
    STACK_CTOR(NULL, 0, sizeof(double), &poison_val);
    STACK_DUMP(&stack, Printf_Format_Function);

    printf("1\n");
    STACK_ASSERT(&stack);

    printf("2\n");
    STACK_DUMP(&stack, Printf_Format_Function);

    double val = 10;
    double ret_val = 0;
    StackPush(&stack, &val);
    val = 20;
    StackPush(&stack, &val);
    val = 30;
    StackPush(&stack, &val);
    val = 40;
    StackPush(&stack, &val);
    val = 50;
    StackPush(&stack, &val);
    val = 60;
    StackPush(&stack, &val);
    val = 80;
    StackPush(&stack, &val);
    val = 90;
    StackPush(&stack, &val);
    val = 100;
    StackPop(&stack, &ret_val);
    StackPush(&stack, &val);
    val = 110;
    StackPush(&stack, &val);


    StackPop(&stack, &ret_val);
    StackPop(&stack, &ret_val);
    StackPop(&stack, &ret_val);
    StackPop(&stack, &ret_val);
    StackPop(&stack, &ret_val);
    StackPop(&stack, &ret_val);
    StackPop(&stack, &ret_val);
    StackPop(&stack, &ret_val);
    StackPop(&stack, &ret_val);
    StackPop(&stack, &ret_val);
    StackPop(&stack, &ret_val);

    STACK_ASSERT(&stack);
    STACK_DUMP(&stack, Printf_Format_Function);
    StackDtor(&stack);
    printf("Free() is used in 'StackAssert' in main\n");
    return 0;
}

void Printf_Format_Function(const void * value) {

    double tmp = 0;
    memcpy(&tmp, value, sizeof(double));
    printf("%lg", tmp);
}
