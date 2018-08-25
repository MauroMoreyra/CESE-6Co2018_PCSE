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

#include "sapi_oneWire.h"			/* <= sAPI 1-Wire header */

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/**
 * @brief			Check CRC from obtained ROM data.
 * @return			Return 1 if CRC is valid, return 0 otherwise.
 */
static bool_t ONE_WIRE_ckeckCRC(void);

/**
 * @brief			Pause (blocking) for exactly 'delay_us' microseconds.
 * @param			delay_us: amount of microseconds to delay.
 */
static void ONE_WIRE_delayUs(uint32_t delay_us);

/**
 * @brief			Pause (blocking) for exactly 'delay_250Ns' times 250 ns.
 * @param			delay_250Ns: amount of 250 ns to delay.
 */
static void ONE_WIRE_delay250Ns(uint32_t delay_250Ns);

/**
 * @brief			Adjust tick values depending on speed.
 * @param			speed: define timing to 'standard' (=1) or 'overdrive' (=0).
 */
static void ONE_WIRE_setSpeed(uint8_t speed);

/**
 * @brief			Generate a 1-Wire reset.
 * @return			Return 1 if presence detect was found, return 0 otherwise.
 */
static bool_t ONE_WIRE_touchReset(void);

/**
 * @brief			Send a 1-Wire write bit. Provide 10us recovery time.
 * @param			bit: 1 or 0.
 */
static void ONE_WIRE_writeBit(uint8_t bit);

/**
 * @brief			Read a bit from the 1-Wire bus and return it.
 * 					Provide 10us recovery time.
 * @return			Bit read.
 */
static uint8_t ONE_WIRE_readBit(void);

/**
 * @brief			Write 1-Wire data byte.
 * @param			data: byte to transmit.
 */
static void ONE_WIRE_writeByte(uint8_t data);

/**
 * @brief			Read 1-Wire data byte and return it.
 * @relval			Byte read.
 */
static uint8_t ONE_WIRE_readByte(void);

/*==================[internal data definition]===============================*/

static uint32_t A, B, C, D, E, F, G, H, I, J;	// 'tick' values
static oneWireData_t oneWireData;					// 1-Wire data structure

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static bool_t ONE_WIRE_checkCRC(void) {
	/*
	 * The 1-Wire CRC is generated using a polynomial generator consisting of a
	 * shift register and XOR gates. The polynomial is X^8 + X^5 + X^4 + 1.
	 */

	uint8_t crc = 0;
	uint8_t byteIndex;
	uint8_t i;

	for(byteIndex = 0; byteIndex < (ROM_CODE_LENGHT - 1); byteIndex++) {

		crc ^= oneWireData.sensorData.romCode[byteIndex];

		for(i = 0; i < 8; i++) {
			if (crc & 0x01)
				crc = (crc >> 1) ^ CRC_POLYNOMIAL_MASK;
			else
				crc >>= 1;
		}
	}

	if(crc == oneWireData.sensorData.romCode[ONE_WIRE_ROM_DATA_CRC])
		return TRUE;
	else
		return FALSE;
}

static bool_t ONE_WIRE_touchReset(void) {
	/*
	 * Generate a 1-Wire reset, return 1 if presence detect was found, return 0
	 * otherwise.
	 * NOTE: Does not handle alarm presence from DS2404/DS1994.
	 */

	uint8_t result;

	ONE_WIRE_delay250Ns(G);
	gpioInit(oneWireData.gpio, GPIO_OUTPUT);
	gpioWrite(oneWireData.gpio, OFF);
	ONE_WIRE_delay250Ns(H);
	gpioWrite(oneWireData.gpio, ON);
	ONE_WIRE_delay250Ns(I);
	// Sample for presence pulse from slave
	gpioInit(oneWireData.gpio, GPIO_INPUT_PULLUP);
	result = !gpioRead(oneWireData.gpio);
	// Complete the reset sequence recovery
	ONE_WIRE_delay250Ns(J);

	return result;							// Return sample presence pulse result
}

