#include <inttypes.h>
#include <stdbool.h>
#include "cm4_periphs.h"

#define CORE_FREQ 16000000UL

// Delay function
void delay(volatile uint32_t delayTime) {
    while (delayTime--);
}

int main(void) {
    // Enable GPIOA clock
    RCC->AHB1ENR |= (1UL);
    
    // Set GPIOA Pin 5 as output (MODER register)
    GPIOA->MODER &= ~(3UL << (5 * 2));  // Clear mode bits for PA5
    GPIOA->MODER |= (1UL << (5 * 2));   // Set mode to 01 (General purpose output)

    // Set GPIOA Pin 5 to push-pull mode (OTYPER register)
    GPIOA->OTYPER &= ~(1UL << 5);  // 0 means push-pull

    // Set GPIOA Pin 5 to no pull-up/pull-down (PUPDR register)
    GPIOA->PUPDR &= ~(3UL << (5 * 2));  // 00 means no pull-up/pull-down

    init_systick();

    // Loop to blink LED
    while (1) {
        delay(500000);  // Delay
    }
}