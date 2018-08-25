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

#ifndef _SAPI_ONE_WIRE_H_
#define _SAPI_ONE_WIRE_H_

/*==================[inclusions]=============================================*/

#include "sapi.h"

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

/* Define number of cycles = 1 us */
#define NUMBER_CYCLES_1US			(EDU_CIAA_NXP_CLOCK_SPEED / 1000000)
/* Define number of cycles = 250 ns */
#define NUMBER_CYCLES_250NS		(NUMBER_CYCLES_1US / 4)
/* Ciclos de ajuste por latencia en gpioWrite (poner en 0 si no interesa) */
#define NUMBER_CYCLES_TO_ADJUST	143
/* Set the 1-Wire timing to 'standard' (=1) or 'overdrive' (=0) */
#define ONE_WIRE_SPEED_STANDARD 	1
#define ONE_WIRE_SPEED_OVERDRIVE	0

#define ROM_CODE_LENGHT				8
#define CRC_POLYNOMIAL_MASK		0x8C

/*==================[typedef]================================================*/

typedef enum {
	ONE_WIRE_SENSOR_NO_PRESENCE,		// 0, FALSE
	ONE_WIRE_SENSOR_OPERATIONAL,		// 1, TRUE
	ONE_WIRE_SENSOR_ERROR
} oneWireSensorState_t;

typedef enum {
	ONE_WIRE_COMMAND_SKIP_OVERDRIVE	= 0x3C,
	ONE_WIRE_COMMAND_READ_ROM 			= 0x33,
	ONE_WIRE_COMMAND_SEARCH_ROM 		= 0xF0,
	ONE_WIRE_COMMAND_MATCH_ROM 		= 0x55,
	ONE_WIRE_COMMAND_SKIP_ROM 			= 0xCC,
} oneWireCommand_t;

typedef enum {
	ONE_WIRE_ROM_DATA_FAMILY_CODE,
	ONE_WIRE_ROM_DATA_SN_BYTE1,
	ONE_WIRE_ROM_DATA_SN_BYTE2,
	ONE_WIRE_ROM_DATA_SN_BYTE3,
	ONE_WIRE_ROM_DATA_SN_BYTE4,
	ONE_WIRE_ROM_DATA_SN_BYTE5,
	ONE_WIRE_ROM_DATA_SN_BYTE6,
	ONE_WIRE_ROM_DATA_CRC
} oneWireROMData;

typedef struct {
	uint8_t romCode[8];			// 64 bit ROM code
	uint8_t familyCode;			// 8 bit family code
	uint8_t serialNumber[6];	// 48 bit Serial number
	uint8_t crc;					// 8 bit CRC
} oneWireSensorData_t;

typedef struct {
	gpioMap_t gpio;
	oneWireSensorData_t sensorData;
	oneWireSensorState_t sensorState;
} oneWireData_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/**
 * @brief			Config gpio and speed for 1-Wire master interface.
 * @param			gpio: gpio 1-Wire interface.
 * @param			speed: define timing to 'standard' (=1) or 'overdrive' (=0).
 */
void ONE_WIRE_config(gpioMap_t gpio, uint8_t speed);

/**
 * @brief			Display via USB_DEBUG sensor ROM data.
 * @return			Sensor state.
 */
oneWireSensorState_t ONE_WIRE_displaySensorData(void);

/**
 * @brief			Check 1-Wire sensor presence.
 * @param			data: gpio 1-Wire interface.
 * @return			Sensor state.
 */
oneWireSensorState_t ONE_WIRE_readSensorRomData(oneWireSensorData_t * data);

/**
 * @brief			Compare read data vs dataToCompare (need to be initialized).
 * @param			dataToCompare: local data to compare vs read data from sensor.
 * @return			Return 1 if data match, return 0 otherwise.
 */
bool_t ONE_WIRE_readAndCompareRomCode(oneWireSensorData_t * dataToCompare);

/**
 * @brief			Check 1-Wire sensor presence.
 * @return			Return 1 if presence detect was found, return 0 otherwise.
 */
bool_t ONE_WIRE_checkSensorPresence(void);

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* #ifndef _SAPI_ONE_WIRE_H_ */
