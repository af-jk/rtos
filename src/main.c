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

stos_tcb_t T1;
void stos_task_1(void) {
	// Timeout highest priority task for 2 sys ticks to ensure task 2 gets the mutex
	STOS_TimeoutTask(1);
	for (;;) {
        // Lock the mutex
        STOS_MutexLock(&mutex, &T1, STOS_MUTEX_WAIT_NONE);
        for (int i = 0; i < 2; i++) {
            printf("Task 1\r\n");
        }
        //Once done with operation, unlock mutex
        STOS_MutexUnlock(&mutex);
	}
}

stos_tcb_t T2;
void stos_task_2(void) {
	for (;;) {
        // Lock the mutex and takes a while to process information
        STOS_MutexLock(&mutex, &T2, STOS_MUTEX_WAIT_NONE);
        for (int i = 0; i < 10000; i++) {
            printf("Task 2\r\n");
        }
        // Once done with operation, unlock mutex
        STOS_MutexUnlock(&mutex);
	}
}

stos_tcb_t T3;
void stos_task_3(void) {
	STOS_TimeoutTask(3);
	// Will try to run while task 1 is blocked, but because task 2's priority gets elevated
	for (;;) {
        printf("Task 3\r\n");
	}
}



int main(void) {
    RCC_Enable_GPIOA_Clk();

    USART_init(USART2,115200);
    printf("DEBUG: RUNNING STOS RTOS\r\n");
    printf("------------------------\r\n");


    GPIO_SetMode(GPIOA, GPIO_PIN_5, GPIO_OUTPUT);
    Enable_Bus_Usage_Flts();

    STOS_CreateTask(&T1, &stos_task_1, 6, 100);

    STOS_CreateTask(&T2, &stos_task_2, 4, 100);

    STOS_CreateTask(&T3, &stos_task_3, 5, 100);


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
