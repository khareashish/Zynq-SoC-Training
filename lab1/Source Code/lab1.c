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
// Create Date:    08/08/2015
// Description: Zynq SoC Training-lab1
//////////////////////////////////////////////////////////////////////////////////



#include "xparameters.h"
#include "xgpio.h"

int main(void)
{
//Variables Declarations
	XGpio push, led; // instance pointer for the push buttons and LEDs
	int i,push_check;
	int out = 0;
	int val=1;
	xil_printf("Lab1 application started.... \r \n");
	int delay_value=9999999;

//AXI GPIO Initialization
	XGpio_Initialize(&push,XPAR_BTNS_5BITS_DEVICE_ID);
	XGpio_Initialize(&led,XPAR_LED_8BITS_DEVICE_ID);
//AXI GPIO Set Direction
	XGpio_SetDataDirection(&push,1,0xffffffff);
	XGpio_SetDataDirection(&led,1,0x00000000);

	while (1){
//Reading from the push buttons
		push_check= XGpio_DiscreteRead(&push,1);

		switch (push_check){

		//Checking if BTNC was pressed
		case 1:
			delay_value=9999999;
			break;
		//Checking if BTND was pressed
		case 2:
			val = -1;
			break;

		//Checking if BTNL was pressed
		case 4:
			delay_value=19999999;
			break;

		//Checking if BTNR was pressed
		case 8:
			delay_value=4099999;
			break;

		//Checking if BTNU was pressed
		case 16:
			val =1;
			break;

		default:
		break;
		}

//Printing the value returned by XGpio_DiscreteRead
		xil_printf("Push Buttons Status: %d \r \n",push_check);

//Printing the value of variable val and out
		xil_printf("Value of val: %d \r \n",val);
		xil_printf("Value of out: %d \r \n",out);
// Incrementing the value of out by val
		out = out+val;
// Checking boundaries
		if (out > 255){
					out=0;
					  }
		if (out< 0) {
					out=255;
					}
//Writing the value of variable out to the LEDs
		XGpio_DiscreteWrite(&led,1,out);

// Creating a software delay
		for (i=0;i<delay_value;i++);

			}

	return (0);
}