static void ONE_WIRE_writeByte(uint8_t data)
{
	/*
	 * Write 1-Wire data byte.
	 */

	uint8_t loop;

	/* Loop to write each bit in the byte, LS-bit first */
	for (loop = 0; loop < 8; loop++)
	{
		ONE_WIRE_writeBit(data & 0x01);
		/* Shift the data byte for the next bit */
		data >>= 1;
	}
}

static uint8_t ONE_WIRE_readByte(void)
{
	/*
	 * Read 1-Wire data byte and return it.
	 */

	uint8_t loop, result=0;

	for(loop = 0; loop < 8; loop++) {
		/* Shift the result to get it ready for the next bit */
		result >>= 1;
		/* If result is one, then set MS bit */
		if(ONE_WIRE_readBit())
			result |= 0x80;
	}

	return result;
}

static void ONE_WIRE_setSpeed(uint8_t speed) {
	/*
	 * Adjust tick values depending on speed [microseconds]
	 * Define 1-Wire timing to 'standard' (=1) or 'overdrive' (=0).
	 */

	if(speed) {
		/* Standard Speed */
		A = 6 * 4;		// 6
		B = 64 * 4;		// 64
		C = 60 * 4;		// 60
		D = 10 * 4;		// 10
		E = 9 * 4;		// 9
		F = 55 * 4;		// 55
		G = 0;			// 0
		H = 480 * 4;	// 480
		I = 70 * 4;		// 70
		J = 410 * 4;	// 410
	} else {
		/* Overdrive Speed */
		A = 1.5 * 4;	// 1.5
		B = 7.5 * 4;	// 7.5
		C = 7.5 * 4;	// 7.5
		D = 2.5 * 4;	// 2.5
		E = 0.75 * 4;	// 0.75
		F = 7 * 4;		// 7
		G = 2.5 * 4;	// 2.5
		H = 70 * 4;		// 70
		I = 8.5 * 4;	// 8.5
		J = 40 * 4;		// 40
	}
}

static void ONE_WIRE_writeBit(uint8_t bit) {
	/*
	 * Send a 1-Wire write bit. Provide 10us recovery time.
	 */

	gpioInit(oneWireData.gpio, GPIO_OUTPUT);

	/* Set gpio to low state */
	gpioWrite(oneWireData.gpio, OFF);

	if(bit) {
		/* Write '1' bit */
		ONE_WIRE_delay250Ns(A);
		gpioWrite(oneWireData.gpio, ON);
		ONE_WIRE_delay250Ns(B);
	} else {
		/* Write '0' bit */
		ONE_WIRE_delay250Ns(C);
		gpioWrite(oneWireData.gpio, ON);
		ONE_WIRE_delay250Ns(D);
	}
}

static uint8_t ONE_WIRE_readBit(void)
{
	/*
	 * Read a bit from the 1-Wire bus and return it.
	 */

	uint8_t result;

	gpioInit(oneWireData.gpio, GPIO_OUTPUT);
	gpioWrite(oneWireData.gpio, OFF);
	ONE_WIRE_delay250Ns(A);
	gpioWrite(oneWireData.gpio, ON);
	ONE_WIRE_delay250Ns(E);
	gpioInit(oneWireData.gpio, GPIO_INPUT_PULLUP);
	result = gpioRead(oneWireData.gpio);	// Sample the bit value from the slave
	ONE_WIRE_delay250Ns(F);				// Complete the time slot and 10us recovery

	return result;
}



