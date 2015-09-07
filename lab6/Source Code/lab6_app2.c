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
// Description: Zynq SoC Training-lab6
//////////////////////////////////////////////////////////////////////////////////
#include "xparameters.h"
#include "xgpio.h"
#include "ZedboardOLED.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xtmrctr.h"
//Lab7 addition starts here
#include "xdmaps.h"
//Lab7 addition ends here

// Parameter definitions
#define INTC_DEVICE_ID 			XPAR_PS7_SCUGIC_0_DEVICE_ID
#define BTNS_DEVICE_ID			XPAR_AXI_GPIO_0_DEVICE_ID
#define INTC_GPIO_INTERRUPT_ID  XPAR_FABRIC_AXI_GPIO_0_IP2INTC_IRPT_INTR
#define BTN_INT 				XGPIO_IR_CH1_MASK // This is the interrupt mask for channel one
#define DELAY 100000000

//Lab7 addition starts here

#define DMA_DEVICE_ID 			XPAR_XDMAPS_1_DEVICE_ID
#define DMA_FAULT_INTR			XPAR_XDMAPS_0_FAULT_INTR
#define DMA_DONE_INTR_0			XPAR_XDMAPS_0_DONE_INTR_0

#define SRC_ADDRESS				0x01F00000 // This region is within the DDR3 main memory region. It is populated manually with the frames(13 frames)of Alex.
	/* IMPORTANT NOTE !! MODIFYING THE MEMORY MANUALLY IS CONSIDERED UNSAFE PROCEDURE ESPECIALLY IF THERE IS AN OS RUNNING ON THE SYSTEM,
	 * THE REGION BEING MODIFIED MIGHT BE USED BY THE OS FOR SOMETHING ELSE! THIS PROCEDURE IS USED JUST FOR THE PURPOSE OF DEMONSTRATION
	 *  */
#define DES_ADDRESS				0x43C00000 // OLED memory-mapped region
#define DMA_LENGTH	68					// Size of the DMA block in bytes, which is equal to the frame size
										// Frame size is 16*4(16 data registers, each register is 4 bytes)+1*4 bytes(1 control register) =  68 bytes.
//Lab7 addition ends here

#define TMR_DEVICE_ID			XPAR_TMRCTR_0_DEVICE_ID
#define INTC_TMR_INTERRUPT_ID 	XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR
/* IMPORTANT NOTE: we will set up the frequency of the AXI timer to be 50Mhz
 * the timer is set up to be counting UP , these two facts effect the value selected for
 * the load register to generate 1 Hz interrupts
 */
#define TMR_LOAD				0xFF08FFFF

//Global variables

XGpio   BTNInst;
XScuGic INTCInst;
XTmrCtr TMRInst;
static int btn_value;


//Lab7 addition starts here
unsigned int temp_address = SRC_ADDRESS;
unsigned int 		frame_pointer =0; // frame pointer, to keep track of the frames
unsigned int 		Channel = 0; // we are using channel 0 of the DMA, the DMA has 8 channels
static XDmaPs_Cmd 	DmaCmd;	// DMA command
XDmaPs 				DmaInstance; // DMA instance
static unsigned int x;
static unsigned int y;
//Lab7 addition ends here

//----------------------------------------------------
// PROTOTYPE FUNCTIONS
//----------------------------------------------------
static void BTN_Intr_Handler(void *baseaddr_p);
static int InterruptSystemSetup(XScuGic *XScuGicInstancePtr);
static int IntcInitFunction(u16 DeviceId, XTmrCtr *TmrInstancePtr, XGpio *GpioInstancePtr,XDmaPs *DmaPtr);
static void TMR_Intr_Handler(void *baseaddr_p);

//Lab7 addition starts here
void XDma_Config(u16 DeviceId); // DMA configuration function
void DmaISR(unsigned int Channel, XDmaPs_Cmd *DmaCmd,void *CallbackRef); // DMA done ISR, called upon every successful DMA transaction.
//Lab7 addition ends here

