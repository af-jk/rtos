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

stos_mutex_t mutex;

stos_tcb_t T1;
void stos_task_1(void) {
    STOS_MutexLock(&mutex, &T1, STOS_MUTEX_WAIT_NONE);
    static int i = 0;
	while (true) {
        //GPIO_SetLow(GPIOA, GPIO_PIN_5);
        i++;
        printf("Task 1\r\n");
        if (i > 10) {
            STOS_MutexUnlock(&mutex);
            i = 0;
        }
	}
}

stos_tcb_t T2;
void stos_task_2(void) {
    STOS_MutexLock(&mutex, &T2, STOS_MUTEX_WAIT_NONE);
	while (true) {
        printf("Task 2\r\n");
        STOS_YieldTask();
	}
}

int main(void) {
    RCC_Enable_GPIOA_Clk();

    USART_init(USART2,115200);
    printf("DEBUG: RUNNING STOS RTOS\r\n");
    printf("------------------------\r\n");


    GPIO_SetMode(GPIOA, GPIO_PIN_5, GPIO_OUTPUT);
    Enable_Bus_Usage_Flts();

    STOS_CreateTask(&T1, &stos_task_1, 4, 100);

    STOS_CreateTask(&T2, &stos_task_2, 4, 100);

    STOS_Run(STOS_IDLE_DEFAULT_HANDLER, STOS_IDLE_DEFAULT_PRIORITY);

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
