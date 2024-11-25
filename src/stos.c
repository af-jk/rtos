#include "stos.h"
#include "interrupts.h"

#define PENDSV_PRIORITY_Pos  0x4U   // PendSV priority is in the upper 4 bits of SHP[7]
#define PENDSV_PRIORITY_Mask (0xF << PENDSV_PRIORITY_Pos)  // Mask for the priority bits (upper 4 bits)
#define BYTE_ALIGN           0x7U

// Would it be best to put these within some sort of kernel struct to make it more compact?
static stos_list_t stos_ready_list; // ready tasks
static stos_list_t stos_blocked_list; // blocked tasks

/* Vision
 * stos_ready_list - tasks which are ready (sorted by priority - DOES NOT CONTAIN THE ACTIVE TASK!)
 * stos_block_list - tasks which are blocked (sorted by timeout)
 * stos_active     - task which is currently TASK_ACTIVE (should only ever have one task in this state!)
 * stos_next       - task which is going to be scheduled next:
 *                  1. Will be stos_active if ready list doesnt have any task with high enough priority
 *                  2. Will be top of ready if priority >= active
 * stos_last       - no longer necessary?
 */

static stos_tcb_t  *stos_next  = NULL; 
static stos_tcb_t  *stos_active = NULL; 
// static stos_tcb_t  *stos_last = NULL;

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


    // Set stos_active to be the highest priority task and remove that highest
    // priority task from the list since it's now the active one
    stos_active         = STOS_LIST_CONTAINER_GET_TCB(stos_ready_list.next, stos_tcb_t, list);
    stos_active->state  = TASK_RUNNING;
    STOS_RemoveTask(stos_active, &stos_ready_list);

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

    STOS_AddTask(task, &stos_ready_list);
}

void STOS_AddTask(stos_tcb_t *task, stos_list_t *task_list) {
    stos_list_t *run;

    // Handle base case - empty list
    if (task_list->next == NULL && task_list->prev == NULL) {
        task_list->next = &(task->list);
        task_list->prev = &(task->list);

        task->list.next = task_list;
        task->list.prev = task_list;
        return;
    }

    run = task_list->next; // run is the head of the task

    // Algorithm behind adding is to append to the end of sorted subsection
    // Ex: Appending 5 to following: 5, 5, 3, 2, 1
    // The appended 5 should go here: 5, 5, _, 3, 2, 1
    // With the intent to give tasks which haven't been scheduled yet priority
    stos_tcb_t *idx = NULL;
    if (task_list == &stos_ready_list) {

        for (run = task_list->next; run != task_list; run = run->next) {
            stos_tcb_t *run_tcb = STOS_LIST_CONTAINER_GET_TCB(run, stos_tcb_t, list);
            if (task->pri > run_tcb->pri) {
                idx = run_tcb;
                break;
            }
        }

        // insert before idx
        if (idx) {
            task->list.prev = run->prev;
            task->list.next = run;

            // Handle run->prev
            run->prev->next = &(task->list);

            // Handle run
            run->prev = &(task->list);

            return;
        } 

        // Insert at the end
        task->list.prev = run->prev;
        task->list.next = run;

        // Handle run->prev
        run->prev->next = &(task->list);

        // Handle run
        run->prev = &(task->list);
        return;
    }
    /*

    if (task_list == &stos_blocked_list) {

        for (run = task_list->next; run != task_list; run = run->next) {
            stos_tcb_t *run_tcb = STOS_LIST_CONTAINER_GET_TCB(run, stos_tcb_t, list);
            if (task->pri > run_tcb->pri) {
                idx = run_tcb;
                break;
            }
        }

        if (idx) {
            // insert before idx
            return;
        } 

        return
    }
    */
}

void STOS_RemoveTask(stos_tcb_t *task, stos_list_t *task_list) {
    stos_list_t *run;
    for (run = task_list->next; run != task_list; run = run->next) {
        stos_tcb_t *run_tcb = STOS_LIST_CONTAINER_GET_TCB(run, stos_tcb_t, list);
        if (task == run_tcb) {
            stos_list_t *prev = run->prev;
            stos_list_t *next = run->next;

            // Remove run from the chain
            prev->next = run->next;
            next->prev = run->prev;
        }
    }
}


void STOS_Schedule() {
    // The head of the stos_ready_list will be the task with the highest priority that has the longest without
    // being scheduled. This will be maintained by always adding tasks to be at the end of their respected ordered
    // priority section.
    // Ex: If i have three tasks w/ priority 8, and my active task has a priority of 8, when I perform this operation
    // I will pop my top task, and push the active one to the end of the tasks with priority 8.
    stos_tcb_t *ready_task_head = STOS_LIST_CONTAINER_GET_TCB(stos_ready_list.next, stos_tcb_t, list);

    if (ready_task_head->pri < stos_active->pri) return; // never call pend_sv because we don't need to switch tasks

    // Only get here if ready_task_head >= stos_active which means we want to switch tasks

    // Set stos next to the highest priority task of the ready list
    stos_next = ready_task_head;

    // Remove the new stos next from the ready list
    STOS_RemoveTask(ready_task_head, &stos_ready_list);

    // Set the state of the new task to running
    stos_next->state = TASK_RUNNING;

    // Update the active task to indicate that it is no longer being run
    stos_active->state = TASK_READY;

    // Put the currently active task back into the ready task list so that we can schedule it in the future
    STOS_AddTask(stos_active, &stos_ready_list);

    // Trigger PENDSV - should write wrapper for this in appropriate location
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

__attribute__((naked))
void STOS_Run(void) {
	__asm volatile(
        " LDR   r1, =stos_active    \n"
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

void svc_handler(void) {
    __asm volatile(
        "MOV LR, #0xFFFFFFFD \n"
        "BX LR \n"              
    );
}

void sys_tick_handler(void) {
    STOS_Schedule();
}

__attribute__ ((naked))
void pend_sv_handler(void) {
	__asm volatile(
        " CPSID     I               \n"

		// When entering the exception, the processor pushes stack frame onto stack
		// xPSR, RA, LR, R12, R3-R0 and will update the PSP
		// we need to further push registers r11-r4 and then update the stos_active->sp
		" MRS	r0, psp			    \n"
		" STMDB r0!, {r4-r11}       \n"
		" MSR	psp, r0			    \n"

        // Load stos_active->sp to save registers
        " LDR   r1, =stos_active    \n"
        " LDR   r2, [r1, #0x00]     \n" // r2 = stos_active
		" STR   r0,	[r2, #0x00]	    \n" // stos_active->sp = psp

        // Done with stos_active (now need to handle stos_next)
        " LDR   r1, =stos_next      \n" //
        " LDR   r2, [r1, #0x00]     \n" // r2 = stos_next
        " LDR   r1, [r2, #0x00]     \n" // r1 = stos_next->sp

        // Now that I'm done I need to maintain r1 and set stos_active = stos_next
        " LDR   r3, =stos_active    \n"
        " STR   r2, [r3, #0x00]     \n"

        // Pop r11-r4 from stack into registers
        " LDMIA r1!, {r4-r11}       \n"

        // Update the PSP to be the new R0 value
		" MSR	psp, r1			    \n" // psp = end of stack frame (r0 - literally (xPSR, RA, LR, R1, R3, R2, R1, R0 <---))

		" CPSIE	I				    \n"
		" BX	lr				    \n"
    );
}
