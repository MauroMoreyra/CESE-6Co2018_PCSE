/*==================[inclusiones]============================================*/

#include "main.h"
#include "sapi.h"

#include "sapi_oneWire.h"
#include "sapi_ibutton.h"

/*==================[definiciones y macros]==================================*/

DEBUG_PRINT_ENABLE

/*==================[definiciones de datos internos]=========================*/

/*==================[definiciones de datos externos]=========================*/

/*==================[declaraciones de funciones internas]====================*/

/*==================[declaraciones de funciones externas]====================*/

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main(void) {

	oneWireSensorState_t sensorState;
	oneWireSensorData_t sensorData;
	oneWireSensorData_t sensorDataLocal;

	// Inicializar y configurar la plataforma
	boardConfig();

	/* UART for debug messages */
	debugPrintConfigUart(UART_USB, 115200);
	debugPrintlnString("Ejemplo de uso sensor iButton DS1990A.");

	/* Configuro GPIO1 para uso de 1-Wire, velocidad estandar*/
	ONE_WIRE_config(GPIO1, ONE_WIRE_SPEED_STANDARD);

	/* Inicializo dataLocal para comparar */
	// Data ibutton que poseo: 01 CA 93 F1 18 00 00
	// Usar memcpy ?
	sensorDataLocal.romCode[ONE_WIRE_ROM_DATA_FAMILY_CODE] = 0x01;
	sensorDataLocal.romCode[ONE_WIRE_ROM_DATA_SN_BYTE1] = 0xca;
	sensorDataLocal.romCode[ONE_WIRE_ROM_DATA_SN_BYTE2] = 0x93;
	sensorDataLocal.romCode[ONE_WIRE_ROM_DATA_SN_BYTE3] = 0xf1;
	sensorDataLocal.romCode[ONE_WIRE_ROM_DATA_SN_BYTE4] = 0x18;
	sensorDataLocal.romCode[ONE_WIRE_ROM_DATA_SN_BYTE5] = 0x00;
	sensorDataLocal.romCode[ONE_WIRE_ROM_DATA_SN_BYTE6] = 0x00;

	while(TRUE) {

		delay(500);

		/* Example readAndCompareRomCode
		if(ONE_WIRE_readAndCompareRomCode(&sensorDataLocal) == TRUE) {
			printf("TRUE\r\n");
		} else {
			printf("FALSE\r\n");
		}
		*/

		/* Example displaySensorData */
		ONE_WIRE_displaySensorData();


		/* Example check 1-Wire sensor presence
		if(ONE_WIRE_checkSensorPresence() == TRUE) {
			printf("ONE_WIRE_SENSOR_OPERATIONAL\r\n");
		} else {
			printf("ONE_WIRE_SENSOR_NO_PRESENCE\r\n");
		}
		*/
	}

	return 0;
}
	/*==================[definiciones de funciones internas]=====================*/

	/*==================[definiciones de funciones externas]=====================*/

	/*==================[fin del archivo]========================================*/