//----------------------------------------------------
// INTERRUPT HANDLER FUNCTIONS
// - called by the timer to manage frame_pointer
// - called by the DMA controller
// - Push buttons interrupts are included but not currently used- they are used in the second application
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


		btn_value = XGpio_DiscreteRead(&BTNInst, 1);
		switch (btn_value){

					//Checking if BTNC was pressed. Action: Show Alex in the middle
					case 1:
						x=8;
						y=2;
						clear_OLED();
						show_alex(x,y);
					break;

					//Checking if BTND was pressed. Action: Check for boundary and move Alex down.
					case 2:
						if (y<3)
							{
								y=y+1;
								clear_OLED();
								show_alex(x,y);
							}
					break;

					//Checking if BTNL was pressed. Action: Check for boundary and move Alex left.
					case 4:
						if (x>0)
							{
								x=x-1;
								clear_OLED();
								show_alex(x,y);
							}
					break;

					//Checking if BTNR was pressed. Action: Check for boundary and move Alex right.
					case 8:
						if (x<15)
							{
								x=x+1;
								clear_OLED();
								show_alex(x,y);
							}
					break;

					//Checking if BTNU was pressed. Action: Check for boundary and move Alex up.
					case 16:
						if (y>0)
							{
								y=y-1;
								clear_OLED();
								show_alex(x,y);
							}
					break;

					default:
					break;
						}


		// Acknowledge GPIO interrupts
	    (void)XGpio_InterruptClear(&BTNInst, BTN_INT);
	    // Enable GPIO interrupts
	    XGpio_InterruptEnable(&BTNInst, BTN_INT);

}


void TMR_Intr_Handler(void *data)
{

//// Disable timer interrupt
//
//	XTmrCtr_Stop(&TMRInst,0);
//
//	// Increment the frame pointer
//	frame_pointer++;
//
//	// Point to the next frame in memory.
//	if (frame_pointer<12){
//	//Each frame is 16*4(16 data registers, each register is 4 bytes) +1*4 bytes(1 control register)= 68bytes.
//	temp_address=temp_address+68;
//
//	DmaCmd.BD.SrcAddr = (u32) temp_address; // Set the new address for the DMA command
//
//	XDmaPs_Start(&DmaInstance, Channel, &DmaCmd, 0); // Start a DMA transaction
//	}
//
//// Enable and acknowledge timer interrupt
//
//	XTmrCtr_Reset(&TMRInst,0);
//	XTmrCtr_Start(&TMRInst,0);


}



//----------------------------------------------------
// MAIN FUNCTION
//----------------------------------------------------
int main (void)
{

  printf("Animation Test\n\r");
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

  //----------------------------------------------------
  // SETUP THE TIMER
  //----------------------------------------------------
  status = XTmrCtr_Initialize(&TMRInst, TMR_DEVICE_ID);
  if(status != XST_SUCCESS) return XST_FAILURE;
  XTmrCtr_SetHandler(&TMRInst, TMR_Intr_Handler, &TMRInst);
  XTmrCtr_SetOptions(&TMRInst, 0, XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);
  XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD);
  XTmrCtr_Start(&TMRInst,0);


  // Initialize interrupt controller
  status = IntcInitFunction(INTC_DEVICE_ID, &TMRInst, &BTNInst,&DmaInstance);
  if(status != XST_SUCCESS) return XST_FAILURE;

  //Lab7 addition starts  here
  //Initialize DMA controller
 // XDma_Config(DMA_DEVICE_ID);
  //Lab7 addition ends  here
  while(1){
// Infinite loop - Do nothing
       }

  // Never reached on normal execution
  return (0);
  }

//----------------------------------------------------
// INITIAL SETUP FUNCTIONS
//----------------------------------------------------
// modified to initialize DMA,timer,GIC and GPIO.

