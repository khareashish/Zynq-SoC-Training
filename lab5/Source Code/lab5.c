/*Copyright (c) 2015, EmbeddedCentric.com
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

//////////////////////////////////////////////////////////////////////////////////
// Company: EmbeddedCentric.com
// Author:  Ali Aljaani
// Create Date:    08/15/2015
// Description: Zynq SoC Training-lab5
//////////////////////////////////////////////////////////////////////////////////


#include "xparameters.h"
#include "xgpio.h"
#include "ZedboardOLED.h"
#include "xscugic.h"
#include "xil_exception.h"
//Lab5 addition start
#include "xtmrctr.h"
//Lab5 addition end

// Parameter Definitions
#define INTC_DEVICE_ID 			XPAR_PS7_SCUGIC_0_DEVICE_ID
#define BTNS_DEVICE_ID			XPAR_AXI_GPIO_0_DEVICE_ID
#define INTC_GPIO_INTERRUPT_ID  XPAR_FABRIC_AXI_GPIO_0_IP2INTC_IRPT_INTR
#define BTN_INT 				XGPIO_IR_CH1_MASK // This is the interrupt mask for channel one


//Lab5 addition start
#define TMR_DEVICE_ID			XPAR_TMRCTR_0_DEVICE_ID
#define INTC_TMR_INTERRUPT_ID 	XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR
/* IMPORTANT NOTE: The AXI timer frequency is set to 50Mhz
 * the timer is set up to be counting UP, these two facts affect the value selected for
 * the load register to generate 1 Hz interrupts
 */
#define TMR_LOAD				0xFD050F7F
//Lab5 addition end

XGpio   BTNInst;
XScuGic INTCInst;
static int btn_value;

//Lab5 addition start
XTmrCtr TMRInst;
//Global variables
unsigned int seconds = 0;
unsigned int minutes = 0;
unsigned int hours = 0;
char buffer [9];
unsigned int options;
//Lab5 addition end

//----------------------------------------------------
// PROTOTYPE FUNCTIONS
//----------------------------------------------------
static void BTN_Intr_Handler(void *baseaddr_p);
static int InterruptSystemSetup(XScuGic *XScuGicInstancePtr);
static int IntcInitFunction(u16 DeviceId, XTmrCtr *TmrInstancePtr, XGpio *GpioInstancePtr);
//Lab5 addition start
static void TMR_Intr_Handler(void *baseaddr_p);
//Lab5 addition end

//----------------------------------------------------
// INTERRUPT HANDLER FUNCTIONS
// - called by the buttons interrupt, performs push buttons read
// - OLED displaying
// - called by the timer to manage the real time
//----------------------------------------------------


void BTN_Intr_Handler(void *InstancePtr)
{

	// Ignore additional button presses
	if ((XGpio_InterruptGetStatus(&BTNInst) & BTN_INT) !=
			BTN_INT) {
			return;

	// Disable GPIO interrupts
	XGpio_InterruptDisable(&BTNInst, BTN_INT);
			 }
	clear_OLED(); // clear the OLED
	btn_value = XGpio_DiscreteRead(&BTNInst, 1);
	switch (btn_value){

				//Checking if BTNC was pressed. Action: reset the  Real time clock
				case 1:
					seconds = 0;
					minutes = 0;
				    hours   = 0;
				    sprintf (buffer, "%d:%d:%d",hours,minutes,seconds);
				    print_message(buffer,0);
				break;

				//Checking if BTND was pressed
				case 2:
					// Nothing to do
				break;

				//Checking if BTNL was pressed. Action: Increment the hours
				case 4:
					if
					(hours <23){
					hours++;
					}
				    sprintf (buffer, "%d:%d:%d",hours,minutes,seconds);
					print_message(buffer,0);
				break;

				//Checking if BTNR was pressed. Action: Increment the minutes
				case 8:
					if (minutes<59){
					minutes++;
					}
			        sprintf (buffer, "%d:%d:%d",hours,minutes,seconds);
				    print_message(buffer,0);
				break;

				//Checking if BTNU was pressed
				case 16:
					// Nothing to do
				break;

				default:
				break;
					}


	// Acknowledge GPIO interrupts
    (void)XGpio_InterruptClear(&BTNInst, BTN_INT);
    // Enable GPIO interrupts
    XGpio_InterruptEnable(&BTNInst, BTN_INT);

}

