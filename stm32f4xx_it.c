/**
  ******************************************************************************
  * @file    GPIO/GPIO_IOToggle/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    18-January-2013
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"


CPU_STATE saved_cpu_state;
 
int exception_code = EXCEPTION_NONE;

volatile unsigned int uwTimingDelay;

/** @addtogroup Template_Project
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

int __attribute__ (( naked )) SaveCPUState(CPU_STATE* StateWord)
{
/*
setjmp():
 8038350:       46ec            mov     ip, sp
 8038352:       e8a0 5ff0       stmia.w r0!, {r4, r5, r6, r7, r8, r9, sl, fp, ip, lr}
 8038356:       f04f 0000       mov.w   r0, #0
 803835a:       4770            bx      lr
*/

	// This works as setjmp() plus saves xPSR register 

	asm volatile ("mov 	ip, sp");
	asm volatile ("stmia 	r0!, {r4, r5, r6, r7, r8, r9, sl, fp, ip, lr}");
	asm volatile ("mrs	r1, xpsr");
	asm volatile ("orr	r1, 0x01000000");
	asm volatile ("str	r1, [r0]");
	asm volatile ("mov 	r0, #0");
	asm volatile ("bx 	lr");
}


void General_Exception_Handler_main(unsigned int * args);

void __attribute__ (( naked )) General_Exception_Handler(void)
{


 	//asm volatile ("ldr r0, [sp, #28]\n"); // r0 - new xPSR from Handler stack

 	asm volatile ("mov r2, %0\n" : : "r" (&saved_cpu_state) ); // r2 - now points to setjmp stack 
 	asm volatile ("ldr r1, [r2, #32]\n"); // r1 <- saved_cpu_state[8] - saved stack 

	// There are two returning modes: with and without FP registers restoration
	asm volatile ("tst lr, 0x00000010\n"); // Check for FPU mode 
	asm volatile ("itt eq\n");
 	asm volatile ("subseq r1, #104\n"); // r1 = r1 - 8*4 + 18*4
 	asm volatile ("subseq r1, #32\n"); // r1 = r1 - 8*4

 	asm volatile ("ldr r0, [r2, #40]\n"); // r0 <- saved_cpu_state[10] - saved xPSR 
 	asm volatile ("str r0, [r1, #28]\n"); // new xPSR -> returning stack xPSR


	// Prepare to call HardFault_Handler_main()
	asm volatile ("tst lr, #4\n"); /* Check EXC_RETURN[2] */
	asm volatile ("ite eq\n");
	asm volatile ("mrseq r0, msp\n"); // R0 points to MSP
	asm volatile ("mrsne r0, psp\n"); // or to PSP
	
	asm volatile ("push {r1, r2, lr}\n"); // Prepare to call to HardFault_Handler_main()
	asm volatile(  
		"bl %[General_Exception_Handler_main]\t\n"
		: // no output 
		: [General_Exception_Handler_main] "i" (General_Exception_Handler_main) // input
		: "r0" // clobber 
	);
	asm volatile ("pop {r1, r2, lr}\n"); // Restore after call to HardFault_Handler_main()


	// Coin return stack

 	asm volatile ("mov sp, r1\n"); // sp <- returning stack - 32
	asm volatile ("tst lr, #4\n"); /* Check EXC_RETURN[2] */
	asm volatile ("ite eq\n");
	asm volatile ("msreq msp, r1\n"); // store R1 to MSP
	asm volatile ("msrne psp, r1\n"); // or to PSP

 	asm volatile ("ldr r1, [r2, #36]\n"); // r1 <- saved_cpu_state[9] - now new PC 
 	asm volatile ("str r1, [sp, #24]\n"); // new PC -> returning stack PC
 	asm volatile ("str r1, [sp, #20]\n"); // new PC -> returning stack LR

 	asm volatile ("mov r0, %0\n" : : "r" (exception_code) ); // r0 - returns exception code 
 	asm volatile ("str r0, [sp, #0]\n"); // R0 -> returning exception code 

 	asm volatile ("ldr r7, [r2, #12]\n"); // r7 <- saved_cpu_state[3]
 	asm volatile ("ldr r8, [r2, #16]\n"); // r7 <- saved_cpu_state[4]
 	asm volatile ("ldr r9, [r2, #20]\n"); // r7 <- saved_cpu_state[5]


	asm volatile ("bx lr\n"); // RETURN 
}

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void __attribute__ (( naked )) NMI_Handler(void)
{
	exception_code = EXCEPTION_NMI;

	asm volatile( "b General_Exception_Handler" );
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */


void __attribute__ (( naked )) HardFault_Handler(void)
{
	exception_code = EXCEPTION_HARDFAULT;

	asm volatile( "b General_Exception_Handler" );
}


/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void __attribute__ (( naked )) MemManage_Handler(void)
{
	exception_code = EXCEPTION_MEMMANAGE;

	asm volatile( "b General_Exception_Handler" );
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void __attribute__ (( naked )) BusFault_Handler(void)
{
	exception_code = EXCEPTION_BUSFAULT;

	asm volatile( "b General_Exception_Handler" );
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void __attribute__ (( naked )) UsageFault_Handler(void)
{
	exception_code = EXCEPTION_USAGEFAULT;

	asm volatile( "b General_Exception_Handler" );
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */

void SVC_Handler_main(unsigned int * svc_args);

void __attribute__ (( naked ))  SVC_Handler(void)
{
	asm volatile(
		"tst lr, #4\t\n" /* Check EXC_RETURN[2] */
		"ite eq\t\n"
		"mrseq r0, msp\t\n"
		"mrsne r0, psp\t\n"
		"b %[SVC_Handler_main]\t\n"
		: /* no output */
		: [SVC_Handler_main] "i" (SVC_Handler_main) /* input */
		: "r0" /* clobber */
	);
}



/*
void 
SVC_Handler
(unsigned 
int
* svc_args) { 
int
a,b,c
;
a = svc_args[0];
//get first argument from stack
b = svc_args[1];
//get second argument from stack
c = a + b;
svc_args[0] = c;
//replace R0 value in stack with result to “return” result in R0
}
} 
*/


/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
	//exception_code = EXCEPTION_DEBUGMON;
	//__enable_irq();
	//longjmp(exception_sj_buf,exception_code); /* return to saved state */
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
	exception_code = EXCEPTION_PENDSV;
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
}


/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f40xx.s/startup_stm32f427x.s).                         */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
