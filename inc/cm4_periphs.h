#pragma once

#include <stdint.h>

/*
    Defines the Cortex-M4 standard peripherals
*/

// Going to copy ARM's way for now

// "Internal" Peripherals
#define SCS_BASE (0xE000E000UL)
#define SysTick_BASE (SCS_BASE + 0x0010UL)
#define SCB_BASE (SCS_BASE + 0x0D00UL)

// "External Peripherals"
#define PERIPH_BASE (0x40000000UL)
#define APB1PERIPH_BASE (PERIPH_BASE)
#define AHB1PERIPH_BASE (PERIPH_BASE + 0x00020000UL)

#define GPIOA_BASE (AHB1PERIPH_BASE + 0x0000UL)
#define GPIOB_BASE (AHB1PERIPH_BASE + 0x0400UL)
#define GPIOC_BASE (AHB1PERIPH_BASE + 0x0800UL)
#define GPIOH_BASE (AHB1PERIPH_BASE + 0x1C00UL)
#define CRC_BASE (AHB1PERIPH_BASE + 0x3000UL)
#define RCC_BASE (AHB1PERIPH_BASE + 0x3800UL)

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
} SCB_Type_t;

typedef struct {
  volatile uint32_t CTRL;
  volatile uint32_t LOAD;
  volatile uint32_t VAL;
  volatile uint32_t CALIB;
} SysTick_Type_t;

typedef struct {
  volatile uint32_t MODER;
  volatile uint32_t OTYPER;
  volatile uint32_t OSPEEDR;
  volatile uint32_t PUPDR;
  volatile uint32_t IDR;
  volatile uint32_t ODR;
  volatile uint32_t BSRR;
  volatile uint32_t LCKR;
  volatile uint32_t AFR[2];
} GPIO_Type_t;

typedef struct {
  volatile uint32_t CR;
  volatile uint32_t PLLCFGR;
  volatile uint32_t CFGR;
  volatile uint32_t CIR;
  volatile uint32_t AHB1RSTR;
  volatile uint32_t RESERVED0[3];
  volatile uint32_t APB1RSTR;
  volatile uint32_t APB2RSTR;
  volatile uint32_t RESERVED1[2];
  volatile uint32_t AHB1ENR;
  volatile uint32_t RESERVED2[3];
  volatile uint32_t APB1ENR;
  volatile uint32_t APB2ENR;
  volatile uint32_t RESERVED3[2];
  volatile uint32_t AHB1LPENR;
  volatile uint32_t RESERVED4[3];
  volatile uint32_t APB1LPENR;
  volatile uint32_t APB2LPENR;
  volatile uint32_t RESERVED5[2];
  volatile uint32_t BDCR;
  volatile uint32_t CSR;
  volatile uint32_t RESERVED6[2];
  volatile uint32_t SSCGR;
  volatile uint32_t RESERVED7[2];
  volatile uint32_t DCKCFGR;
  volatile uint32_t CKGATENR;
  volatile uint32_t DCKCFGR2;
} RCC_Type_t;

#define SCB ((SCB_Type_t *)SCB_BASE)
#define SysTick ((SysTick_Type_t *)SysTick_BASE)
#define GPIOA ((GPIO_Type_t *)GPIOA_BASE)
#define RCC ((RCC_Type_t *)RCC_BASE)

// void init(void) {
//   // Assuming FPU is present, enable FPU
//   // SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));  /* set CP10 and CP11 Full
//   // Access */
// 
//   // Tell VTOR where the vector table is?
//   // Why would we need to update this if it's already able to find the start?
//   // SCB->VTOR = VECT_TAB_BASE_ADDRESS | VECT_TAB_OFFSET; /* Vector Table
//   // Relocation in Internal SRAM */
// 
//   // Is there anything else that *needs* to be initialized now?
// }

int sum(int a, int b);