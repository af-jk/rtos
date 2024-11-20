#pragma once

#include "cm4_periphs.h"

#define USART2_BASE (APB1PERIPH_BASE + 0x00004400UL)

typedef struct {
    volatile uint32_t USART_SR;
    volatile uint32_t USART_DR;
    volatile uint32_t USART_BRR;
    volatile uint32_t USART_CR1;
    volatile uint32_t USART_CR2;
    volatile uint32_t USART_CR3;
    volatile uint32_t USART_GTPR;
} USART_t;

#define USART2 ((USART_t *)USART2_BASE);
