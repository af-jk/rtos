#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "task.h"

typedef struct stos_mutex {
    uint32_t lock;
} stos_mutex_t;

bool STOS_MutexLock(stos_mutex_t *mutex);
bool STOS_MutexUnlock(stos_mutex_t *mutex);
