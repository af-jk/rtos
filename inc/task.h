#pragma once

#include <stdint.h>
#include <stddef.h>

#define TASK_READY      0x00U
#define TASK_BLOCKED    0x01U
#define TASK_RUNNING    0x02U
#define TASK_SUSPENDED  0x03U

#define STOS_MIN_PRI    0x0U
#define STOS_MAX_PRI    0x8U

#define BYTE_ALIGN 0x7U

typedef struct stos_tcb {
    void        *sp;
    void        (*func)(void);

    uint32_t    state;

    uint32_t    pri;
    uint32_t    timeout;

    struct stos_tcb *next;
    struct stos_tcb *prev;
} stos_tcb_t;