int IntcInitFunction(u16 DeviceId, XTmrCtr *TmrInstancePtr, XGpio *GpioInstancePtr, XDmaPs *DmaPtr)
{
	XScuGic_Config *IntcConfig;
	int status;

	// Interrupt controller initialization
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


	// Connect timer interrupt to handler
	status = XScuGic_Connect(&INTCInst,
							 INTC_TMR_INTERRUPT_ID,
							 (Xil_ExceptionHandler)TMR_Intr_Handler,
							 (void *)TmrInstancePtr);
	if(status != XST_SUCCESS) return XST_FAILURE;


	// Enable GPIO interrupts interrupt
	XGpio_InterruptEnable(GpioInstancePtr, 1);
	XGpio_InterruptGlobalEnable(GpioInstancePtr);

	// Enable GPIO interrupts in the controller
	XScuGic_Enable(&INTCInst, INTC_GPIO_INTERRUPT_ID);


	// Enable  timer interrupts in the controller
	XScuGic_Enable(&INTCInst, INTC_TMR_INTERRUPT_ID);

	//Lab7 addition starts here
	// Connect DMA interrupt to handler
	XScuGic_Connect(&INTCInst,DMA_FAULT_INTR,(Xil_InterruptHandler)XDmaPs_FaultISR,(void *)DmaPtr);


	XScuGic_Connect(&INTCInst,DMA_DONE_INTR_0,(Xil_InterruptHandler)XDmaPs_DoneISR_0,(void *)DmaPtr);
	// Enable DMA interrupts in the controller
	XScuGic_Enable(&INTCInst, DMA_DONE_INTR_0);

	//Lab7 addition ends here

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

//Lab7 addition starts here
void XDma_Config(u16 DeviceId)
{



		volatile int Checked[XDMAPS_CHANNELS_PER_DEV];
		XDmaPs_Config *DmaCfg;

		//Explicitly reset all fields of the DMA command
		memset(&DmaCmd, 0, sizeof(XDmaPs_Cmd));

		// Populate the fields of the DMA command

		/* the DMA command structure has 9 fields, two fields of our concern are the Channel Control(ChanCtrl) field , and the DMA block descriptor(BD) field .
		 * In the DMA block descriptor field structure you specify three parameters (1)Source starting address,(2)Destination starting address,(3)Number of bytes for the block
		 * In the Channel Control field structure you specify parameters related to the AXI bus transaction. which will be translated into a 32-bit channel control register value
		 * */

		//Channel Control field structure
		DmaCmd.ChanCtrl.SrcBurstSize = 4; // Source burst size (word is 4 bytes)
		DmaCmd.ChanCtrl.SrcBurstLen = 4; // Source burst length (word is 4 bytes)
		DmaCmd.ChanCtrl.SrcInc = 1; // Source incrementing or fixed (incrementing)
		DmaCmd.ChanCtrl.DstBurstSize = 4; // Destination burst size (word is 4 bytes)
		DmaCmd.ChanCtrl.DstBurstLen = 4; // Destination burst length(word is 4 bytes)
		DmaCmd.ChanCtrl.DstInc = 1; // Destination incrementing or fixed (incrementing)

		//DMA block descriptor(BD) field structure
		DmaCmd.BD.SrcAddr = (u32) SRC_ADDRESS; // Source starting address
		DmaCmd.BD.DstAddr = (u32) DES_ADDRESS; // Destination starting address
		DmaCmd.BD.Length = DMA_LENGTH ;			//Number of bytes for the block(made equal to the frame size)

		DmaCfg = XDmaPs_LookupConfig(DeviceId);
		XDmaPs_CfgInitialize(&DmaInstance,DmaCfg,DmaCfg->BaseAddress);


		Checked[Channel] = 0;

		/* Set the Done interrupt handler */
		XDmaPs_SetDoneHandler(&DmaInstance,Channel,DmaISR,(void *)(Checked + Channel));
		XDmaPs_Start(&DmaInstance, Channel, &DmaCmd, 0); // Start first frame copying.


}
/* Done Handler */
// Called upon every successful DMA transaction
void DmaISR(unsigned int Channel, XDmaPs_Cmd *DmaCmd, void *CallbackRef)
{


		volatile int *Checked = (volatile int *)CallbackRef;

		*Checked = 1;
		printf("DMA stage %d passed\n\r",frame_pointer);
}
//Lab7 addition ends here
void show_alex(int x, int y)

	{
		print_char(24,y,x);
	}
