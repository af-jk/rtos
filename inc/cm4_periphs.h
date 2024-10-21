#pragma once

#include <stdint.h>

/*
    Defines the Cortex-M4 standard peripherals
*/

// Going to copy ARM's way for now

// "Internal" Peripherals
#define SCB_BASE     (0xE000E000UL)

// "External Peripherals"
#define PERIPH_BASE     (0x40000000UL)
#define APB1PERIPH_BASE (PERIPH_BASE)
#define AHB1PERIPH_BASE (PERIPH_BASE + 0x00020000UL)

typedef struct {
  volatile uint32_t CPUID;
  volatile uint32_t ICSR;
  volatile uint32_t VTOR;
  volatile uint32_t AIRCR;
  volatile uint32_t SCR;
  volatile uint32_t CCR;
  volatile uint8_t SHP[12U];
  volatile uint32_t SHCSR;
  volatile uint32_t CFSR;
  volatile uint32_t HFSR;
  volatile uint32_t DFSR;
  volatile uint32_t MMFAR;
  volatile uint32_t BFAR;
  volatile uint32_t AFSR;
  volatile uint32_t PFR[2U];
  volatile uint32_t DFR;
  volatile uint32_t ADR;
  volatile uint32_t MMFR[4U];
  volatile uint32_t ISAR[5U];
  volatile uint32_t RESERVED0[5U];
  volatile uint32_t CPACR;
} SCB_t;

#define SCB     ((SCB_t     *) SCB_BASE     )