#pragma once

#include <stdint.h>
#include <stddef.h>

#include "task.h"

// From core_cm4.h
#define SCB_ICSR_PENDSVSET_Pos  28U                                            /*!< SCB ICSR: PENDSVSET Position */
#define SCB_ICSR_PENDSVSET_Msk  (1UL << SCB_ICSR_PENDSVSET_Pos) 

#define STOS_IDLE_DEFAULT_CONFIG NULL, 0
#define STOS_IDLE_DEFAULT_PRIORITY 0

#define STACK_CRPT_DETECT_REG_SIZE (4U)
#define STACK_CRPT_DETECT_SEQ      (0xFAFAFAFA)
#define PENDSV_PRIORITY_Pos        (0x4U)  // PendSV priority is in the upper 4 bits of SHP[7]
#define PENDSV_PRIORITY_Mask       (0xFU << PENDSV_PRIORITY_Pos)  // Mask for the priority bits (upper 4 bits)

typedef struct stos_kernel {
    stos_tcb_t *list_ready_head;
    stos_tcb_t *list_blocked_head;

    stos_tcb_t *next_task;
    stos_tcb_t *active_task;
    stos_tcb_t *idle_task;
} stos_kernel_t;

void STOS_CreateTask(stos_tcb_t * const task,
                     void       (*handler)(void),
                     uint32_t   pri,
                     uint32_t   size);
void STOS_AddTask(stos_tcb_t * const task, uint32_t state);
void STOS_RemoveTask(stos_tcb_t * const task);
void STOS_TimeoutTask(uint32_t timeout);

void STOS_Init(void (*handler)(void), uint32_t size);
void STOS_Schedule(void);
void STOS_Run(void);