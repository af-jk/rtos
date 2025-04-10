#include "stos.h"
#include "interrupts.h"

static stos_kernel_t stos_ker;

void STOS_CreateTask(stos_tcb_t * const task, void (*handler)(void), uint32_t pri,
                     uint32_t size) {

    if (task == NULL || handler == NULL) return;

    /* 
    Need to ensure 8-byte alignment on stack - per ARM ATPCS.

    Ex: stack (this example grows opp. of Cortex stack - it would go from high to low)
    0x0000 0000 // first doubleword
    0x0000 0004 // first doubleword
    0x0000 0008 // second double word
    0x0000 000c // second double word
    0x0000 0010 // third double word
    0x0000 0014 // third double word
    0x0000 0018 // and so on
    ...

    Practically speaking, the last three bits (8) need to be masked 0x.... .000

    However, the push and pop instruction operates on word alignment. Thus, the registers 
    need to be separated by four bytes, but the initial access and stack pointer saved at the end
    need to be double word aligned.
    */

    // Can assume this is always going to be valid
    uint32_t *psp;
    __asm volatile(" MRS   %[psp_var], psp    \n" : [psp_var] "=r"(psp) : :);
    uint32_t *init_sp = (uint32_t *)((uint32_t)psp);

    *(--init_sp) = (1U << 24);          // xPSR
    *(--init_sp) = (uint32_t)handler;   // PC
    *(--init_sp) = 0x0000000EU;         // LR - thread doesn't return (infinite loop so
                                        // this doesn't matter)
    *(--init_sp) = 0x0000000CU;         // R12
    *(--init_sp) = 0x00000003U;         // R3
    *(--init_sp) = 0x00000002U;         // R2
    *(--init_sp) = 0x00000001U;         // R1
    *(--init_sp) = 0x00000000U;         // R0

    // Save R11-R4 as well (these are just filler as a reminder)
    *(--init_sp) = 0x0000000BU;         // R11
    *(--init_sp) = 0x0000000AU;         // R10
    *(--init_sp) = 0x00000009U;         // R9
    *(--init_sp) = 0x00000008U;         // R8
    *(--init_sp) = 0x00000007U;         // R7
    *(--init_sp) = 0x00000006U;         // R6
    *(--init_sp) = 0x00000005U;         // R5
    *(--init_sp) = 0x00000004U;         // R4

    /* Now we update the local sp copy, which this is the new top of the stack. This is 
    aligned because we pushed 16 elements (if it was odd, we'd need to further ensure alignment) */

    task->sp    = init_sp;
    task->func  = handler;
    task->pri   = pri;

    // Fill in what remains of size - 16 (size of stack frame) w/ pre-set val
    uint32_t *region_end = init_sp - size;
    while (init_sp > region_end) {
        *(--init_sp) = 0xDEADBEEF;
    }

    // Ensure eight byte alignment
    while (((uint32_t)init_sp & BYTE_ALIGN) != 0) {
        *(--init_sp) = 0xFEEBDAEDU;
        region_end--;
    }

    /* Now, let's add a corruption region to the end of the stack frame. Purpose is to
    have this always be a fixed value that exists outside the functions allocated
    stack and to check it's content during a context switch to ensure it hasn't 
    been modified.
    
    As a note the STACK_CORRUPT_REGION_SIZE should always be a multiple of two to ensure
    and eight-byte stack alignment

    */
    for (uint32_t i = 0; i < STACK_CRPT_DETECT_REG_SIZE*2; i++) {
        *(--init_sp) = STACK_CRPT_DETECT_SEQ;
    }

    // Save the first address of the corruption region
    task->stack_end = region_end;

    // update PSP to work for next allocation
    __asm volatile("MSR psp, %0" : : "r"(init_sp));

    STOS_AddTask(task, TASK_READY);
}

