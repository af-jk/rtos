#include "stos.h"
#include "interrupts.h"

#define PENDSV_PRIORITY_Pos \
    0x4U  // PendSV priority is in the upper 4 bits of SHP[7]
#define PENDSV_PRIORITY_Mask \
    (0xF << PENDSV_PRIORITY_Pos)  // Mask for the priority bits (upper 4 bits)

static stos_kernel_t stos_ker;

void stos_idle_task(void) {}

void STOS_CreateTask(stos_tcb_t * const task, void (*handler)(void), uint32_t pri,
                     uint32_t size) {
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

    // update sp -- this is the new top of the stack
    task->sp    = init_sp;
    task->func  = handler;
    task->pri   = pri;

    // Fill in what remains of size - 16 (size of stack frame) w/ pre-set val
    uint32_t *region_end = init_sp - size + 4;
    while (init_sp > region_end) {
        *(--init_sp) = 0xDEADBEEF;
    }

    // maybe overkill, but ensure alignment
    while (((uint32_t)init_sp & BYTE_ALIGN) != 0) {
        *(--init_sp) = 0xFEEBDEADU;
    }

    // update PSP to work for next allocation
    __asm volatile("MSR psp, %0" : : "r"(init_sp));

    STOS_AddTask(task, TASK_READY);
}

void STOS_AddTask(stos_tcb_t * const task, uint32_t state) {
    stos_tcb_t **head = &(stos_ker.list_ready_head);
    if (state == TASK_BLOCKED) {
        head = &(stos_ker.list_blocked_head);
    }

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
    while (runner->next != NULL && TASK_CONDITION(task, runner, task->state)) {
        runner = runner->next;
    }

    // At this point we're either at the end or at a priority lower than the current task

    head = &runner;
    if (runner->next == NULL && TASK_CONDITION(task, runner, task->state)) {
        // Make the last task of this list the task we are adding
        (*head)->next = task;
        // Make the next task of the list be the end
        task->next = NULL;
        // Ensure last task points to previous one
        task->prev = (*head);
        return;
    }
    task->next = (*head)->next;
    task->prev = (*head);
    (*head)->next->prev = task;
    (*head)->next = task;
}

void STOS_RemoveTask(stos_tcb_t * const task) {
    stos_tcb_t **head = &(stos_ker.list_ready_head);

    if (task->state == TASK_BLOCKED) {
        head = &(stos_ker.list_blocked_head);
    }

    stos_tcb_t *runner = *head;
    while (runner->next != NULL && (runner != task)) {
        runner = runner->next;
    }

    head = &runner;

    // Removing from top
    if ((*head)->prev == NULL) {
        (*head)->next->prev = NULL;
        stos_ker.list_ready_head = (*head)->next;
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

// Set active task to blocked state for timeout amt
void STOS_TimeoutTask(uint32_t timeout) {
    STOS_RemoveTask(stos_ker.active_task);
    stos_ker.active_task->timeout = timeout;
    STOS_AddTask(stos_ker.active_task, TASK_BLOCKED);
}

void STOS_Init(void (*handler)(void), uint32_t size) {
    uint8_t priority = 15U;
    priority = (priority & 0xFU) << PENDSV_PRIORITY_Pos;
    SCB->SHP[7] = priority;

    if (handler == NULL) {
        handler = &stos_idle_task;
        size = 0;
    }

    // Add idle task
    STOS_CreateTask(&stos_ker.idle_task, handler, STOS_IDLE_DEFAULT_PRIORITY, size);

    // Set stos_active to be the highest priority task and remove that highest
    // priority task from the list since it's now the active one
    stos_ker.active_task = stos_ker.list_ready_head;
    STOS_RemoveTask(stos_ker.active_task);
    stos_ker.active_task->state = TASK_RUNNING;

    SYSTICK_Config();
}

void STOS_Schedule() {
    stos_tcb_t *ready_task_head = stos_ker.list_ready_head;

    // If the current task has been blocked, switch to the next highest priority
    if (stos_ker.active_task->state == TASK_BLOCKED) {
        stos_ker.next_task = ready_task_head;
        STOS_RemoveTask(ready_task_head);
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

__attribute__((naked)) void STOS_Run(void) {
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

void svc_handler(void) {
    __asm volatile(
        "MOV LR, #0xFFFFFFFD \n"
        "BX LR \n");
}

void sys_tick_handler(void) { 
    /*
    stos_list_t *run;
    for (run = stos_blocked_list.next; run != &stos_blocked_list; run = run->next) {
        stos_tcb_t *run_tcb = STOS_LIST_CONTAINER_GET_TCB(run, stos_tcb_t, list);
        run_tcb->timeout--;

        if (run_tcb->timeout == 0) {
            // Switch to ready and remove from blocked
            STOS_RemoveTask(run_tcb, &stos_blocked_list);
            // Have a bug when adding task
            STOS_AddTask(run_tcb, &stos_ready_list);
            run_tcb->state = TASK_READY;
        }
    }
    */
    STOS_Schedule(); 
}

__attribute__((naked)) void pend_sv_handler(void) {
    __asm volatile(
        " CPSID     I               \n"

        // When entering the exception, the processor pushes stack frame onto
        // stack xPSR, RA, LR, R12, R3-R0 and will update the PSP.
        // we need to further push registers r11-r4 and then update the active_task->sp

        " MRS	r0, psp             \n" // Store old PSP in r0
        " STMDB r0!, {r4-r11}       \n" // push r4-r11 onto stack based on r0 (r0 gets updated to new stack addr)
        " MSR	psp, r0             \n" // update psp to be the new stack addr

        " LDR   r1, =stos_ker       \n"
        " LDR   r2, [r1, #0x0C]     \n" // r2 = active_task
        " LDR   r3, [r2, #0x00]     \n" // r3 = active_task->sp
        " STR 	r0, [r3, #0x00]		\n" // active_task->sp = psp

    	" LDR 	r0, = stos_ker		\n"
        " LDR   r3, [r0, #0x08]     \n" // r3 = next_task
        " LDR   r1, [r3, #0x00]     \n" // r1 = next_task->sp

        " STR   r3, [r0, #0x0C]     \n" // store r3 (next_task) to addr of active_task
        " MOV   r4, 0               \n"
        " STR   r4, [r0, #0x08]     \n" // store r4 (null) to addr of next_task 

        // Pop r11-r4 from stack into registers
        " LDMIA r1!, {r4-r11}       \n"

        // Update the PSP to be the new R0 value
        " MSR	psp, r1             \n" // psp = end of stack frame (r0
                                        // - (xPSR, RA, LR,
                                        // R1, R3, R2, R1, R0 <---))

        " CPSIE	I                   \n"
        " BX	lr                  \n");
}
