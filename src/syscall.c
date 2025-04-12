/*
Operating on the assumption that the following macros map

        ".equ STOS_SYSC_LAUNCH_RTOS,           0    \n"
        ".equ STOS_SYSC_KERNEL_CRIT_START,     1    \n"
        ".equ STOS_SYSC_KERNEL_CRIT_END,       2    \n"

Need to come up with a good way to share pre-processor macros between C and ASM files
*/

// W iant prologue and epilogue sequences (will use the stack frame)
void svc_handler(void) {
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
        " LDR   r3, [r2, #28]                       \n"
        " LDR  	r2, [r3, #-2]                       \n"

        // R3 now has the syscall number
        " AND 	r3, r2, #0xFF	                    \n"


        // Pick syscall
        " CMP   r3, #0                              \n" // STOS_SYSC_LAUNCH_RTOS (= 0)
        " BEQ   SYSC_LAUNCH_RTOS                    \n"
        " CMP   r3, #1                              \n" // STOS_SYSC_KERNEL_CRIT_START (= 1)
        " BEQ   SYSC_KERNEL_CRIT_START              \n"

    	// Fall through to last syscall (STOS_SYSC_KERNEL_END = 2)
    	" SYSC_KERNEL_CRIT_END:                     \n"
    	" BL    __stos_kernel_critical_end          \n"
    	" BX    LR                                  \n"


        " SYSC_LAUNCH_RTOS:                         \n"
        " MOV   LR, #0xFFFFFFFD                     \n"
        " BX    LR                                  \n"

        " SYSC_KERNEL_CRIT_START:                   \n"
        " BL    __stos_kernel_critical_start        \n"
        " BX    LR                                  \n"


    );
}