//Lab5 addition start
void TMR_Intr_Handler(void *data)
{
// Disable interrupt

	XTmrCtr_Stop(&TMRInst,0);

// Increment the global seconds variable
			seconds++;

// Increment the global minutes variable
	if (seconds>=60)
		{
		minutes++;
		seconds = 0;
		}
// Increment the global hours variable
	if (minutes>=60)

		{
		hours++;
		minutes = 0;
		}
// Restricting boundaries
    if (hours >= 24) {
      hours = 0;
    }

//Clear the OLED
    clear_OLED();
/*sprintf : Composes a string with the same text that would be printed if format was used on printf
 it is used here to organise the time in HH:MM:SS format*/
    sprintf (buffer, "%d:%d:%d",hours,minutes,seconds);

// print the time on the first page of the OLED
    print_message(buffer,0);

// Enable and acknowledge  interrupt

	XTmrCtr_Reset(&TMRInst,0);
	XTmrCtr_Start(&TMRInst,0);


}

//Lab5 addition end


//----------------------------------------------------
// MAIN FUNCTION
//----------------------------------------------------
int main (void)
{
  int status;
  char c;
  //----------------------------------------------------
  // INITIALIZE THE PERIPHERALS & SET DIRECTIONS OF GPIO
  //----------------------------------------------------
  // Initialize Push Buttons
  status = XGpio_Initialize(&BTNInst, BTNS_DEVICE_ID);
  if(status != XST_SUCCESS) return XST_FAILURE;

  // Set all buttons direction to inputs
  XGpio_SetDataDirection(&BTNInst, 1, 0xFF);

  //Lab5 addition start
  //----------------------------------------------------
  // SETUP THE TIMER
  //----------------------------------------------------
  status = XTmrCtr_Initialize(&TMRInst, TMR_DEVICE_ID);
  if(status != XST_SUCCESS) return XST_FAILURE;
  XTmrCtr_SetHandler(&TMRInst, TMR_Intr_Handler, &TMRInst);
  XTmrCtr_SetOptions(&TMRInst, 0, XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);
  XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD);
  XTmrCtr_Start(&TMRInst,0);
  //Lab5 addition end

  // Initialize interrupt controller
  status = IntcInitFunction(INTC_DEVICE_ID, &TMRInst, &BTNInst);
  if(status != XST_SUCCESS) return XST_FAILURE;



  while(1){
// Infinite loop - Do nothing
       }

  // Never reached on normal execution
  return (0);
  }


//----------------------------------------------------
// INITIAL SETUP FUNCTIONS
//----------------------------------------------------
// modified to initialize the timer as well as the GIC and the GPIO.

int IntcInitFunction(u16 DeviceId, XTmrCtr *TmrInstancePtr, XGpio *GpioInstancePtr)
{
	XScuGic_Config *IntcConfig;
	int status;

	// Interrupt controller initialisation
	IntcConfig = XScuGic_LookupConfig(DeviceId);
	status = XScuGic_CfgInitialize(&INTCInst, IntcConfig, IntcConfig->CpuBaseAddress);
	if(status != XST_SUCCESS) return XST_FAILURE;

	// Call to interrupt setup
	status = InterruptSystemSetup(&INTCInst);
	if(status != XST_SUCCESS) return XST_FAILURE;

	// Connect GPIO interrupt to handler
	status = XScuGic_Connect(&INTCInst,
					  	  	 INTC_GPIO_INTERRUPT_ID,
					  	  	 (Xil_ExceptionHandler)BTN_Intr_Handler,
					  	  	 (void *)GpioInstancePtr);
	if(status != XST_SUCCESS) return XST_FAILURE;

	//Lab5 addition start
	// Connect timer interrupt to handler
	status = XScuGic_Connect(&INTCInst,
							 INTC_TMR_INTERRUPT_ID,
							 (Xil_ExceptionHandler)TMR_Intr_Handler,
							 (void *)TmrInstancePtr);
	if(status != XST_SUCCESS) return XST_FAILURE;
	//Lab5 addition end

	// Enable GPIO interrupts interrupt
	XGpio_InterruptEnable(GpioInstancePtr, 1);
	XGpio_InterruptGlobalEnable(GpioInstancePtr);

	// Enable GPIO interrupts in the controller
	XScuGic_Enable(&INTCInst, INTC_GPIO_INTERRUPT_ID);

	//Lab5 addition start
	// Enable  timer interrupts in the controller
	XScuGic_Enable(&INTCInst, INTC_TMR_INTERRUPT_ID);
	//Lab5 addition end

	return XST_SUCCESS;
}

int InterruptSystemSetup(XScuGic *XScuGicInstancePtr)
{
	// Register GIC interrupt handler

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			 	 	 	 	 	 (Xil_ExceptionHandler)XScuGic_InterruptHandler,
			 	 	 	 	 	 XScuGicInstancePtr);
	Xil_ExceptionEnable();


	return XST_SUCCESS;

}
