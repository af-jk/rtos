#pragma once

#include <stdint.h>
#include <stddef.h>

#define TASK_READY      0x01U
#define TASK_RUNNING    0x02U
#define TASK_SUSPENDED  0x03U
#define TASK_BLOCKED    0x04U

#define STOS_MIN_PRI    0x0U
#define STOS_MAX_PRI    0x8U

// From core_cm4.h
#define SCB_ICSR_PENDSVSET_Pos  28U                                            /*!< SCB ICSR: PENDSVSET Position */
#define SCB_ICSR_PENDSVSET_Msk  (1UL << SCB_ICSR_PENDSVSET_Pos) 

#define STOS_IDLE_DEFAULT_CONFIG NULL, 0
#define STOS_IDLE_DEFAULT_PRIORITY 0

// Credit to @https://dmitryfrank.com/articles/how_i_ended_up_writing_my_own_kernel
// for the inspiration on how to implement intrusive linked list

#define STOS_LIST_CONTAINER_GET_TCB(ptr, type, member) \
   ((type *)((char *)(ptr)-(char *)(&((type *)0)->member)))

typedef struct stos_list stos_list_t;

// Time to make an intrusive doubly linked list
typedef struct stos_list {
    stos_list_t *prev;
    stos_list_t *next;
} stos_list_t;

typedef struct stos_tcb {
    void        *sp;
    uint32_t    pri;
    uint32_t    state;
    void        (*func)(void);
    uint32_t    delay_amt;

    // intrusive list item
    stos_list_t list;
} stos_tcb_t;

void STOS_Init(void (*handler)(void), uint32_t size);
void STOS_CreateTask(stos_tcb_t *task,
                     void       (*handler)(void),
                     uint32_t   pri,
                     uint32_t   size);
void STOS_AddTask(stos_tcb_t *task, stos_list_t *task_list);
void STOS_RemoveTask(stos_tcb_t *task, stos_list_t *task_list);
void STOS_TaskDelay(uint32_t delay);
void STOS_Schedule(void);
void STOS_Run(void);
