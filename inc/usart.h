#pragma once

#include "cm4_periphs.h"

#define USART2_BASE (APB1PERIPH_BASE + 0x00004400UL)

/**
 * @brief USART port
 */

typedef struct {
    volatile uint32_t USART_SR;
    volatile uint32_t USART_DR;
    volatile uint32_t USART_BRR;
    volatile uint32_t USART_CR1;
    volatile uint32_t USART_CR2;
    volatile uint32_t USART_CR3;
    volatile uint32_t USART_GTPR;
} USART_t;

typedef struct USART_handle_s{
    USART_t *port;
} USART_handle_t;

#define USART2 ((USART_t *)USART2_BASE)

/** SR Register Bit Defines **/
#define USART_SR_PE      (0x1UL) << (0U)
#define USART_SR_FE      (0x1UL) << (1U)
#define USART_SR_NF      (0x1UL) << (2U)
#define USART_SR_ORE     (0x1UL) << (3U)
#define USART_SR_IDLE    (0x1UL) << (4U)
#define USART_SR_RXNE    (0x1UL) << (5U)
#define USART_SR_TC      (0x1UL) << (6U)
#define USART_SR_TXE     (0x1UL) << (7U)
#define USART_SR_LBD     (0x1UL) << (8U)
#define USART_SR_CTS     (0x1UL) << (9U)

/** CR1 Register Bit Defines **/
#define USART_CR1_SBK    (0x1UL) << (0U)
#define USART_CR1_RWU    (0x1UL) << (1U)
#define USART_CR1_RE     (0x1UL) << (2U)
#define USART_CR1_TE     (0x1UL) << (3U)
#define USART_CR1_IDLEIE (0x1UL) << (4U)
#define USART_CR1_RXNEIE (0x1UL) << (5U)
#define USART_CR1_TCIE   (0x1UL) << (6U)
#define USART_CR1_PEIE   (0x1UL) << (7U)
#define USART_CR1_PS     (0x1UL) << (8U)
#define USART_CR1_PCE    (0x1UL) << (9U)
#define USART_CR1_WAKE   (0x1UL) << (10U)
#define USART_CR1_M      (0x1UL) << (11U)
#define USART_CR1_UE     (0x1UL) << (12U)
#define USART_CR1_OVER8  (0x1UL) << (13U)


void USART_init(USART_t *port,uint32_t baudrate);