void STOS_AddTask(stos_tcb_t * const task, uint32_t state) {
    if (task == NULL) return;

    if (state == TASK_READY) {
        stos_tcb_t **head = &(stos_ker.list_ready_head);

        task->state = state;

        if (*head == NULL) {
           *head = task; 
           task->prev = NULL;
           task->next = NULL;
           return;
        }

        if (task->pri > (*head)->pri) {
           task->next = *head;
           task->prev = NULL;
           (*head)->prev = task;
           *head = task;
           return;
        }

        stos_tcb_t *runner = stos_ker.list_ready_head;
        while (runner->next != NULL && task->pri <= runner->next->pri) {
            runner = runner->next;
        }

        // At this point we're either at the end or at a priority lower than the current task

        head = &runner;

        task->next = (*head)->next;
        task->prev = (*head);
        (*head)->next->prev = task;
        (*head)->next = task;
        return;
    }

    if (state == TASK_BLOCKED) {
        stos_tcb_t **head = &(stos_ker.list_blocked_head);

        task->state = state;

        if (*head == NULL) {
           *head = task; 
           task->prev = NULL;
           task->next = NULL;
           return;
        }

        if (task->timeout < (*head)->timeout) {
           task->next = *head;
           task->prev = NULL;
           (*head)->prev = task;
           *head = task;
           return;
        }

        stos_tcb_t *runner = stos_ker.list_blocked_head;
        while (runner->next != NULL && task->timeout >= runner->next->timeout) {
            runner = runner->next;
        }

        // At this point we're either at the end or at a priority lower than the current task

        head = &runner;

        task->next = (*head)->next;
        task->prev = (*head);
        (*head)->next->prev = task;
        (*head)->next = task;
        return;
    }
}

void STOS_RemoveTask(stos_tcb_t * const task) {
    if (task == NULL) return;

    stos_tcb_t **head = &(stos_ker.list_ready_head);

    if (task->state == TASK_BLOCKED) {
        head = &(stos_ker.list_blocked_head);
    }

    if (*head == NULL) return;

    stos_tcb_t *runner = *head;
    while (runner->next != NULL && (runner != task)) {
        runner = runner->next;
    }

    head = &runner;

    // Removing from top
    if ((*head)->prev == NULL) {

        (*head)->next->prev = NULL;

        if (task->state == TASK_READY) {
            stos_ker.list_ready_head = (*head)->next;
        } else {
            stos_ker.list_blocked_head = (*head)->next;
        }

        (*head)->next = NULL;
        return;
    }

    // Removing from bottom
    if ((*head)->next == NULL) {
        (*head)->prev->next = NULL;
        (*head)->prev = NULL;
        return;
    }

    // Any other case
    (*head)->prev->next = (*head)->next;
    (*head)->next->prev = (*head)->prev;

    (*head)->next = NULL;
    (*head)->prev = NULL;
}

// Set active task (which is in running state) to blocked state for timeout amt
void STOS_TimeoutTask(uint32_t timeout) {
    stos_ker.active_task->timeout = timeout;
    STOS_AddTask(stos_ker.active_task, TASK_BLOCKED);
    STOS_Schedule();
}


__attribute__((naked)) static void STOS_Launch(void) {
    __asm volatile(
        " LDR   r1, =stos_ker       \n"
        " LDR   r0, [r1, #0x0C]     \n" // set r0 =stos_ker.active_task
        " LDR   r1, [r0, #0x00]     \n" // set r1 = the sp of the active task
        // The current stos_next points to the end of stack registers
        // which includes stack frame and then r11-r4
        // to be able to exit into our desired function we need to add
        // to the psp value the content between r11-r4 toget to r0 where
        // the processor can by itself return us into the desired function
        " ADD	r1, r1, #32         \n" // adjust SP of active task to account for frame
        " MSR   psp, r1             \n"
        " SVC   #0                  \n"

        " l:                        \n"
        " NOP                       \n"
        " B l                       \n");
}

void STOS_Run(void (*handler)(void), uint32_t size) {
    PendSV_SetPri(IRQ_MIN_PRI);
    SysTick_SetPri(IRQ_MIN_PRI - 1);

    if (handler == NULL) {
        handler = &STOS_IdleTask;
        size = 0;
    }

    // Add idle task
    STOS_CreateTask(stos_ker.idle_task, handler, STOS_IDLE_DEFAULT_PRIORITY, size);

    // Set active_task to be the highest priority task and remove that highest
    // priority task from the list since it's now the active one
    stos_ker.active_task = stos_ker.list_ready_head;
    STOS_RemoveTask(stos_ker.active_task);
    stos_ker.active_task->state = TASK_RUNNING;

    SysTick_Config();

    STOS_Launch();
}

