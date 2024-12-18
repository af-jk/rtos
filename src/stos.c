#include "stos.h"
#include "interrupts.h"

#define PENDSV_PRIORITY_Pos  0x4U   // PendSV priority is in the upper 4 bits of SHP[7]
#define PENDSV_PRIORITY_Mask (0xF << PENDSV_PRIORITY_Pos)  // Mask for the priority bits (upper 4 bits)
#define BYTE_ALIGN           0x7U

// Would it be best to put these within some sort of kernel struct to make it more compact?
static stos_list_t task_list;
static stos_tcb_t  *stos_next  = NULL; 
static stos_tcb_t  *stos_active = NULL; 
static stos_tcb_t  *stos_last = NULL;

static stos_tcb_t stos_idle_tcb;

void stos_idle_task(void) {

}

void STOS_Init (void (*handler)(void), uint32_t size) {
    uint8_t priority = 15U;
    priority = (priority & 0xFU) << PENDSV_PRIORITY_Pos;
    SCB->SHP[7] = priority;

    if (handler == NULL) { handler = &stos_idle_task; size = 0; }

    // Add idle task
    STOS_CreateTask(&stos_idle_tcb,
                    handler,
                    STOS_IDLE_DEFAULT_PRIORITY,
                    size);

    stos_next   = STOS_LIST_CONTAINER_GET_TCB(task_list.next, stos_tcb_t, list);
    stos_active = stos_next;


    SYSTICK_Config();
}

void STOS_CreateTask (stos_tcb_t *task,
                      void       (*handler)(void),
                      uint32_t   pri,   
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
    task->sp    = init_sp;
    task->func  = handler;
    task->pri   = pri;
    task->state = TASK_READY;

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

    STOS_AddTask(task);
}

void STOS_AddTask(stos_tcb_t *task) {
    stos_list_t *run;
    // iterate through task_list until we get back to task_list (it's circular)
    if (task_list.next == NULL && task_list.prev == NULL) {
        task_list.next = &(task->list);
        task_list.prev = &(task->list);

        task->list.next = &task_list;
        task->list.prev = &task_list;
        return;
    }

    for (run = task_list.next; run != &task_list; run = run->next) {
        // compare the priority of task we're adding with that of the runner
        stos_tcb_t *run_tcb = STOS_LIST_CONTAINER_GET_TCB(run, stos_tcb_t, list);
        if (task->pri >= run_tcb->pri) {

            // We want to set up the following: run->prev -> task -> run
            // Handle task
            task->list.prev = run->prev;
            task->list.next = run;

            // Handle run->prev
            run->prev->next = &(task->list);

            // Handle run
            run->prev = &(task->list);
            break;
        } 
    }

    // Couldn't find anything smaller so we append to the end there's definitely
    // some optimizations that can be made by using the priorities of the head and tail to
    // more quickly jump and insert at that location but that will be thought through and added later
    if (run == &task_list) {
        task->list.prev = run->prev;
        task->list.next = run;

        // Handle run->prev
        run->prev->next = &(task->list);

        // Handle run
        run->prev = &(task->list);

    }
}

void STOS_Schedule() {
    stos_list_t *run;
    stos_active = stos_next;

    // If there's no current task, start from the task_list head
    if (!stos_last) {
        stos_last = STOS_LIST_CONTAINER_GET_TCB(task_list.next, stos_tcb_t, list);
    }

    // List is sorted - since at the moment we don't support updating priority on the fly
    // we know it will remain that way. Looping through the tasks as such allows us to time-slice 
    // between tasks that share the same priority. A more elegant solution is in order
    // i.e. (if we can change priority >= and we'd need to worry about elements before stos_last)
    for (run = stos_last->list.next; run != &task_list; run = run->next) {
        stos_tcb_t *run_tcb = STOS_LIST_CONTAINER_GET_TCB(run, stos_tcb_t, list);

        // Check if the task has the same priority as the highest-priority task
        if (run_tcb->pri   == stos_active->pri &&
            run_tcb->state != TASK_RUNNING   &&
            run_tcb->state != TASK_SUSPENDED &&
            run_tcb->state != TASK_BLOCKED) {

            stos_next = run_tcb;
            stos_next->state = TASK_RUNNING;
            stos_active->state = TASK_READY;

            stos_last = stos_next;

            break;
        }
    }

    // We don't check the items between head and stos_last->list.next in the previous loop, thus if we couldn't
    // find our desired task, we want to check the entire list once more
    if (run == &task_list) {
        for (run = task_list.next; run != &task_list; run = run->next) {
            stos_tcb_t *run_tcb = STOS_LIST_CONTAINER_GET_TCB(run, stos_tcb_t, list);

            if (run_tcb->pri   == stos_active->pri &&
                run_tcb->state != TASK_RUNNING   &&
                run_tcb->state != TASK_SUSPENDED &&
                run_tcb->state != TASK_BLOCKED) {

                stos_next 		   = run_tcb;
                stos_next->state   = TASK_RUNNING;
                stos_active->state = TASK_READY;

                stos_last = stos_next;

                break;
            }
        }
    }

    // Only trigger a context switch if a new task is selected
    if (stos_next != stos_active) {
        SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
    }
}

__attribute__((naked))
void STOS_Run(void) {
	__asm volatile(
        " LDR   r1, =stos_next       \n"
        " LDR   r0, [r1, #0x00]     \n"
        " LDR   r1, [r0, #0x00]     \n"
		// The current stos_next points to the end of stack registers
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
		// we need to further push registers r11-r4 and then update the stos_active->sp
		" MRS	r0, psp			\n"
		" STMDB r0!, {r4-r11}	\n"
		" MSR	psp, r0			\n"

        // Load stos_active->sp to save registers
        " LDR   r1, =stos_active \n"
        " LDR   r2, [r1, #0x00] \n" // r2 = stos_active
		" STR   r0,	[r2, #0x00]	\n" // stos_active->sp = psp

        // Done with stos_active (now need to handle stos_next)
        " LDR   r1, =stos_next  \n" //
        " LDR   r2, [r1, #0x00] \n" // r2 = stos_next
        " LDR   r1, [r2, #0x00] \n" // r1 = stos_next->sp

        // Pop r11-r4 from stack into registers
        " LDMIA r1!, {r4-r11}   \n"

        // Update the PSP to be the new R0 value
		" MSR	psp, r1			\n" // psp = end of stack frame (r0 - literally (xPSR, RA, LR, R1, R3, R2, R1, R0 <---))

		" CPSIE	I				\n"
		" BX	lr				\n"
    );
}
