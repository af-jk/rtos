#include "sync.h"
#include "stos.h"
#include "syscall.h"

/*
 * Returns the value stored within lock
 * Lock should only have two values:
 * Returns 0 if value can be written
 * 0 -> Not currently locked
 * 1 -> Currently locked 
 */
__attribute__((naked)) static uint32_t __stos_check_lock(stos_mutex_t *mutex) {
    /*
    * mutex will be passed as param through R0
    * store into R1, perform the LDREX instruction and get the result through R0
    * default return value of ARM is through r0 (BX lr will return r0)
    */
    __asm volatile(
        " MOV   r1, r0          \n"
        " LDREX r0, [r1, #0x00] \n"
        " BX lr                 \n"
    );
}

/*
 * Returns 0 if value can be written
 * Returns 1 if value can't be written
 */
__attribute__((naked)) static uint32_t __stos_write_to_lock(uint32_t val, uint32_t *addr) {
    /*
     * val will be passed as param through R0
     * mutex will be passed as param through R1
     * store val into R2 (use R0 as return value for after the STREX inst.)
     */

    __asm volatile(
        " MOV   r2, r0              \n"
        " STREX r0, r2, [r1, 0x00]  \n"
        " BX lr                     \n"
    );
}

static bool STOS_MutexTryLock(stos_mutex_t *mutex) {
    /*
     * LDREX performs a load (read mutex) and establishes a hardware monitor on the variable
     * STREX *must* be performed after a LDREX - if the hardware monitor hasn't seen any
     * changes to the variable, it will allow the store instruciton and return 0.
     * Otherwise, if a change has happened since, it will fail
     * 
     * Essentially, there are two main (maybe more?) ways this lock will fail:
     * 1: The lock is already taken (LDREX returns 1)
     * 2: The lock is not taken, but somehow the mutex has been written to
     */
    uint32_t status;

    // Check to see if mutex is currently being accessed
    if (__stos_check_lock(mutex) == 0) {

        // Attempt to set lock to 1
        status = __stos_write_to_lock(1, &(mutex->lock));

        if (status == 1) return STOS_MUTEX_NOT_ACQUIRED;

        return STOS_MUTEX_ACQUIRED;
    }

    return STOS_MUTEX_NOT_ACQUIRED;
}

void STOS_MutexLock(stos_mutex_t *mutex, stos_tcb_t *task, uint32_t wait) {
    STOS_Syscall_KernelCriticalStart();

    if (wait == STOS_MUTEX_WAIT_NONE) {

        bool lock_status = STOS_MutexTryLock(mutex);

        if (lock_status == STOS_MUTEX_ACQUIRED) {
            mutex->pri = task->cur_pri;
            mutex->holder = task;
        }
        
        if (lock_status == STOS_MUTEX_NOT_ACQUIRED) {

            // If a task with higher priority than our holder has attempted to access the mutex and failed,
            // we must bump the priority of the holder to prevent priority inversion
            if (task->cur_pri > mutex->pri) {
                mutex->pri = task->cur_pri;
                mutex->holder->cur_pri = mutex->pri;
            }

            // Going to remove the status of active_task and call scheduler
            STOS_Block(mutex);
        }
    }

    // Consider the priority inversion in this case
    if (wait == STOS_MUTEX_WAIT_RETRY) {
        while (STOS_MutexTryLock(mutex) == STOS_MUTEX_NOT_ACQUIRED) {
            if (task->cur_pri > mutex->pri) {
                mutex->pri = task->cur_pri;
                mutex->holder->cur_pri = mutex->pri;
            }
            STOS_YieldTask();
            // Yield and try again
        }
    }

    if (wait == STOS_MUTEX_WAIT_INDEF) {
        while (STOS_MutexTryLock(mutex) == STOS_MUTEX_NOT_ACQUIRED) {
            continue;
            // continually retry
        }
    }

    STOS_Syscall_KernelCriticalEnd();
}

bool STOS_MutexUnlock(stos_mutex_t *mutex) {
    /*
     * STREX is dependent on the LDREX value not being modified.
     * This is why it's necessary to call check lock (LDREX) because
     * unless this is done, at this point the previous LDREX instruction
     * has likely been overwritten and the STREX will fail.
     */

    STOS_Syscall_KernelCriticalStart();
    __stos_check_lock(mutex);


    uint32_t lock_status = __stos_write_to_lock(0, &(mutex->lock));

    if (lock_status == 0) {
        // Need to signal signal to the mutex's blocked tasks that they can be added to ready list
        // Further the unblock task will restore the holders priority (in case we bumped it 
        // to prevent priority inversion)
        STOS_Unblock(mutex); 
        mutex->holder = NULL;
        mutex->pri = 0;
    }

    STOS_Syscall_KernelCriticalEnd();
    return lock_status;
}

// Do not need to implement priority inversion logic, they generally shouldn't be used for critical region locking
// and instead are for resource sharing
void STOS_SemInit(stos_sem_t *sem, uint32_t val) {
    sem->lock = val;
}

void STOS_SemWait(stos_sem_t *sem) {
    uint32_t sem_val;
    while ((sem_val = __stos_check_lock(sem)) == 0) {
        continue;
    }
    __stos_write_to_lock(sem_val - 1, &(sem->lock));
}

void STOS_SemPost(stos_sem_t *sem) {
    STOS_Syscall_KernelCriticalStart();
    uint32_t sem_val = __stos_check_lock(sem);
    __stos_write_to_lock(sem_val + 1, &(sem->lock));
    STOS_Syscall_KernelCriticalEnd();
}
