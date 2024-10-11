#pragma once

#include <stdint.h>

#define SIZE_OF_STACK_FRAME 16U

typedef struct stos_tcb {
    void            *sp;
    struct stos_tcb *next;
    void            (*func)(void);
} stos_tcb_t;

void STOS_Init(void);
void STOS_CreateTask(stos_tcb_t *task,
                     void       (*handler)(void),
                     uint32_t   *stk,
                     uint32_t   size);
void STOS_Schedule(void);
void STOS_Run(void);
void pend_sv_handler(void);
