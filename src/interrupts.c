#include "interrupts.h"

void NVIC_EnableIRQ(uint8_t irqn) {
    // 0-31, 32-63 ... 239 (each is 5U apart)
    uint8_t idx     = irqn >> 5U;
    uint8_t shamt   = irqn & 0x1FU;  // (irqn >> 31)
    NVIC->ISER[idx] = (1 << shamt);
}

void NVIC_DisableIRQ(uint8_t irqn) {
    uint8_t idx     = irqn >> 5U;
    uint8_t shamt   = irqn & 0x1FU;
    NVIC->ICER[idx] = (1 << shamt);
}

void NVIC_SetPri(uint8_t irqn, uint8_t pri) {
    uint8_t idx    = irqn >> 2U; // Map 0-239 to 0-59
    uint8_t shamt  = (irqn & 3U) << 3U;
    NVIC->IPR[idx] &= ((~0xFF) & pri << shamt);  
    NVIC->IPR[idx] = NVIC->IPR[idx] & ~(0xFFUL << )
}