#include "clock.h"

void RCC_Enable_GPIOA_Clk(void) {
    RCC->AHB1ENR |= (1UL);
}

void RCC_Enable_USART2_Clk(void) {
    RCC->APB1ENR |= 1 << 17;
}
