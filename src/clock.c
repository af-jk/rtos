#include "clock.h"

void init_systick(void) {
    /*  SysTick CTRL: 31 - 0
        [0] -> Enable (counter loads reload to load and counts down)
        [1] -> Tick Int (counting down to zero asserts exception request)
        [2] -> Clk Source (AHB/8 or Processor clock (AHB) as clock source)
    */
    SysTick->CTRL &= ~(1UL << 0); // Don't enable
    SysTick->CTRL |= 1UL << 1; // Enable exception
    SysTick->CTRL |= 1UL << 2; // Select processor clock

    /*  SysTick LOAD: 23 - 0
        [23:0] -> RELOAD Value
        If we want 1ms ticks, set to the mxaimum timer count value 15.9e3
    */
   SysTick->LOAD &= ~(0xFFFFFFUL); // Clear LOAD value
   SysTick->LOAD |= (CORE_FREQ/10) - 1; // Set the LOAD value

    /*  SysTick LOAD: 23 - 0
        [23:0] -> RELOAD Value
        If we want 1ms ticks, set to the mxaimum timer count value 15.9e3
    */
   SysTick->VAL &= ~(0xFFFFFFUL); // Clear VAL value= 0;

   SysTick->CTRL |= 1UL; // Enable SysTick
}

void sys_tick_handler(void) {
        // Toggle PA5
        GPIOA->ODR ^= (1UL << 5);
}