#pragma once

#include <stdint.h>
#include <stddef.h>

#define STOS_TASK_READY         (0x00U)
#define STOS_TASK_BLOCKED       (0x01U)
#define STOS_TASK_RUNNING       (0x02U)
#define STOS_TASK_TIMEOUT       (0x03U)
#define STOS_TASK_YIELD         (0x04U)

#define STOS_MIN_PRI            (0x0U)
#define STOS_MAX_PRI            (0x8U)

#define BYTE_ALIGN              (0x7U)

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