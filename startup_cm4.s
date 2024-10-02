.syntax unified
.cpu    cortex-m4
.fpu    softvfp // Cortex-M4 FPU
.thumb

.global vectors

.word __si_data // current location of data in sram
.word __s_data  // start of data in flash
.word __e_data  // end of data in flash
.word __s_bss   // start of bss in flash
.word __e_bss   // end of bss in flash

// executable instructions are stored in text,
// so need to create section within text for the start handler
  .section .text._start
  .global _start
  .type _start, %function
_start:
	ldr		sp, =_estack
	ldr 	r0, =__s_data
	ldr 	r1, =__e_data
  	ldr		r2, =__si_data
	movs 	r3, #0

    b 		CompareDataAddress

	// These are from STM, will update them with copy tables after verifying all else works
  CopyDataLoop:
	ldr		r4, [r2, r3] // load value stored at r2+r3 (si_data + offset) in r4
	str 	r4, [r0, r3] // store the value from r4 into the r0 + r3 (s_data + offset)
	adds	r3, r3, #4   // increment the offset by four and continue to CompareDataAddress

  CompareDataAddress:
	adds 	r4, r0, r3 // increment s_data by four each time and store in r4
	cmp		r4, r1	   // compare updated s_data with e_data, set carry if result is positive or zero
	bcc 	CopyDataLoop

	// At this point, we've finished copying the data segment over
	ldr 	r1, =__s_bss
	ldr 	r2, =__e_bss
	movs	r3, #0
	b 		CompareBssAddress	

  ZeroBssLoop:
	str 	r3, [r1]   // Store 0 into address of r1 (s_bss + offset)
	adds 	r1, r1, #4 // Offset r1 (s_bss) by 4

  CompareBssAddress:
	cmp  	r1, r2		// compare current s_bss + offset wiith e_bss
	bcc 	ZeroBssLoop // continue the loop

	bl 		main
	bx 		lr
  .size	_start, .-_start

