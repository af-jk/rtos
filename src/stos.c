#include "stos.h"
#include "interrupts.h"

#define PENDSV_PRIORITY_Pos  0x4U   // PendSV priority is in the upper 4 bits of SHP[7]
#define PENDSV_PRIORITY_Mask (0xF << PENDSV_PRIORITY_Pos)  // Mask for the priority bits (upper 4 bits)
#define BYTE_ALIGN           0x7U

static stos_tcb_t *stos_cur = NULL;
static stos_kernel_t stos_kernel;

void STOS_Init (void) {
    //NVIC_SetPri(pend_sv_IRQn, 15U);
    uint8_t priority = 15U;
    priority = (priority & 0xFU) << PENDSV_PRIORITY_Pos;
    SCB->SHP[7] = priority;
}

void STOS_CreateTask (stos_tcb_t *task,
                      void       (*handler)(void),
                      uint32_t   size) {

    uint32_t *psp;
    __asm volatile ("MRS %0, psp" : "=r" (psp) );
    uint32_t *init_sp = (uint32_t *)((uint32_t)psp - stos_kernel.amount_psp_alloc);

    // When the task first starts, we're going to pre-fill the stack frame as such
    // PC and LR are required, the others are filled in just for easier debugging
    *(--init_sp) = (1U << 24);        // xPSR
    *(--init_sp) = (uint32_t)handler; // PC
    *(--init_sp) = 0xFFFFFFFDU;       // LR - needs to be updated for PSP/MSP
    *(--init_sp) = 0x0000000CU;       // R12
    *(--init_sp) = 0x0000000BU;       // R11
    *(--init_sp) = 0x0000000AU;       // R10
    *(--init_sp) = 0x00000009U;       // R9
    *(--init_sp) = 0x00000008U;       // R8
    *(--init_sp) = 0x00000007U;       // R7
    *(--init_sp) = 0x00000006U;       // R6
    *(--init_sp) = 0x00000005U;       // R5
    *(--init_sp) = 0x00000004U;       // R4
    *(--init_sp) = 0x00000003U;       // R3
    *(--init_sp) = 0x00000002U;       // R2
    *(--init_sp) = 0x00000001U;       // R1
    *(--init_sp) = 0x00000000U;       // R0
    
    // update sp -- this is the new top of the stack
    task->sp = init_sp;
    task->func = handler;

    // Fill in what remains of size - 16 (size of stack frame) w/ pre-set val
    uint32_t *region_end = init_sp - size + 4;
    while (init_sp > region_end) {
        *(--init_sp) = 0xDEADBEEF;
    }

    // maybe overkill, but ensure alignment
    while (((uint32_t)init_sp & BYTE_ALIGN) != 0) {
        *(--init_sp) =0xFEEBDEADU;
    }

    // update psp offset
    stos_kernel.amount_psp_alloc += (uint32_t) (psp - init_sp)*4; // get in bytes

    // no tasks yet
    if (stos_cur == NULL) {
        stos_cur   = task;
        task->next = stos_cur;
        return;
    }

    // find final task and append
    stos_tcb_t *temp = stos_cur;
    while (temp->next != stos_cur) temp = temp->next;
    
    temp->next = task;
    task->next = stos_cur;
}

void STOS_Schedule(void) {
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void STOS_Run(void) {
    stos_cur->func();
    for (;;) {;}
}

__attribute__ ((naked))
void pend_sv_handler(void) {
__asm volatile (
    "CPSID I                     \n"

    // Move MSP to R0 (save the current SP in R0)
    "MRS r0, msp                 \n"
    "PUSH {r4-r11}               \n"

    // stos_cur->sp = sp
    // set the stos_cur->sp to the current MSP value
    // (stos_cur in this context is what was previously running)
    "LDR   r1,=stos_cur          \n"
    "LDR   r1,[r1,#0x00]         \n"
    "STR   r0,[r1,#0x00]         \n"

    // stos_cur = stos_cur->next
    "LDR   r0, =stos_cur         \n"
    "LDR   r1, [r0]              \n"
    "LDR   r1, [r1, #4]          \n"
    "STR   r1, [r0]              \n"

    // sp = stos_cur->sp
    "LDR   r0, =stos_cur         \n"
    "LDR   r1, [r0]              \n"
    "LDR   r2, [r1]              \n"
    "MSR   msp, r2               \n"

    "POP {r4-r11}                \n"

    "CPSIE I                     \n"
    "BX    lr                    \n"
    );
}
