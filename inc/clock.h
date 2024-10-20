#pragma once

#include "cm4_periphs.h"

#define CORE_FREQ 16000000UL

#define RCC_BASE   (AHB1PERIPH_BASE + 0x3800UL)

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
} RCC_t;

#define RCC     ((RCC_t     *) RCC_BASE     )

void RCC_Enable_GPIOA_Clk(void);