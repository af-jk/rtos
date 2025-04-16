// In the future, will come up with a method to tie C preprocessor defines to with ASM so that 
// constants like these don't need to be defined twice.
// For now, be careful to make sure syscalls follow format defined below:
#define STOS_SYSC_LAUNCH_RTOS           (0x0U)
#define STOS_SYSC_KERNEL_CRIT_START     (0x1U)
#define STOS_SYSC_KERNEL_CRIT_END       (0x2U)

#include <stdint.h>

void STOS_Syscall_LaunchSTOS(void);
void STOS_Syscall_TriggerPendSV(void);
void STOS_Syscall_DisableInterrupts(void);
void STOS_Syscall_EnableInterrupts(void);
uint32_t STOS_Syscall_KernelCriticalStart(void);
void STOS_Syscall_KernelCriticalEnd(uint32_t old_basepri);