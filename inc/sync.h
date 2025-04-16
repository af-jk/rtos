#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "task.h"

#define STOS_MUTEX_ACQUIRED     (0U)
#define STOS_MUTEX_NOT_ACQUIRED (1U)

typedef struct stos_mutex {
    uint32_t lock;
    uint32_t pri;
    stos_tcb_t *holder;
    stos_tcb_t *blocked_list_head; // maintain singly linked list
} stos_mutex_t;

#define STOS_MUTEX_WAIT_NONE    (0U)

typedef stos_mutex_t stos_sem_t;

void STOS_MutexLock(stos_mutex_t *mutex, stos_tcb_t *task, uint32_t wait);
bool STOS_MutexUnlock(stos_mutex_t *mutex);

void STOS_SemInit(stos_sem_t *sem, uint32_t val);
void STOS_SemWait(stos_sem_t *sem);
void STOS_SemPost(stos_sem_t *sem);