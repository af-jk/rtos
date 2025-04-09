#pragma once

#include "cm4_periphs.h"

#define IRQ_MIN_PRI (15U)

#define SYSTICK_BASE (SCS_BASE + 0x0010UL)

typedef struct {
  volatile uint32_t CTRL;
  volatile uint32_t LOAD;
  volatile uint32_t VAL;
  volatile uint32_t CALIB;
} SYSTICK_t;

#define SYSTICK ((SYSTICK_t *) SYSTICK_BASE )

void SysTick_Config();