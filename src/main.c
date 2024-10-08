#include <inttypes.h>
#include <stdbool.h>
#include "cm4_periphs.h"
#include "interrupts.h"
#include "gpio.h"
#include "clock.h"

void delay(volatile uint32_t delayTime) {
    while (delayTime--);
}

void sys_tick_handler(void) {
        GPIO_Toggle(GPIOA, GPIO_PIN_5);
}

int main(void) {
    RCC_EnableGPIOAClk();

    GPIO_SetMode(GPIOA, GPIO_PIN_5, GPIO_OUTPUT);
    
    SYSTICK_Config();

    while (1) {
        delay(500000);
    }
}