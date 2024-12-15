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


int buffer[BUFFER_SIZE];
int in = 0;
int out = 0;

stos_mutex_t mutex;
stos_sem_t empty;
stos_sem_t full;

void producer(void) {
    int item = 0;
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

void consumer(void) {
	while (1) {
        STOS_SemWait(&full);
        STOS_MutexLock(&mutex);
        
        int item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;

        STOS_MutexUnlock(&mutex);
        STOS_SemPost(&empty);
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

    STOS_Init(STOS_IDLE_DEFAULT_CONFIG);
    STOS_Run();

    for (;;) {
    }
}
