.syntax unified
.cpu    cortex-m4
.fpu    softvfp
.thumb

.global vectors

.word __si_data
.word __s_data
.word __e_data
.word __s_bss
.word __e_bss

    .section .text._start
    .global _start
    .type _start, %function
_start:
    ldr     sp, =_estack
    ldr     r0, =__s_data
    ldr     r1, =__e_data
    ldr     r2, =__si_data
    movs    r3, #0
    b       CompareDataAddress

CopyDataLoop:
    ldr     r4, [r2, r3]
    str     r4, [r0, r3]
    adds    r3, r3, #4

CompareDataAddress:
    adds    r4, r0, r3
    cmp     r4, r1
    bcc     CopyDataLoop

    ldr     r1, =__s_bss
    ldr     r2, =__e_bss
    movs    r3, #0
    b       CompareBssAddress

ZeroBssLoop:
    str     r3, [r1]
    adds    r1, r1, #4

CompareBssAddress:
    cmp     r1, r2
    bcc     ZeroBssLoop

    bl      main
    bx      lr
.size _start, .-_start

    .section .text.default_handler, "ax", %progbits
default_handler:
Infinite_Loop:
    b Infinite_Loop
.size default_handler, .-default_handler

    .section .rodata.vectors
vectors:
    // Arm Exceptions
    .word _estack
    .word _start
    .word nmi_handler
    .word hard_flt_handler
    .word mem_mg_handler 
    .word bus_flt_handler
    .word usage_flt_handler
    .word 0
    .word 0
    .word 0
    .word 0
    .word svc_handler
    .word debug_mon_handler
    .word 0
    .word pend_sv_handler
    .word sys_tick_handler

    // STM IRQs : setting to 0 for now
    .word 0 /* Window WatchDog                              */                                        
    .word 0 /* PVD through EXTI Line detection              */                        
    .word 0 /* Tamper and TimeStamps through the EXTI line  */            
    .word 0 /* RTC Wakeup through the EXTI line             */                      
    .word 0 /* FLASH                                        */                                          
    .word 0 /* RCC                                          */                                            
    .word 0 /* EXTI Line0                                   */                        
    .word 0 /* EXTI Line1                                   */                          
    .word 0 /* EXTI Line2                                   */                          
    .word 0 /* EXTI Line3                                   */                          
    .word 0 /* EXTI Line4                                   */                          
    .word 0 /* DMA1 Stream 0                                */                  
    .word 0 /* DMA1 Stream 1                                */                   
    .word 0 /* DMA1 Stream 2                                */                   
    .word 0 /* DMA1 Stream 3                                */                   
    .word 0 /* DMA1 Stream 4                                */                   
    .word 0 /* DMA1 Stream 5                                */                   
    .word 0 /* DMA1 Stream 6                                */                   
    .word 0 /* ADC1, ADC2 and ADC3s                         */                   
    .word 0 /* CAN1 TX                                      */                         
    .word 0 /* CAN1 RX0                                     */                          
    .word 0 /* CAN1 RX1                                     */                          
    .word 0 /* CAN1 SCE                                     */                          
    .word 0 /* External Line[9:5]s                          */                          
    .word 0 /* TIM1 Break and TIM9                          */         
    .word 0 /* TIM1 Update and TIM10                        */         
    .word 0 /* TIM1 Trigger and Commutation and TIM11       */
    .word 0 /* TIM1 Capture Compare                         */                          
    .word 0 /* TIM2                                         */                   
    .word 0 /* TIM3                                         */                   
    .word 0 /* TIM4                                         */                   
    .word 0 /* I2C1 Event                                   */                          
    .word 0 /* I2C1 Error                                   */                          
    .word 0 /* I2C2 Event                                   */                          
    .word 0 /* I2C2 Error                                   */                            
    .word 0 /* SPI1                                         */                   
    .word 0 /* SPI2                                         */                   
    .word 0 /* USART1                                       */                   
    .word 0 /* USART2                                       */                   
    .word 0 /* USART3                                       */                   
    .word 0 /* External Line[15:10]s                        */                          
    .word 0 /* RTC Alarm (A and B) through EXTI Line        */                 
    .word 0 /* USB OTG FS Wakeup through EXTI line          */                       
    .word 0 /* TIM8 Break and TIM12                         */         
    .word 0 /* TIM8 Update and TIM13                        */         
    .word 0 /* TIM8 Trigger and Commutation and TIM14       */
    .word 0 /* TIM8 Capture Compare                         */                          
    .word 0 /* DMA1 Stream7                                 */                          
    .word 0 /* FMC                                          */                   
    .word 0 /* SDIO                                         */                   
    .word 0 /* TIM5                                         */                   
    .word 0 /* SPI3                                         */                   
    .word 0 /* UART4                                        */                   
    .word 0 /* UART5                                        */                   
    .word 0 /* TIM6 and DAC1&2 underrun errors              */                   
    .word 0 /* TIM7                                         */
    .word 0 /* DMA2 Stream 0                                */                   
    .word 0 /* DMA2 Stream 1                                */                   
    .word 0 /* DMA2 Stream 2                                */                   
    .word 0 /* DMA2 Stream 3                                */                   
    .word 0 /* DMA2 Stream 4                                */                   
    .word 0 /* Reserved                                     */                   
    .word 0 /* Reserved                                     */                     
    .word 0 /* CAN2 TX                                      */                          
    .word 0 /* CAN2 RX0                                     */                          
    .word 0 /* CAN2 RX1                                     */                          
    .word 0 /* CAN2 SCE                                     */                          
    .word 0 /* USB OTG FS                                   */                   
    .word 0 /* DMA2 Stream 5                                */                   
    .word 0 /* DMA2 Stream 6                                */                   
    .word 0 /* DMA2 Stream 7                                */                   
    .word 0 /* USART6                                       */                    
    .word 0 /* I2C3 event                                   */                          
    .word 0 /* I2C3 error                                   */                          
    .word 0 /* USB OTG HS End Point 1 Out                   */                   
    .word 0 /* USB OTG HS End Point 1 In                    */                   
    .word 0 /* USB OTG HS Wakeup through EXTI               */                         
    .word 0 /* USB OTG HS                                   */                   
    .word 0 /* DCMI                                         */                   
    .word 0 /* Reserved                                     */                   
    .word 0 /* Reserved                                     */
    .word 0 /* FPU                                          */
    .word 0 /* Reserved                                     */
    .word 0 /* Reserved                                     */
    .word 0 /* SPI4                                         */
    .word 0 /* Reserved                                     */
    .word 0 /* Reserved                                     */
    .word 0 /* SAI1                                         */
    .word 0 /* Reserved                                     */
    .word 0 /* Reserved                                     */
    .word 0 /* Reserved                                     */
    .word 0 /* SAI2                                         */
    .word 0 /* QuadSPI                                      */
    .word 0 /* CEC                                          */
    .word 0 /* SPDIF RX                                     */
    .word 0 /* FMPI2C 1 Event                               */
    .word 0 /* FMPI2C 1 Error                               */
.size  vectors, .-vectors

    .weak       nmi_handler
    .thumb_set  nmi_handler, default_handler

    .weak       hard_flt_handler
    .thumb_set  hard_flt_handler, default_handler

    .weak       mem_mg_handler
    .thumb_set  mem_mg_handler, default_handler

    .weak       bus_flt_handler
    .thumb_set  bus_flt_handler, default_handler

    .weak       usage_flt_handler
    .thumb_set  usage_flt_handler, default_handler

    .weak       svc_handler
    .thumb_set  svc_handler, default_handler

    .weak       debug_mon_handler
    .thumb_set  debug_mon_handler, default_handler

    .weak       pend_sv_handler
    .thumb_set  pend_sv_handler, default_handler

    .weak       sys_tick_handler
    .thumb_set  sys_tick_handler, default_handler
