#include <inttypes.h>
#include <stdbool.h>

#include "clock.h"
#include "cm4_periphs.h"
#include "gpio.h"
#include "interrupts.h"
#include "mutex.h"
#include "stos.h"

#pragma GCC diagnostic ignored "-Wunused-variable"

stos_mutex_t mutex;
static int i = 0;

void main_tick1(void) {
	while (1) {
        if (STOS_MutexLock(&mutex) == 0) {
            GPIO_SetHigh(GPIOA, GPIO_PIN_5);
            STOS_Sleep(10);
            STOS_MutexUnlock(&mutex);
            STOS_Sleep(10);
        }
    }
}

void main_tick2(void) {
	while (1) {
        if (STOS_MutexLock(&mutex) == 0) {
            GPIO_SetLow(GPIOA, GPIO_PIN_5);
            STOS_MutexUnlock(&mutex);
        }
    }
}

int main(void) {
    RCC_Enable_GPIOA_Clk();

    GPIO_SetMode(GPIOA, GPIO_PIN_5, GPIO_OUTPUT);
    Enable_Bus_Usage_Flts();

    stos_tcb_t T1 = {0};
    STOS_CreateTask(&T1, &main_tick1, 3, 40);

    stos_tcb_t T2 = {0};
    STOS_CreateTask(&T2, &main_tick2, 3, 40);

    STOS_Init(STOS_IDLE_DEFAULT_CONFIG);
    STOS_Run();

    for (;;) {
    }
}
