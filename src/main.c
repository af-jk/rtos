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
#include "syscall.h"

stos_mutex_t mutex;

stos_tcb_t T1 = {0};
void task1(void) {
	while (1) {
        // Acquire mutex
        while (STOS_MutexLock(&mutex, &T1, STOS_MUTEX_WAIT_NONE) != STOS_MUTEX_ACQUIRED) {
            continue;
        }

        // Enter critical region

        for (volatile int i = 0; i < 1000; i++) {
            GPIO_Toggle(GPIOA, GPIO_PIN_8);
            GPIO_Toggle(GPIOA, GPIO_PIN_8);
        }

        // Exit critical region
        STOS_MutexUnlock(&mutex);
    }
}

stos_tcb_t T2 = {0};
void task2(void) {
    STOS_TimeoutTask(2);
	while (1) {
        for (volatile int i = 0; i < 1000; i++) {
            GPIO_Toggle(GPIOA, GPIO_PIN_7);
            GPIO_Toggle(GPIOA, GPIO_PIN_7);
        }
        STOS_TimeoutTask(1);
    }
}

stos_tcb_t T3 = {0};
void task3(void) {
    STOS_TimeoutTask(1);
	while (1) {
        // Acquire mutex
        while (STOS_MutexLock(&mutex, &T3, STOS_MUTEX_WAIT_NONE) != STOS_MUTEX_ACQUIRED) {
            continue;
        }

        // Enter critical region
        for (volatile int i = 0; i < 1000; i++) {
            GPIO_Toggle(GPIOA, GPIO_PIN_6);
            GPIO_Toggle(GPIOA, GPIO_PIN_6);
        }

        // Exit critical region
        STOS_MutexUnlock(&mutex);

        STOS_TimeoutTask(1);
    }
}

int main(void) {
    RCC_Enable_GPIOA_Clk();

    USART_init(USART2,115200);
    printf("DEBUG: RUNNING STOS RTOS\r\n");
    printf("------------------------\r\n");


    GPIO_SetMode(GPIOA, GPIO_PIN_1, GPIO_OUTPUT);
    GPIO_SetMode(GPIOA, GPIO_PIN_4, GPIO_OUTPUT);
    GPIO_SetMode(GPIOA, GPIO_PIN_5, GPIO_OUTPUT);
    GPIO_SetMode(GPIOA, GPIO_PIN_6, GPIO_OUTPUT);   // 1st
    GPIO_SetMode(GPIOA, GPIO_PIN_7, GPIO_OUTPUT);   // 2nd
    GPIO_SetMode(GPIOA, GPIO_PIN_8, GPIO_OUTPUT);   // 4th
    GPIO_SetMode(GPIOA, GPIO_PIN_9, GPIO_OUTPUT);   // 3rd
    GPIO_SetMode(GPIOA, GPIO_PIN_10, GPIO_OUTPUT);  // 5th
    Enable_Bus_Usage_Flts();


    STOS_CreateTask(&T1, &task1, 1, 200);

    STOS_CreateTask(&T2, &task2, 2, 200);

    STOS_CreateTask(&T3, &task3, 3, 200);

    for (int i = 0; i < 100000; i++) {
        continue;
    }

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
