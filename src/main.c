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


void delay(volatile uint32_t delayTime) {
    while (delayTime--);
}

#pragma GCC diagnostic ignored "-Wunused-variable"

#define BUFFER_SIZE 5


uint32_t buffer[BUFFER_SIZE];
uint32_t in = 0;
uint32_t out = 0;

stos_mutex_t mutex;
stos_sem_t empty;
stos_sem_t full;

void producer(void) {
    uint32_t item = 0;
	while (1) {
        item++;
        printf("PRODUCER:\tWaiting for an empty slot ...\r\n");
        STOS_SemWait(&empty);
        printf("PRODUCER:\tAcquired an empty slot\r\n");
        STOS_MutexLock(&mutex);
        printf("PRODUCER:\tProducing item %d at index %d\r\n", item, in);
        
        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE;
        
        STOS_MutexUnlock(&mutex);
        printf("PRODUCER:\tProduced item\r\n");
        STOS_SemPost(&full);
    }
}

void consumer(void) {
	while (1) {
        printf("CONSUMER 1:\tWaiting for an full slot ...\r\n");
        STOS_SemWait(&full);
        printf("CONSUMER 1:\tAcquired for full slot\r\n");
        STOS_MutexLock(&mutex);
        printf("CONSUMER 1:\tConsuming at index %d\r\n", out);
        
        uint32_t item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;

        STOS_MutexUnlock(&mutex);
        printf("CONSUMER 1:\tConsumed item %d\r\n", item);
        STOS_SemPost(&empty);

        // STOS_TimeoutTask(2);
    }
}

void consumer2(void) {
	while (1) {
        printf("CONSUMER 2:\tWaiting for an full slot ...\r\n");
        STOS_SemWait(&full);
        printf("CONSUMER 2:\tAcquired for full slot\r\n");
        STOS_MutexLock(&mutex);
        printf("CONSUMER 2:\tConsuming at index %d\r\n", out);

        uint32_t item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;

        STOS_MutexUnlock(&mutex);
        printf("CONSUMER 2:\tConsumed item %d\r\n", item);
        STOS_SemPost(&empty);

        // STOS_TimeoutTask(10);
    }
}

int main(void) {
    RCC_Enable_GPIOA_Clk();

    USART_init(USART2,115200);
    printf("DEBUG: RUNNING STOS RTOS\r\n");
    printf("------------------------\r\n");


    GPIO_SetMode(GPIOA, GPIO_PIN_5, GPIO_OUTPUT);
    Enable_Bus_Usage_Flts();

    STOS_SemInit(&empty, BUFFER_SIZE);
    STOS_SemInit(&full, 0);

    stos_tcb_t T1 = {0};
    STOS_CreateTask(&T1, &producer, 3, 200);

    stos_tcb_t T2 = {0};
    STOS_CreateTask(&T2, &consumer, 3, 200);

    stos_tcb_t T3 = {0};
    STOS_CreateTask(&T3, &consumer2, 3, 200);

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
    return len;
 }
