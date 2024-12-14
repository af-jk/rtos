#include "mutex.h"

/*
 * Returns the value stored within lock
 * Lock should only have two values:
 * Returns 0 if value can be written
 * 0 -> Not currently locked
 * 1 -> Currently locked 
 */
__attribute__((naked)) uint32_t __stos_check_lock(stos_mutex_t *mutex) {
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
__attribute__((naked)) uint32_t __stos_write_to_lock(uint32_t val, uint32_t *addr) {
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

bool STOS_MutexLock(stos_mutex_t *mutex) {
    /*
     * LDREX performs a load (read mutex) and establishes a hardware monitor on the variable
     * STREX *must* be performed after a LDREX - if the hardware monitor hasn't seen any
     * changes to the variable, it will allow the store instruciton and return 0.
     * Otherwise, if a change has happened since, it will fail
     * 
     * Essneitally, there are two main (maybe more?) ways this lock will fail:
     * 1: The lock is already taken (LDREX returns 1)
     * 2: The lock is not taken, but somehow the mutex has been written to
     */
    uint32_t status;

    // Check to see if mutex is currently being accessed
    if (__stos_check_lock(mutex) == 0) {

        // Attempt to set lock to 1
        status = __stos_write_to_lock(1, &(mutex->lock));

        if (status == 1) return 1;

        return 0;
    }

    return 1;
}

bool STOS_MutexUnlock(stos_mutex_t *mutex) {
    /*
     * STREX is dependent on the LDREX value not being modified.
     * This is why it's necessary to call check lock (LDREX) because
     * unless this is done, at this point the previous LDREX instruction
     * has likely been overwritten and the STREX will fail.
     */
    __stos_check_lock(mutex);
    return __stos_write_to_lock(0, &(mutex->lock));
}
