#include "gpio.h"

void GPIO_SetMode(GPIO_t *port, GPIO_PIN_t pin, GPIO_MODE_t mode) {
    port->MODER &= ~(0x3U << ((uint8_t)pin << 1));
    port->MODER |= (((uint8_t)mode) << (pin << 1));
}

void GPIO_SetAF(GPIO_t *port, GPIO_PIN_t pin, GPIO_AF_t af)
{
    port->AFR[pin >> 3] &= ~(0xFU << (((uint8_t)pin & 0x7U) * 4));
    port->AFR[pin >> 3] |= ((uint32_t)af) << (((uint8_t)pin & 0x7U) * 4);
}

void GPIO_SetHigh(GPIO_t *port, GPIO_PIN_t pin) {
    port->ODR |= (1U << (uint8_t)pin);
}

void GPIO_SetLow(GPIO_t *port, GPIO_PIN_t pin) {
    port->ODR &= ~(1U << (uint8_t)pin);
}

void GPIO_Toggle(GPIO_t *port, GPIO_PIN_t pin) {
    port->ODR ^= (1U << (uint8_t)pin);
}
