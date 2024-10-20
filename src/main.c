#include <inttypes.h>
#include <stdbool.h>
#include "cm4_periphs.h"
#include "gpio.h"
#include "clock.h"
#include "interrupts.h"

void delay(volatile uint32_t delayTime) {
    while (delayTime--);
}

void sys_tick_handler(void) {
        GPIO_Toggle(GPIOA, GPIO_PIN_5);
}

int main(void) {
    RCC_Enable_GPIOA_Clk();

    GPIO_SetMode(GPIOA, GPIO_PIN_5, GPIO_OUTPUT);

    SYSTICK_Config();

    // Loop to blink LED
    while (1) {
       // delay(500000);  // Delay
    }
}