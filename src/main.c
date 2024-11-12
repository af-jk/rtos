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

void usage_flt_handler(void) {
    while (1);
}

void bus_flt_handler(void) {
    while (1);
}

void sys_tick_handler(void) {
    STOS_Schedule();
}

void main_ledOn(void) {
    while (1) GPIO_SetHigh(GPIOA, GPIO_PIN_5);

}

void main_ledOff(void) {
	while (1) GPIO_SetLow(GPIOA, GPIO_PIN_5);
}

void svc_handler(void) {
    __asm volatile(
        "MOV LR, #0xFFFFFFFD \n"
        "BX LR \n"              
    );
}

int main(void) {
    RCC_Enable_GPIOA_Clk();

    GPIO_SetMode(GPIOA, GPIO_PIN_5, GPIO_OUTPUT);

    // Enable UsageFault and BusFault exceptions
    SCB->SHCSR |= (1 << 17) | (1 << 18);

    stos_tcb_t T1 = {0};
    STOS_CreateTask(&T1,
                    &main_ledOn,
                    40);

    stos_tcb_t T2 = {0};
    STOS_CreateTask(&T2,
                    &main_ledOff,
                    40);
    STOS_Init();
    STOS_Run();

    for (;;) {}
}
