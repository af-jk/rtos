/*
Operating on the assumption that the following macros map

        ".equ STOS_SYSC_LAUNCH_RTOS,           0    \n"
        ".equ STOS_SYSC_KERNEL_CRIT_START,     1    \n"
        ".equ STOS_SYSC_KERNEL_CRIT_END,       2    \n"

Need to come up with a good way to share pre-processor macros between C and ASM files
*/

/*
Within an exception, the control value privileged vs. unpriviledged doesn't matter,
it's only relevant outside of handler mode (within user code). Thus priviledged kernel 
operations requested by a user task must occur under the context of a svc call (syscall). 
*/

#include "syscall.h"
#include "interrupts.h"

__attribute((naked)) void svc_handler(void) {
     __asm volatile (

        /*
        1. Determine if we entered from the PSP or MSP
        2. Based on that, we pop a different context
        */

        // Determine if entered from PSP or MSP
        " TST   lr, #4                              \n"
        " ITE   eq                                  \n"
        " MRSEQ r2, msp                             \n"
        " MRSNE r2, psp                             \n"

        // Extract SVC value
    	// Our stack frame pushes the next address onto the stack
        // The SVC argument is passed through machine code instruction
        // We can extract it by getting our previous pc instruction and
    	// Checking the LSB

    	// Why use 28 earlier???
    	" LDR   r3, [r2, #24]                       \n"
        " LDR  	r2, [r3, #-2]                       \n"

        // R3 now has the syscall number
        " AND 	r3, r2, #0xFF	                    \n"


        // Pick syscall
        " CMP   r3, #0                              \n"
        " BEQ   SYSC_LAUNCH_RTOS                    \n"

        " CMP   r3, #1                              \n"
        " BEQ   SYSC_TRIGGER_PENDSV                 \n"

        " CMP   r3, #2                              \n"
        " BEQ   SYSC_DISABLE_INTERRUPTS             \n"

        " CMP   r3, #3                              \n"
        " BEQ   SYSC_KERNEL_CRIT_START              \n"

        " CMP   r3, #4                              \n"
        " BEQ   SYSC_KERNEL_CRIT_END                \n"
        
        // Otherwise return this should be an error condition because
        // we passed an invalid syscall
        " BX    LR                                  \n"

    	// Fall through to last syscall (STOS_SYSC_KERNEL_END = 2)

        /*
        1. Get original basepri as an argument
        2. Restore the original basepri
        */

        " SYSC_LAUNCH_RTOS:                         \n"
        " MOV   lr, #0xFFFFFFFD                     \n"
        " BX    lr                                  \n"

        " SYSC_TRIGGER_PENDSV:                      \n"
        " PUSH  {lr}                                \n"
        " BL    PendSV_Set                          \n"
        " POP   {lr}                                \n"
        " BX    LR                                  \n"

        " SYSC_DISABLE_INTERRUPTS:                  \n"
        // Before disabling interrupts, allow thread mode to be priviledged
        " AND   r0, r0, #0xFFFFFFFE                 \n" // set priv to 0
        " MSR   control, r0                         \n"
        " CPSID I                                   \n"
        " BX    LR                                  \n"

        // Enabling interrupts will be called a "system call"
        // but it naturally cant happen within an SVC while interrupts are masked


        /*
        1. Read and store original basepri (in R0, the return register)
        2. Write new basepri
        */
        " SYSC_KERNEL_CRIT_START:                   \n"
        // Using 0xF0 -> to set basepri to F (15) so that it sets the minimum required 
        // exception priority above systick and pendsv (no other exceptions should directly modify)
        // kernel data structures
        " MOV   r1, #0xE0                           \n"
        " MSR   basepri, r1                         \n"
        " BX    lr                                  \n"

    	" SYSC_KERNEL_CRIT_END:                     \n"
        " PUSH  {r0}                                \n"
        " MOV   r0, #0x0                            \n"
        " MSR   basepri, r0                         \n"
        " POP   {r0}                                \n"
        " BX    lr                                  \n"
    );
}

void STOS_Syscall_TriggerPendSV(void) {
    // Check to see if we're currently in thread/handler mode
    // Will use the IPSR register
    uint32_t active_exception;
    __asm volatile("MRS %0, IPSR": "=r" (active_exception));

    if (active_exception > 0) {
        // Handler mode, can just trigger pendsv
        PendSV_Set();
        return;
    }

    // Otherwise, in thread mode and need to activate a system call
    __asm volatile("SVC #1 \n");
}

// To disable interrupts means that the processor must have privliged execution while disabled
// so that it can re-enable them later on
__attribute__((naked)) void STOS_Syscall_DisableInterrupts(void) {

    __asm volatile(
        " SVC   #2  \n"
        " BX    lr  \n"
    );
}

__attribute__((naked)) void STOS_Syscall_EnableInterrupts(void) {
    __asm volatile(
        " CPSIE I               \n"
        " PUSH  {r0}            \n"
        " ORR   r0, r0, #0x1    \n" // set priv to 1
        " MSR   control, r0     \n"
        " POP   {r0}            \n"
        " BX    lr              \n"
    );
}

// Issue: This sequence clears the previously held basepri value, which made be an issue if a user is
// relying on it. Need to determine a way to return the current basepri through the svc exception.
// However, it could also be argued that this sort of priv vs. unpriv RTOS implies not personally 
// modifying privileged registers within a task

__attribute__((naked)) void STOS_Syscall_KernelCriticalStart(void) {
    __asm volatile(
        " SVC    #3 \n"
        " BX     lr \n"
    );
}

__attribute__((naked)) void STOS_Syscall_KernelCriticalEnd(void) {
    __asm volatile(
        " SVC    #4 \n"
        " BX     lr \n"
    );
}
