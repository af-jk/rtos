#pragma once

#include "cm4_periphs.h"

#define CORE_FREQ 16000000UL

#define NVIC_BASE    (SCS_BASE + 0x0100UL)
#define SYSTICK_BASE (SCS_BASE + 0x0010UL)

typedef struct {
    volatile uint32_t ISER[8U];
    uint32_t RESERVED0[24U];
    volatile uint32_t ICER[8U];
    uint32_t RESERVED1[24U];
    volatile uint32_t ISPR[8U];
    uint32_t RESERVED2[24U];
    volatile uint32_t ICPR[8U];
    uint32_t RESERVED3[24U];
    volatile uint32_t IABR[8];
    uint32_t RESERVED4[56U];
    volatile uint32_t IPR[60U];
    uint32_t RESERVED5[644U];
    volatile uint32_t STIR;
} NVIC_t;

typedef struct {
  volatile uint32_t CTRL;
  volatile uint32_t LOAD;
  volatile uint32_t VAL;
  volatile uint32_t CALIB;
} SYSTICK_t;

#define NVIC    ((NVIC_t    *) NVIC_BASE    )
#define SYSTICK ((SYSTICK_t *) SYSTICK_BASE )

void NVIC_EnableIRQ(uint8_t irqn);
void NVIC_DisableIRQ(uint8_t irqn);
void NVIC_SetPri(uint8_t irqn, uint8_t pri);
void SYSTICK_Config(void);