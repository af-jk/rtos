#include "stos.h"
#include "interrupts.h"

static stos_tcb_t *stos_cur = NULL;
uint32_t cur_sp = 0;

void STOS_Init (void) {
    NVIC_SetPri(pend_sv_IRQn, 15U);
}

void STOS_CreateTask (stos_tcb_t *task,
                      void       (*handler)(void),
                      uint32_t   *stk,
                      uint32_t   size) {
    uint32_t *sp = (uint32_t *)((((uint32_t)stk + size) + 7) & ~0x7U);

    // When the task first starts, we're going to pre-fill the stack frame as such
    // PC and LR are required, the others are filled in just for easier debugging
    *(--sp) = (1U << 24);        // xPSR
    *(--sp) = (uint32_t)handler; // PC
    *(--sp) = 0x0000000DU;       // LR - needs to be updated for PSP/MSP
    *(--sp) = 0x0000000CU;       // R12
    *(--sp) = 0x0000000BU;       // R11
    *(--sp) = 0x0000000AU;       // R10
    *(--sp) = 0x00000009U;       // R9
    *(--sp) = 0x00000008U;       // R8
    *(--sp) = 0x00000007U;       // R7
    *(--sp) = 0x00000006U;       // R6
    *(--sp) = 0x00000005U;       // R5
    *(--sp) = 0x00000004U;       // R4
    *(--sp) = 0x00000003U;       // R3
    *(--sp) = 0x00000002U;       // R2
    *(--sp) = 0x00000001U;       // R1
    *(--sp) = 0x00000000U;       // R0
    
    // update sp
    task->sp = sp;
    task->func = handler;

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

/*
void STOS_Schedule(void) {
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}
*/

void STOS_Run(void) {
    stos_cur->func();
    for (;;) {;}
}

__attribute__ ((naked))
void pend_sv_handler(void) {
__asm volatile (
    "CPSID I                     \n"

    "MRS r0, msp                 \n"
    "PUSH {r4-r11}               \n"

    // stos_cur->sp = sp
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