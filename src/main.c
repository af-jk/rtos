#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "clock.h"
#include "cm4_periphs.h"
#include "gpio.h"
#include "interrupts.h"
#include "sync.h"
#include "stos.h"
#include "usart.h"

#pragma GCC diagnostic ignored "-Wunused-variable"

void stos_task_1(void) {
	volatile uint32_t arr[10] = {0xFFFFFFFF,
                                 0xFFFFFFFF,
                                 0xFFFFFFFF,
                                 0xFFFFFFFF,
                                 0xFFFFFFFF,
                                 0xFFFFFFFF,
                                 0xFFFFFFFF,
                                 0xFFFFFFFF,
                                 0xFFFFFFFF,
                                 0xFFFFFFFF};
	while (true) {
		arr[0]--;
		continue;
	}
}

void stos_task_2(void) {
	volatile uint32_t arr[10] = {0xFFFFFFFF,
                                 0xFFFFFFFF,
                                 0xFFFFFFFF,
                                 0xFFFFFFFF,
                                 0xFFFFFFFF,
                                 0xFFFFFFFF,
                                 0xFFFFFFFF,
                                 0xFFFFFFFF,
                                 0xFFFFFFFF,
                                 0xFFFFFFFF};
	while (true) {
		arr[0]--;
		continue;
	}
}

int main(void) {
    RCC_Enable_GPIOA_Clk();

    USART_init(USART2,115200);
    printf("DEBUG: RUNNING STOS RTOS\r\n");
    printf("------------------------\r\n");


    GPIO_SetMode(GPIOA, GPIO_PIN_5, GPIO_OUTPUT);
    Enable_Bus_Usage_Flts();

    stos_tcb_t T1 = {0};
    STOS_CreateTask(&T1, &stos_task_1, 4, 4);

    stos_tcb_t T2 = {0};
    STOS_CreateTask(&T2, &stos_task_2, 3, 4);

    STOS_Init(STOS_IDLE_DEFAULT_CONFIG);
    STOS_Run();

    for (;;) {
    }
}

int _write(int fd, char *ptr, uint32_t len) {
    (void) fd;
    if (fd == 1) {
        USART_transmit(USART2,(uint8_t *) ptr, len);
    }
    return (int)len;
 }
