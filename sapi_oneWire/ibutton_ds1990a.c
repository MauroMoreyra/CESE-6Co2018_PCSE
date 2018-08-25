/* Copyright 2018, Mauro Oscar Moreyra <mauromoreyra@gmail.com>.
 * Copyright 2018, Eric Pernia.
 * All rights reserved.
 *
 * This file is part sAPI library for microcontrollers.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* Date: 2018-08-24 */

/*==================[inclusions]=============================================*/

#include "ibutton_ds1990a.h"

/*==================[macros]=================================================*/

DEBUG_PRINT_ENABLE

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main(void) {

	oneWireSensorState_t ibuttonState;
	oneWireSensorData_t ibuttonData;

	// Inicializar y configurar la plataforma
	boardConfig();

	/* Config UART for debug messages */
	uartConfig(UART_USB, 115200);
	//debugPrintConfigUart(UART_USB, 115200);
	printf("Ejemplos de uso de sensor iButton DS1990A\r\n");
	//debugPrintlnString("Ejemplo de uso sensor iButton DS1990A.");

	/* Configuro GPIO1 para uso de 1-Wire, velocidad estandar*/
	ONE_WIRE_config(GPIO1, ONE_WIRE_SPEED_STANDARD);

	/*
	 * Inicializo ibuttonData para comparar
	 * Se debe colocal la data ya conocida del ibutton que se posee
	 * Ej1: 01 ca 93 f1 18 00 00 05 (ds1990a)
	 * Ej2: 33 5d 6e 8b 00 00 00 c9 (ds1961s)
	 */
	ibuttonData.romCode[ONE_WIRE_ROM_DATA_FAMILY_CODE] = 0x01;
	ibuttonData.romCode[ONE_WIRE_ROM_DATA_SN_BYTE1] = 0xca;
	ibuttonData.romCode[ONE_WIRE_ROM_DATA_SN_BYTE2] = 0x93;
	ibuttonData.romCode[ONE_WIRE_ROM_DATA_SN_BYTE3] = 0xf1;
	ibuttonData.romCode[ONE_WIRE_ROM_DATA_SN_BYTE4] = 0x18;
	ibuttonData.romCode[ONE_WIRE_ROM_DATA_SN_BYTE5] = 0x00;
	ibuttonData.romCode[ONE_WIRE_ROM_DATA_SN_BYTE6] = 0x00;

	while(TRUE) {

		delay(1000);

		/* Example displaySensorData (uncomment) */
		ONE_WIRE_displaySensorData();

		/* Example readAndCompareRomCode (uncomment) */
		/*
		if(ONE_WIRE_readAndCompareRomCode(&sensorDataLocal) == TRUE)
			printf("TRUE\r\n");
		else
			printf("FALSE\r\n");
		*/

		/* Example check 1-Wire sensor presence (uncomment) */
		/*
		if(ONE_WIRE_checkSensorPresence() == TRUE)
			printf("ONE_WIRE_SENSOR_OPERATIONAL\r\n");
		else
			printf("ONE_WIRE_SENSOR_NO_PRESENCE\r\n");
		*/
	}

	return 0;
}

/*==================[external functions definition]==========================*/

/*==================[end of file]============================================*/