.section .rodata.vectors
vectors:
    // ARM Exceptions
    .word _estack
    .word _start
    .word _nmi_handler
    .word _hardFlt_handler
    .word _busFlt_handler
    .word _usageFlt_handler
    .word 0                   /* reserved */
    .word 0                   /* reserved */
    .word 0                   /* reserved */
    .word 0                   /* reserved */
    .word _svc_handler
    .word _debugMon_handler
    .word 0                   /* reserved */
    .word _pendSV_handler
    .word _sysTick_handler

    // STM IRQs : setting to 0 for now
    .word 0 /* Window WatchDog              */                                        
    .word 0 /* PVD through EXTI Line detection */                        
    .word 0 /* Tamper and TimeStamps through the EXTI line */            
    .word 0 /* RTC Wakeup through the EXTI line */                      
    .word 0 /* FLASH                        */                                          
    .word 0 /* RCC                          */                                            
    .word 0 /* EXTI Line0                   */                        
    .word 0 /* EXTI Line1                   */                          
    .word 0 /* EXTI Line2                   */                          
    .word 0 /* EXTI Line3                   */                          
    .word 0 /* EXTI Line4                   */                          
    .word 0 /* DMA1 Stream 0                */                  
    .word 0 /* DMA1 Stream 1                */                   
    .word 0 /* DMA1 Stream 2                */                   
    .word 0 /* DMA1 Stream 3                */                   
    .word 0 /* DMA1 Stream 4                */                   
    .word 0 /* DMA1 Stream 5                */                   
    .word 0 /* DMA1 Stream 6                */                   
    .word 0 /* ADC1, ADC2 and ADC3s         */                   
    .word 0 /* CAN1 TX                      */                         
    .word 0 /* CAN1 RX0                     */                          
    .word 0 /* CAN1 RX1                     */                          
    .word 0 /* CAN1 SCE                     */                          
    .word 0 /* External Line[9:5]s          */                          
    .word 0 /* TIM1 Break and TIM9          */         
    .word 0 /* TIM1 Update and TIM10        */         
    .word 0 /* TIM1 Trigger and Commutation and TIM11 */
    .word 0 /* TIM1 Capture Compare         */                          
    .word 0 /* TIM2                         */                   
    .word 0 /* TIM3                         */                   
    .word 0 /* TIM4                         */                   
    .word 0 /* I2C1 Event                   */                          
    .word 0 /* I2C1 Error                   */                          
    .word 0 /* I2C2 Event                   */                          
    .word 0 /* I2C2 Error                   */                            
    .word 0 /* SPI1                         */                   
    .word 0 /* SPI2                         */                   
    .word 0 /* USART1                       */                   
    .word 0 /* USART2                       */                   
    .word 0 /* USART3                       */                   
    .word 0 /* External Line[15:10]s        */                          
    .word 0 /* RTC Alarm (A and B) through EXTI Line */                 
    .word 0 /* USB OTG FS Wakeup through EXTI line */                       
    .word 0 /* TIM8 Break and TIM12         */         
    .word 0 /* TIM8 Update and TIM13        */         
    .word 0 /* TIM8 Trigger and Commutation and TIM14 */
    .word 0 /* TIM8 Capture Compare         */                          
    .word 0 /* DMA1 Stream7                 */                          
    .word 0 /* FMC                          */                   
    .word 0 /* SDIO                         */                   
    .word 0 /* TIM5                         */                   
    .word 0 /* SPI3                         */                   
    .word 0 /* UART4                        */                   
    .word 0 /* UART5                        */                   
    .word 0 /* TIM6 and DAC1&2 underrun errors */                   
    .word 0 /* TIM7                         */
    .word 0 /* DMA2 Stream 0                */                   
    .word 0 /* DMA2 Stream 1                */                   
    .word 0 /* DMA2 Stream 2                */                   
    .word 0 /* DMA2 Stream 3                */                   
    .word 0 /* DMA2 Stream 4                */                   
    .word 0 /* Reserved                     */                   
    .word 0 /* Reserved                     */                     
    .word 0 /* CAN2 TX                      */                          
    .word 0 /* CAN2 RX0                     */                          
    .word 0 /* CAN2 RX1                     */                          
    .word 0 /* CAN2 SCE                     */                          
    .word 0 /* USB OTG FS                   */                   
    .word 0 /* DMA2 Stream 5                */                   
    .word 0 /* DMA2 Stream 6                */                   
    .word 0 /* DMA2 Stream 7                */                   
    .word 0 /* USART6                       */                    
    .word 0 /* I2C3 event                   */                          
    .word 0 /* I2C3 error                   */                          
    .word 0 /* USB OTG HS End Point 1 Out   */                   
    .word 0 /* USB OTG HS End Point 1 In    */                   
    .word 0 /* USB OTG HS Wakeup through EXTI */                         
    .word 0 /* USB OTG HS                   */                   
    .word 0 /* DCMI                         */                   
    .word 0 /* Reserved                     */                   
    .word 0 /* Reserved                     */
    .word 0 /* FPU                          */
    .word 0 /* Reserved                     */
    .word 0 /* Reserved                     */
    .word 0 /* SPI4                         */
    .word 0 /* Reserved                     */
    .word 0 /* Reserved                     */
    .word 0 /* SAI1                         */
    .word 0 /* Reserved                     */
    .word 0 /* Reserved                     */
    .word 0 /* Reserved                     */
    .word 0 /* SAI2                         */
    .word 0 /* QuadSPI                      */
    .word 0 /* CEC                          */
    .word 0 /* SPDIF RX                     */
    .word 0 /* FMPI2C 1 Event               */
    .word 0 /* FMPI2C 1 Error               */
  .size  vectors, .-vectors

.weak _nmi_handler
.weak _hardFlt_handler
.weak _busFlt_handler
.weak _usageFlt_handler
.weak _svc_handler
.weak _debugMon_handler
.weak _pendSV_handler
.weak _sysTick_handler
