#include "clock.h"

void RCC_Enable_GPIOA_Clk(void) { RCC->AHB1ENR |= (1UL); }
