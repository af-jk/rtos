#pragma once

#include <stdint.h>
#include <stddef.h>

#define SIZE_OF_STACK_FRAME 16U

// From core_cm4.h
#define SCB_ICSR_PENDSVSET_Pos             28U                                            /*!< SCB ICSR: PENDSVSET Position */
#define SCB_ICSR_PENDSVSET_Msk             (1UL << SCB_ICSR_PENDSVSET_Pos) 

typedef struct stos_tcb {
    void            *sp;
    struct stos_tcb *next;
    void            (*func)(void);
} stos_tcb_t;

void STOS_Init(void);
void STOS_CreateTask(stos_tcb_t *task,
                     void       (*handler)(void),
                     void       *stk,
                     uint32_t   size);
void STOS_Schedule(void);
void STOS_Run(void);
//void pend_sv_handler(void);
