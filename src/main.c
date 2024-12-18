#include <inttypes.h>
#include <stdbool.h>

#include "clock.h"
#include "cm4_periphs.h"
#include "gpio.h"
#include "interrupts.h"
#include "sync.h"
#include "stos.h"

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
        STOS_SemWait(&empty);
        STOS_MutexLock(&mutex);
        
        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE;
        
        STOS_MutexUnlock(&mutex);
        STOS_SemPost(&full);
    }
}

uint32_t last_cons = 0;
void consumer(void) {
	while (1) {
        STOS_SemWait(&full);
        STOS_MutexLock(&mutex);
        
        uint32_t item = buffer[out];
        last_cons = item;
        out = (out + 1) % BUFFER_SIZE;

        STOS_MutexUnlock(&mutex);
        STOS_SemPost(&empty);

        STOS_TimeoutTask(3);
    }
}

void consumer2(void) {
	while (1) {
        STOS_SemWait(&full);
        STOS_MutexLock(&mutex);

        uint32_t item = buffer[out];
        last_cons = item;
        out = (out + 1) % BUFFER_SIZE;

        STOS_MutexUnlock(&mutex);
        STOS_SemPost(&empty);

        STOS_TimeoutTask(2);
    }
}

int main(void) {
    RCC_Enable_GPIOA_Clk();

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
