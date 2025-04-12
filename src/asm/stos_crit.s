.syntax unified
.cpu    cortex-m4
.fpu    softvfp
.thumb

// Need to come up with a better way to unify this value with my pre-processor one
// Tried making this file a .S, but ran into issues with header files not being proper
// assembly
.equ STOS_MAX_KERNEL_EXCEP_MODIFIER, 0xF

/*
1. Can only write to basepri if in priveleged mode
2. Read and store original basepri (in R0, the return register)
3. Write new basepri
4. Branch back
*/
    .global __stos_kernel_critical_start
    .type __stos_kernel_critical_start, %function
__stos_kernel_critical_start:
    mrs r0, basepri
    mov r1, #STOS_MAX_KERNEL_EXCEP_MODIFIER
    msr basepri, r1
    bx  lr


/*
1. Get original basepri as an argument
2. Restore the original basepri
3. Branch back
*/
    .global __stos_kernel_critical_end
    .type __stos_kernel_critical_end, %function
__stos_kernel_critical_end:
    msr basepri, r0
    bx  lr
