#include <inttypes.h>
#include <stdbool.h>

#include "clock.h"
#include "cm4_periphs.h"
#include "gpio.h"
#include "interrupts.h"
#include "stos.h"

#pragma GCC diagnostic ignored "-Wunused-variable"


static int i = 0;
void main_tick1(void) {
    while (1) {
        i++;
        if (i == 10) {
            STOS_TimeoutTask(20);
        }
        GPIO_SetHigh(GPIOA, GPIO_PIN_5);
    }
}

void main_tick2(void) {
    while (1) GPIO_SetLow(GPIOA, GPIO_PIN_5);
}

void main_tick3(void) {
    while (1) GPIO_SetLow(GPIOA, GPIO_PIN_5);
}

void main_tick4(void) {
    while (1) GPIO_SetLow(GPIOA, GPIO_PIN_5);
}

void main_tick5(void) {
    while (1) GPIO_SetLow(GPIOA, GPIO_PIN_5);
}


int main(void) {
    RCC_Enable_GPIOA_Clk();

    GPIO_SetMode(GPIOA, GPIO_PIN_5, GPIO_OUTPUT);
    Enable_Bus_Usage_Flts();

    stos_tcb_t T1 = {0};
    STOS_CreateTask(&T1, &main_tick1, 3, 40);

    stos_tcb_t T2 = {0};
    STOS_CreateTask(&T2, &main_tick2, 3, 40);

    /*

    stos_tcb_t T3 = {0};
    STOS_CreateTask(&T3, &main_tick3, 3, 40);

    stos_tcb_t T4 = {0};
    STOS_CreateTask(&T4, &main_tick4, 3, 40);

    stos_tcb_t T5 = {0};
    STOS_CreateTask(&T5, &main_tick5, 3, 40);
    */

    STOS_Init(STOS_IDLE_DEFAULT_CONFIG);
    STOS_Run();

    for (;;) {
    }
}
