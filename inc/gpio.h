#pragma once

#include "cm4_periphs.h"

#define GPIOA_BASE (AHB1PERIPH_BASE + 0x0000UL)
#define GPIOB_BASE (AHB1PERIPH_BASE + 0x0400UL)
#define GPIOC_BASE (AHB1PERIPH_BASE + 0x0800UL)
#define GPIOH_BASE (AHB1PERIPH_BASE + 0x1C00UL)

typedef enum {
    GPIO_INPUT,
    GPIO_OUTPUT,
    GPIO_ALTERNATE,
    GPIO_ANALOG,
} GPIO_MODE_t;

typedef enum {
    GPIO_PIN_0,
    GPIO_PIN_1,
    GPIO_PIN_2,
    GPIO_PIN_3,
    GPIO_PIN_4,
    GPIO_PIN_5,
    GPIO_PIN_6,
    GPIO_PIN_7,
    GPIO_PIN_8,
    GPIO_PIN_9,
    GPIO_PIN_10,
    GPIO_PIN_11,
    GPIO_PIN_12,
    GPIO_PIN_13,
    GPIO_PIN_14,
    GPIO_PIN_15,
} GPIO_PIN_t;

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
} GPIO_t;

#define GPIOA   ((GPIO_t    *) GPIOA_BASE   )

void GPIO_SetMode(GPIO_t *port, GPIO_PIN_t pin, GPIO_MODE_t mode);
void GPIO_SetHigh(GPIO_t *port, GPIO_PIN_t pin);
void GPIO_SetLow(GPIO_t *port, GPIO_PIN_t pin);
void GPIO_Toggle(GPIO_t *port, GPIO_PIN_t pin);