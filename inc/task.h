#pragma once

#include <stdint.h>
#include <stddef.h>

#define STOS_MIN_PRI            (0x0U)
#define STOS_MAX_PRI            (0x8U)
#define BYTE_ALIGN              (0x7U)

typedef enum stos_task_state {
    STOS_TASK_READY,
    STOS_TASK_RUNNING,
    STOS_TASK_BLOCKED,
    STOS_TASK_TIMEOUT,
    STOS_TASK_ERROR
} stos_task_state_t;

typedef struct stos_tcb {
    uint32_t    *sp;
    void        (*func)(void);
    uint32_t    *stack_end;

    uint32_t    state;

    uint32_t    base_pri;
    uint32_t    cur_pri;
    uint32_t    timeout;

    struct stos_tcb *next;
    struct stos_tcb *prev;
} stos_tcb_t;