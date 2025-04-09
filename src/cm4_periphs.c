#include "cm4_periphs.h"

void SysTick_SetPri(uint8_t pri) {
    /*
    SHP[8]
    
      SysTick(W)    (R)
    [ -------- ][ ------- ]
    31 30 29 28 27 26 25 24
    */

    uint8_t mask = 0xFF;
    // Clear SysTick bits
    mask ^= 0xF0U;
    SCB->SHP[11] &= mask;

    // Mask first for bits of pri (priority can only be 0-15)
    SCB->SHP[11] |= (uint8_t)((pri & mask) << 4U);
}

void PendSV_SetPri(uint8_t pri) {
    /*
    SHP[9]
    
      PendSV (W)    (R)
    [ -------- ][ ------- ]
    31 30 29 28 27 26 25 24
    */
    uint8_t mask = 0xFF;
    // Clear PendSV bits
    mask ^= 0xF0U;
    SCB->SHP[10] &= mask;

    // Mask first for bits of pri (priority can only be 0-15)
    SCB->SHP[10] |= (uint8_t)((pri & mask) << 4U);
}

void usage_flt_handler(void) {
    while (1)
        ;
}

void bus_flt_handler(void) {
    while (1)
        ;
}

void Enable_Bus_Usage_Flts(void) {
    // Enable UsageFault and BusFault exceptions
    SCB->SHCSR |= (1 << 17) | (1 << 18);
}
