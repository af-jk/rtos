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

typedef enum stos_mutex_wait {
    STOS_MUTEX_WAIT_NONE,
    STOS_MUTEX_WAIT_RETRY,
    STOS_MUTEX_WAIT_INDEF
} stos_mutex_wait_t;

typedef stos_mutex_t stos_sem_t;

void STOS_MutexLock(stos_mutex_t *mutex, stos_tcb_t *task, stos_mutex_wait_t wait);
bool STOS_MutexUnlock(stos_mutex_t *mutex);


// Don't worry about semaphores for now
void STOS_SemInit(stos_sem_t *sem, uint32_t val);
void STOS_SemWait(stos_sem_t *sem);
void STOS_SemPost(stos_sem_t *sem);