#include "interrupts.h"

#include "clock.h"

void SysTick_Config(void) {
    /*  SYSTICK CTRL: 31 - 0
        [0] -> Enable (counter loads reload to load and counts down)
        [1] -> Tick Int (counting down to zero asserts exception request)
        [2] -> Clk Source (AHB/8 or Processor clock (AHB) as clock source)
    */
    SYSTICK->CTRL &= ~(1UL << 0);  // Don't enable
    SYSTICK->CTRL |= 1UL << 1;     // Enable exception
    SYSTICK->CTRL |= 1UL << 2;     // Select processor clock

    /*  SYSTICK LOAD: 23 - 0
        [23:0] -> RELOAD Value
        If we want 1ms ticks, set to the mxaimum timer count value 15.9e3
    */
    SYSTICK->LOAD |= (CORE_FREQ / 10) - 1;  // Set the LOAD value

    SYSTICK->VAL &= ~(0xFFFFFFUL);  // Clear VAL value= 0;

    SYSTICK->CTRL |= 1UL;  // Enable SYSTICK
}