static void ONE_WIRE_delayUs(uint32_t delay_us) {
	/*
	 * 1 ciclo 		= 4.902 ns
	 * 204 ciclos 	= 1 us		@ 204 MHz
	 * Se que 1 us es igual a 204 ciclos
	 */
	volatile uint32_t startCycles = cyclesCounterRead();
	uint32_t cyclesToReach = delay_us * NUMBER_CYCLES_1US;

	/*
	 * Debo considerar una latencia de 0.7 us que corresponde al gpioWrite();
	 * 143 ciclos	= 0.7 us
	 * Esa cantidad de ciclos debo restarla a cyclesToReach para obtener así
	 * en forma precisa una señal de delay_us
	 */
	cyclesToReach = cyclesToReach - NUMBER_CYCLES_TO_ADJUST;

	/* While mientras sea menor a 1 us */
	while((cyclesCounterRead() - startCycles) < cyclesToReach) {
	}
}

static void ONE_WIRE_delay250Ns(uint32_t delay_250Ns) {
	/*
	 * Esta funcion genera 'ticks' de 250 ns
	 * El valor mínimo que se le puede pasar es '3'
	 * 1 ciclo 		= 4.902 ns
	 * 51 ciclos 	= 250 ns		@ 204 MHz
	 */
	volatile uint32_t startCycles = cyclesCounterRead();
	uint32_t cyclesToReach = delay_250Ns * NUMBER_CYCLES_250NS;

	if(cyclesToReach < NUMBER_CYCLES_TO_ADJUST)
		return;
	/*
	 * Debo considerar una latencia de 0.7 us que corresponde al gpioWrite();
	 * 143 ciclos	= 0.7 us
	 * Esa cantidad de ciclos debo restarla a cyclesToReach para obtener así
	 * en forma precisa una señal de delay_250Ns
	 */
	cyclesToReach = cyclesToReach - NUMBER_CYCLES_TO_ADJUST;

	/* While mientras sea menor a 250 ns */
	while((cyclesCounterRead() - startCycles) < cyclesToReach) {
	}
}

/*==================[external functions definition]==========================*/

void ONE_WIRE_config(gpioMap_t gpio, uint8_t speed) {
	/*
	 * Configuro puerto 1-Wire
	 */

	/* Configuro GPIO */
	oneWireData.gpio = gpio;
	/* Configuro velocidad */
	ONE_WIRE_setSpeed(speed);
}

oneWireSensorState_t ONE_WIRE_readSensorRomData(oneWireSensorData_t * sensorData) {

	uint8_t byteIndex;

	/* Envio señal de reset para determinar si hay presencia de sensor */
	if(ONE_WIRE_touchReset() == FALSE)
		return (oneWireData.sensorState = ONE_WIRE_SENSOR_NO_PRESENCE);

	/* Envio comando de lectura de codigo de ROM */
	ONE_WIRE_writeByte(ONE_WIRE_COMMAND_READ_ROM);

	/* Leo ROM data */
	for(byteIndex = 0; byteIndex < ROM_CODE_LENGHT; byteIndex++) {
		sensorData->romCode[byteIndex] = ONE_WIRE_readByte();
		/* Guardo datos en variables para acceso inmediato */
		if(byteIndex == ONE_WIRE_ROM_DATA_FAMILY_CODE)
			sensorData->familyCode = sensorData->romCode[byteIndex];
		else if(byteIndex <= ONE_WIRE_ROM_DATA_SN_BYTE6)
			sensorData->serialNumber[byteIndex - 1] = sensorData->romCode[byteIndex];
		else
			sensorData->crc = sensorData->romCode[byteIndex];
	}

	/* Check false contact error */
	if(sensorData->romCode[ONE_WIRE_ROM_DATA_FAMILY_CODE] == 0x00)
		return (oneWireData.sensorState = ONE_WIRE_SENSOR_ERROR);

	/* Verifico CRC */
	if(ONE_WIRE_checkCRC() == FALSE)
		return (oneWireData.sensorState = ONE_WIRE_SENSOR_ERROR);
	else
		return (oneWireData.sensorState = ONE_WIRE_SENSOR_OPERATIONAL);
}