void STOS_Schedule() {
    stos_tcb_t *ready_task_head = stos_ker.list_ready_head;

    if (ready_task_head == NULL) return; // should only happen if all other tasks are blocked
    // and we are currently in the idle task

    // If the current task has been blocked, switch to the next highest priority
    if (stos_ker.active_task->state == TASK_BLOCKED) {
        STOS_RemoveTask(ready_task_head);

        stos_ker.next_task = ready_task_head;
        stos_ker.next_task->state = TASK_RUNNING;

        SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
        return;
    }

    // At this point stos_active must be in the RUNNING state, if it's still the 
    // highest priority don't reschedule
    if (ready_task_head->pri < stos_ker.active_task->pri) return;

    STOS_RemoveTask(ready_task_head);

    // Set stos next to the highest priority task of the ready list
    stos_ker.next_task           = ready_task_head;
    stos_ker.next_task->state    = TASK_RUNNING;

    STOS_AddTask(stos_ker.active_task, TASK_READY);

    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void svc_handler(void) {
    __asm volatile(
        " MOV   LR, #0xFFFFFFFD \n"
        " BX    LR              \n");
}

/* Results: 
0 -> no corruption
1 -> corruption
*/
static uint32_t STOS_CheckTaskCorruption(stos_tcb_t *task) {
    uint32_t *end_sp = task->stack_end;

    for (uint32_t i = 0; i < STACK_CRPT_DETECT_REG_SIZE*2; i++) {
        if (*(--end_sp) != STACK_CRPT_DETECT_SEQ) {
            return 1;
        }
    }

    return 0;
}

void sys_tick_handler(void) { 
    stos_tcb_t *runner = stos_ker.list_blocked_head;

    /* Check for stack corruption in the active task, if there is corruption do not schedule
    and instead proceed to default fault */
    if (STOS_CheckTaskCorruption(stos_ker.active_task)) {
        // Manually trigger a usage fault
        volatile uint32_t fault = 1;
        fault = 1/0;

        // Should never get here
        for (;;);
    }

    while (runner != NULL) {
        stos_tcb_t **head = &runner;
        (*head)->timeout--;
        if ((*head)->timeout == 0) {
            STOS_RemoveTask((*head));
            STOS_AddTask((*head), TASK_READY);
        }
        runner = runner->next;
    }

    STOS_Schedule(); 
}

__attribute__((naked)) void pend_sv_handler(void) {
    __asm volatile(
        " CPSID     I               \n"

        // When entering the exception, the processor pushes stack frame onto
        // stack xPSR, RA, LR, R12, R3-R0 and will update the PSP.
        // we need to further push registers r11-r4 and then update the active_task->sp

        // Store the context of the active_task
        " MRS	r0, psp             \n" // Store PSP in r0
        " STMDB r0!, {r4-r11}       \n" // push r4-r11 onto stack based on r0 (r0 gets updated to new stack addr)
        " MSR	psp, r0             \n" // update psp to be the new stack addr

        // Update the sp of the active task after storing context
        " LDR   r1, =stos_ker       \n"
        " LDR   r2, [r1, #0x0C]     \n" // r2 = active_task
        " STR 	r0, [r2, #0x00]		\n" // active_task->sp = psp

        // Switch desired SP to be that of the next task
    	" LDR 	r0, = stos_ker      \n"
        " LDR   r3, [r0, #0x08]     \n" // r3 = next_task
        " LDR   r1, [r3, #0x00]     \n" // r1 = next_task->sp

        // Set the active task as the next task, optionally null out the next task
        " STR   r3, [r0, #0x0C]     \n" // store r3 (next_task) to addr of active_task
        " MOV   r4, 0               \n"
        " STR   r4, [r0, #0x08]     \n" // store r4 (null) to addr of next_task 

        // Pop the context of the "next task" (now active task) r11-r4 from stack into registers
        " LDMIA r1!, {r4-r11}       \n"

        // Update the PSP of the "next task (now active task)" to be the new R0 value
        " MSR	psp, r1             \n" // psp = end of stack frame (r0
                                        // - (xPSR, RA, LR,
                                        // R1, R3, R2, R1, R0 <---))

        " CPSIE	I                   \n"
        // Return to context of active task
        " BX	lr                  \n");
}


void STOS_IdleTask(void) {
	for (;;);
}