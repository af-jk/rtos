#include "interrupts.h"
#include "clock.h"

void NVIC_EnableIRQ(IRQn_t irqn) {
    if ((int8_t)irqn >= 0)
    {
        // 0-31, 32-63 ... 239 (each is 5U apart)
        uint8_t idx     = (uint8_t)irqn >> 5U;
        uint8_t shamt   = irqn & 0x1FU;  // (irqn >> 31)
        NVIC->ISER[idx] = (1 << shamt);
    }
}

void NVIC_DisableIRQ(IRQn_t irqn) {
    if ((int8_t)irqn >= 0)
    {
        uint8_t idx     = (uint8_t)irqn >> 5U;
        uint8_t shamt   = irqn & 0x1FU;
        NVIC->ICER[idx] = (1 << shamt);
    }
}

// Need to handle negative priority case
void NVIC_SetPri(IRQn_t irqn, uint8_t pri) {
    if ((int8_t)irqn >= 0)
    {
        uint8_t idx    = (uint8_t)irqn >> 2U; // Map 0-239 to 0-59
        uint8_t shamt  = ((uint8_t)irqn & 3U) << 3U;
        // Do we need to worry about this being atomic?
        // There's an instance here where our priority may be 0
        NVIC->IPR[idx] &= ~(0xFFU << shamt); // Clear the existing priority
        NVIC->IPR[idx] |= (pri & 0xFFU) << shamt; // Set only the new priority bits and or to not change the other 0-3
    }
}

/*
__STATIC_INLINE void __NVIC_SetPriority(IRQn_Type IRQn, uint32_t priority)
{
  if ((int32_t)(IRQn) >= 0)
  {
    NVIC->IP[((uint32_t)IRQn)]               = (uint8_t)((priority << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
  }
  else
  {
    SCB->SHP[(((uint32_t)IRQn) & 0xFUL)-4UL] = (uint8_t)((priority << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
  }
}
*/

void SYSTICK_Config(void) {
    /*  SYSTICK CTRL: 31 - 0
        [0] -> Enable (counter loads reload to load and counts down)
        [1] -> Tick Int (counting down to zero asserts exception request)
        [2] -> Clk Source (AHB/8 or Processor clock (AHB) as clock source)
    */
    SYSTICK->CTRL &= ~(1UL << 0); // Don't enable
    SYSTICK->CTRL |= 1UL << 1; // Enable exception
    SYSTICK->CTRL |= 1UL << 2; // Select processor clock

    /*  SYSTICK LOAD: 23 - 0
        [23:0] -> RELOAD Value
        If we want 1ms ticks, set to the mxaimum timer count value 15.9e3
    */
   SYSTICK->LOAD &= ~(0xFFFFFFUL); // Clear LOAD value
   SYSTICK->LOAD |= (CORE_FREQ/10) - 1; // Set the LOAD value

    /*  SYSTICK LOAD: 23 - 0
        [23:0] -> RELOAD Value
        If we want 1ms ticks, set to the mxaimum timer count value 15.9e3
    */
   SYSTICK->VAL &= ~(0xFFFFFFUL); // Clear VAL value= 0;

   SYSTICK->CTRL |= 1UL; // Enable SYSTICK
}