bool_t ONE_WIRE_readAndCompareRomCode(oneWireSensorData_t * dataToCompare) {

	/* Read 1-Wire sensor ROM data */
	if(ONE_WIRE_readSensorRomData(&oneWireData.sensorData) == TRUE) {

		gpioWrite(LEDB, ON);
		/* Compare read data vs dataToCompare */
		if(memcmp(&dataToCompare->romCode, &oneWireData.sensorData.romCode, (ROM_CODE_LENGHT - 1)) == 0)
			return TRUE;
		else
			return FALSE;
	}
	return FALSE;
}

oneWireSensorState_t ONE_WIRE_displaySensorData(void) {

	/* Read 1-Wire sensor ROM data */
	oneWireData.sensorState = ONE_WIRE_readSensorRomData(&oneWireData.sensorData);

	switch(oneWireData.sensorState) {

	case ONE_WIRE_SENSOR_OPERATIONAL:
		printf("ONE_WIRE_SENSOR_OPERATIONAL\r\n");
		printf("ROM Code      %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
				oneWireData.sensorData.romCode[ONE_WIRE_ROM_DATA_FAMILY_CODE],
				oneWireData.sensorData.romCode[ONE_WIRE_ROM_DATA_SN_BYTE1],
				oneWireData.sensorData.romCode[ONE_WIRE_ROM_DATA_SN_BYTE2],
				oneWireData.sensorData.romCode[ONE_WIRE_ROM_DATA_SN_BYTE3],
				oneWireData.sensorData.romCode[ONE_WIRE_ROM_DATA_SN_BYTE4],
				oneWireData.sensorData.romCode[ONE_WIRE_ROM_DATA_SN_BYTE5],
				oneWireData.sensorData.romCode[ONE_WIRE_ROM_DATA_SN_BYTE6],
				oneWireData.sensorData.romCode[ONE_WIRE_ROM_DATA_CRC]);
		printf("Family Code   %02X\r\n",
				oneWireData.sensorData.romCode[ONE_WIRE_ROM_DATA_FAMILY_CODE]);
		printf("Serial Number %02X %02X %02X %02X %02X %02X\r\n",
				oneWireData.sensorData.romCode[ONE_WIRE_ROM_DATA_SN_BYTE1],
				oneWireData.sensorData.romCode[ONE_WIRE_ROM_DATA_SN_BYTE2],
				oneWireData.sensorData.romCode[ONE_WIRE_ROM_DATA_SN_BYTE3],
				oneWireData.sensorData.romCode[ONE_WIRE_ROM_DATA_SN_BYTE4],
				oneWireData.sensorData.romCode[ONE_WIRE_ROM_DATA_SN_BYTE5],
				oneWireData.sensorData.romCode[ONE_WIRE_ROM_DATA_SN_BYTE6]);
		printf("CRC           %02X\r\n",
				oneWireData.sensorData.romCode[ONE_WIRE_ROM_DATA_CRC]);
		return ONE_WIRE_SENSOR_OPERATIONAL;
		break;

	case ONE_WIRE_SENSOR_NO_PRESENCE:
		printf("ONE_WIRE_SENSOR_NO_PRESENCE\r\n");
		return ONE_WIRE_SENSOR_NO_PRESENCE;
		break;

	case ONE_WIRE_SENSOR_ERROR:
		printf("ONE_WIRE_SENSOR_ERROR\r\n");
		return ONE_WIRE_SENSOR_ERROR;
		break;

	default:
		break;
	}
}

bool_t ONE_WIRE_checkSensorPresence(void) {

	/* Envio señal de reset para determinar si hay presencia de sensor */
	if(ONE_WIRE_touchReset() == FALSE)
		return (oneWireData.sensorState = ONE_WIRE_SENSOR_NO_PRESENCE);

	return (oneWireData.sensorState = ONE_WIRE_SENSOR_OPERATIONAL);
}

/*==================[end of file]============================================*/
