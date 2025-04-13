#pragma once

#include "cm4_periphs.h"

#define STOS_MAX_KERNEL_EXCEP_MODIFIER (0xF)

#define SCB_ICSR_PENDSVSET_Pos          (28U)
#define SCB_ICSR_PENDSVSET_Msk          (1UL << SCB_ICSR_PENDSVSET_Pos) 

#define SYSTICK_BASE (SCS_BASE + 0x0010UL)

typedef struct {
  volatile uint32_t CTRL;
  volatile uint32_t LOAD;
  volatile uint32_t VAL;
  volatile uint32_t CALIB;
} SYSTICK_t;

#define SYSTICK ((SYSTICK_t *) SYSTICK_BASE )

void SysTick_Config(void);
void PendSV_Set(void);