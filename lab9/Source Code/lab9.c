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
// Create Date:    08/26/2015
// Description: Zynq SoC Training-lab9
// Stepper Motor Controller
// Motor Type : SM-42BYG011-25
// Resolution : 1.8 Degree per step
// Phases : 2
// Driver Board : PmodSTEP by Digilent
// Connection: Pmod JA1 connector
// GPIO0(0):JA7: A
// GPIO0(1):JA8: B
// GPIO0(2):JA9: A'
// GPIO0(3):JA10:B'
//////////////////////////////////////////////////////////////////////////////////

#include <xparameters.h>
#include "xstatus.h"
#include "xgpio.h"

// Parameter definitions
#define STEPPER_ID 		XPAR_GPIO_0_DEVICE_ID
#define PUSHBUTTONS_ID  XPAR_GPIO_1_DEVICE_ID
#define STEPPER_CHANNEL 	1
#define PUSHBUTTONS_CHANNEL 1
#define SPEED 1000000 // Controls rotation speed


//Global variables
int push_check;
	XGpio Gpio;
	XGpio Gpio1;

//----------------------------------------------------
// PROTOTYPE FUNCTIONS
//----------------------------------------------------
	int step_cw(void);
	int step_ccw(void);
	void delay(void);
	int rotate360_cw(void);
	int rotate360_ccw(void);
	int Initialize_GPIO(void);

main (){

	// Initialize pushbuttons and Stepper GPIO
	Initialize_GPIO();

	// Infinite loop
	while (1){
		//Reading from the push buttons
		push_check= XGpio_DiscreteRead(&Gpio1,PUSHBUTTONS_CHANNEL);

		switch (push_check){

		//Checking if BTNC was pressed
		case 1:
			// NOT USED
			break;
		//Checking if BTND was pressed - Rotate counter clock wise 360 degrees
		case 2:
			rotate360_ccw();
			break;

		//Checking if BTNL was pressed - Rotate counter clock wise 7.2 degrees
		case 4:
			step_ccw();
			break;

		//Checking if BTNR was pressed - Rotate clock wise 7.2 degrees
		case 8:
			step_cw();
			break;

		//Checking if BTNU was pressed - Rotate clock wise 360 degrees
		case 16:
			rotate360_cw();
			break;

		default:
		break;
		}

}
}

//----------------------------------------------------
// Rotate 7.2 degrees clock wise
// 4X1.8= 7.2 degrees
//----------------------------------------------------
int step_cw(){

			XGpio_DiscreteWrite(&Gpio,STEPPER_CHANNEL , 12);  // ++--
			delay();
			XGpio_DiscreteWrite(&Gpio,STEPPER_CHANNEL , 6);  // -++-
			delay();
			XGpio_DiscreteWrite(&Gpio,STEPPER_CHANNEL , 3); // --++
			delay();
			XGpio_DiscreteWrite(&Gpio,STEPPER_CHANNEL , 9);  // +--+
			delay();
			XGpio_DiscreteWrite(&Gpio,STEPPER_CHANNEL , 12);  // ++--

			return (0);
}
//----------------------------------------------------
// Rotate 7.2 degrees counter clock wise
// 4X1.8= 7.2 degrees
//----------------------------------------------------
int step_ccw(){

		XGpio_DiscreteWrite(&Gpio,STEPPER_CHANNEL , 12);  // ++--
		delay();
		XGpio_DiscreteWrite(&Gpio,STEPPER_CHANNEL , 9);  // +--+
		delay();
		XGpio_DiscreteWrite(&Gpio,STEPPER_CHANNEL , 3); // --++
		delay();
		XGpio_DiscreteWrite(&Gpio,STEPPER_CHANNEL , 6);  // -++-
		delay();
		XGpio_DiscreteWrite(&Gpio,STEPPER_CHANNEL , 12);  // ++--

		return (0);
}

//----------------------------------------------------
// Software delay
// controls the speed of the motor
//----------------------------------------------------
delay(){
	int i ;
	int x;
	for (i=0;i<SPEED;i++){
		x=x+0; // Idle Loop

	}
}

//----------------------------------------------------
// Rotate 360 degrees clock wise
// 50X7.2= 360 degrees
//----------------------------------------------------
int rotate360_cw(){
	int i ;
	for (i=0;i<50;i++)
				{
					step_cw();
				}

	return 0;

	}

//----------------------------------------------------
// Rotate 360 degrees counter clock wise
// 50X7.2= 360 degrees
//----------------------------------------------------
int rotate360_ccw(){
	int i ;
	for (i=0;i<50;i++)
				{
					step_ccw();
				}

	return 0;

	}


//----------------------------------------------------
// INITIALIZE & SET DIRECTIONS OF GPIOs
//----------------------------------------------------
int Initialize_GPIO(){

	int Status;
	// Initialize the Stepper GPIO
	Status = XGpio_Initialize(&Gpio, STEPPER_ID);
				if(Status != XST_SUCCESS) return XST_FAILURE;
	// Initialize Push Buttons
	Status = XGpio_Initialize(&Gpio1, PUSHBUTTONS_ID);
					if(Status != XST_SUCCESS) return XST_FAILURE;
	// Set all four Stepper GPIO to output
	XGpio_SetDataDirection(&Gpio, STEPPER_CHANNEL,0x00);
	// Set all five pushbuttons direction to inputs
	XGpio_SetDataDirection(&Gpio1,PUSHBUTTONS_CHANNEL,0x1f);

return Status;
}