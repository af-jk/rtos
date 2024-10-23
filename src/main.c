#include <inttypes.h>
#include <stdbool.h>
#include "cm4_periphs.h"
#include "gpio.h"
#include "clock.h"
#include "interrupts.h"
#include "stos.h"

void delay(volatile uint32_t delayTime) {
    while (delayTime--);
}

/*
void __attribute__((naked)) usage_flt_handler (void) {
    while (1);
}
*/

void sys_tick_handler(void) {
    STOS_Schedule();
    volatile uint32_t *invalid_address = (uint32_t *)0xFFFFFFF0;  // Invalid address
    *invalid_address = 0x12345678;  // Writing to the invalid address
}

uint32_t t1_stack[40];
void main_ledOn(void) {
    while (1) GPIO_SetHigh(GPIOA, GPIO_PIN_5);
}

uint32_t t2_stack[40];
void main_ledOff(void) {
    while (1) GPIO_SetLow(GPIOA, GPIO_PIN_5);
}

void __attribute__((naked)) pend_sv_handler(void) {
    while (1);
}

int main(void) {
    RCC_Enable_GPIOA_Clk();

    GPIO_SetMode(GPIOA, GPIO_PIN_5, GPIO_OUTPUT);

    SYSTICK_Config();

    STOS_Init();

    stos_tcb_t T1;
    STOS_CreateTask(&T1,
                    &main_ledOn,
                    &t1_stack,
                    sizeof(t1_stack));

    stos_tcb_t T2;
    STOS_CreateTask(&T2,
                    &main_ledOff,
                    &t2_stack,
                    sizeof(t2_stack));

    SCB->ICSR |= (1U << 26);

    // STOS_Schedule();
    // STOS_Run();

    // Loop to blink LED
    while (1) {
       // delay(500000);  // Delay
    }
}