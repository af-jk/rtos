#include "stos.h"
#include "interrupts.h"

#define PENDSV_PRIORITY_Pos  0x4U   // PendSV priority is in the upper 4 bits of SHP[7]
#define PENDSV_PRIORITY_Mask (0xF << PENDSV_PRIORITY_Pos)  // Mask for the priority bits (upper 4 bits)
#define BYTE_ALIGN           0x7U

static stos_tcb_t *stos_cur = NULL;

void STOS_Init (void) {
    //NVIC_SetPri(pend_sv_IRQn, 15U);
    uint8_t priority = 15U;
    priority = (priority & 0xFU) << PENDSV_PRIORITY_Pos;
    SCB->SHP[7] = priority;

    SYSTICK_Config();
}

void STOS_CreateTask (stos_tcb_t *task,
                      void       (*handler)(void),
                      uint32_t   size) {

    uint32_t *psp;
    __asm volatile (" MRS   %[psp_var], psp    \n"
                    : [psp_var] "=r" (psp)
                    : 
                    :
    );
    uint32_t *init_sp = (uint32_t *)((uint32_t)psp);

    // When the task first starts, we're going to pre-fill the stack frame as such
    // PC and LR are required, the others are filled in just for easier debugging
    *(--init_sp) = (1U << 24);        // xPSR
    *(--init_sp) = (uint32_t)handler; // PC
    *(--init_sp) = 0x0000000EU;       // LR - thread doesn't return (infinite loop so this doesn't matter)
    *(--init_sp) = 0x0000000CU;       // R12
    *(--init_sp) = 0x00000003U;       // R3
    *(--init_sp) = 0x00000002U;       // R2
    *(--init_sp) = 0x00000001U;       // R1
    *(--init_sp) = 0x00000000U;       // R0

    // Save R11-R4 as well (these are just filler as a reminder)
    *(--init_sp) = 0x0000000BU;       // R11
    *(--init_sp) = 0x0000000AU;       // R10
    *(--init_sp) = 0x00000009U;       // R9
    *(--init_sp) = 0x00000008U;       // R8
    *(--init_sp) = 0x00000007U;       // R7
    *(--init_sp) = 0x00000006U;       // R6
    *(--init_sp) = 0x00000005U;       // R5
    *(--init_sp) = 0x00000004U;       // R4
    
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

    // update PSP to work for next allocation
    __asm volatile ("MSR psp, %0" : : "r" (init_sp) );

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

__attribute__((naked))
void STOS_Run(void) {
	__asm volatile(
        " LDR   r1, =stos_cur       \n"
        " LDR   r0, [r1, #0x00]     \n"
        " LDR   r1, [r0, #0x00]     \n"
		// The current stos_cur points to the end of stack registers
		// which includes stack frame and then r11-r4
		// to be able to exit into our desired function we need to add
		// to the psp value the content between r11-r4 toget to r0 where
		// the processor can by itself return us into the desired function
		" ADD	r1, r1, #32			\n"
        " MSR   psp, r1             \n"
        " SVC   #0                  \n"

        " l:                        \n"
        " NOP                       \n"
        " B l                       \n"
    );
}

__attribute__ ((naked))
void pend_sv_handler(void) {
	__asm volatile(
        " CPSID     I           \n"

		// When entering the exception, the processor pushes stack frame onto stack
		// xPSR, RA, LR, R12, R3-R0 and will update the PSP
		// we need to further push registers r11-r4 and then update the stack_cur->sp
		" MRS	r0, psp			\n"
		" STMDB r0!, {r4-r11}	\n"
		" MSR	psp, r0			\n"

        // Load stos_cur->sp to save registers
        " LDR   r1, = stos_cur  \n"
        " LDR   r2, [r1, #0x00] \n" // r2 = stos_cur
		" STR   r0,	[r2, #0x00]	\n" // stos_cur->sp = psp

        // Done with stos_cur (now need to handle stos_next)
        // Switch stos_cur to stos_next (stos_cur will become = stos_next)
        " LDR   r1, [r2, #0x04] \n" // r1 = stos_cur->next
        " LDR   r2, [r1, #0x00] \n" // r2 = stos_cur->next->sp

		/*
        // Set the psp to be the stos_cur->next->sp (which should point to the end of the stack frame + r11-r4)
		" MSR	psp, r2			\n" // psp = stos_cur->next->sp
        " MRS   r0, psp         \n" // r0 = psp
        */

        // Pop r11-r4 from stack into registers
        " LDMIA r2!, {r4-r11}   \n"

        // Update the PSP to be the new R0 value
		" MSR	psp, r2			\n" // psp = end of stack frame (r0 - literally (xPSR, RA, LR, R1, R3, R2, R1, R0 <---))

        // Set stos_cur = stos_cur->next and stos_cur->next = stos_cur
        " LDR   r0, = stos_cur  \n" // stos_cur, r1 already has stos_cur->next
        " STR   r1, [r0, #0x00] \n" // store stos_cur into stos_cur->next

		" CPSIE	I				\n"
		" BX	lr				\n"
    );
}
