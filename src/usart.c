#include "usart.h"
#include "gpio.h"
#include "clock.h"

/**
 * Procedure:
 * 1. Enable the USART by writing the UE bit in USART_CR1 register to 1.
 * 2. Program the M bit in USART_CR1 to define the word length.
 * 3. Program the number of stop bits in USART_CR2.
 * 4. Select DMA enable (DMAT) in USART_CR3 if Multi buffer Communication is to take
 * place. Configure the DMA register as explained in multibuffer communication.
 * 5. Select the desired baud rate using the USART_BRR register.
 * 6. Set the TE bit in USART_CR1 to send an idle frame as first transmission.
 * 7. Write the data to send in the USART_DR register (this clears the TXE bit). Repeat this
 * for each data to be transmitted in case of single buffer.
 * 8. After writing the last data into the USART_DR register, wait until TC=1. This indicates
 * that the transmission of the last frame is complete. This is required for instance when
 * the USART is disabled or enters the Halt mode to avoid corrupting the last
 * transmission.
 */

void USART_init(USART_t *port, uint32_t baudrate)
{
    RCC_Enable_USART2_Clk();

    // Must enable PA2 & PA3
    GPIO_PIN_t tx = GPIO_PIN_2;
    GPIO_PIN_t rx = GPIO_PIN_3;

    GPIO_SetMode(GPIOA,tx,GPIO_AF);
    GPIO_SetAF(GPIOA,tx,GPIO_AF_7);

    GPIO_SetMode(GPIOA,rx,GPIO_AF);
    GPIO_SetAF(GPIOA,rx,GPIO_AF_7);

    port->USART_BRR = (uint32_t) (CORE_FREQ / (16 * baudrate));

    port->USART_CR1 |= (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE);
}

void USART_transmit()
{

}
