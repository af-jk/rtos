#pragma once

#include "cm4_periphs.h"

#define STOS_MAX_KERNEL_EXCEP_MODIFIER (0xF)

#define SYSTICK_BASE (SCS_BASE + 0x0010UL)

typedef struct {
  volatile uint32_t CTRL;
  volatile uint32_t LOAD;
  volatile uint32_t VAL;
  volatile uint32_t CALIB;
} SYSTICK_t;

#define SYSTICK ((SYSTICK_t *) SYSTICK_BASE )

void SysTick_Config(void);

uint32_t __stos_kernel_critical_start(void);
void __stos_kernel_critical_end(uint32_t prev_basepri);