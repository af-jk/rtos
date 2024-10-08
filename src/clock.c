#include "clock.h"

void RCC_EnableGPIOAClk(void) {
    RCC->AHB1ENR |= (1UL);
}
