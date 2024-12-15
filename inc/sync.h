#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "task.h"

typedef struct stos_mutex {
    uint32_t lock;
} stos_mutex_t;

typedef stos_mutex_t stos_sem_t;

bool STOS_MutexTryLock(stos_mutex_t *mutex);
void STOS_MutexLock(stos_mutex_t *mutex);
bool STOS_MutexUnlock(stos_mutex_t *mutex);

void STOS_SemInit(stos_sem_t *sem, uint32_t val);
void STOS_SemWait(stos_sem_t *sem);
void STOS_SemPost(stos_sem_t *sem);