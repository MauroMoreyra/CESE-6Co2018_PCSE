/*
 * Ejercicio: Bluetooth
 *
 * Bridge entre PC y BLE con EDU-CIAA-NXP (ppt 05 - Bluetooth)
 */

/*==================[inlcusiones]============================================*/

#include "../../bridgeBluetooth/inc/main.h"	// <= own header (optional)

#include "sapi.h"	// <= sAPI header

/*==================[definiciones y macros]==================================*/

/*==================[definiciones de datos internos]=========================*/

/*==================[definiciones de datos externos]=========================*/

/*==================[declaraciones de funciones internas]====================*/

/*==================[declaraciones de funciones externas]====================*/

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main(void) {

	// Inicializar y configurar la plataforma
	boardConfig();

	uartConfig(UART_USB, 9600);
	uartConfig(UART_232, 9600);

	uint8_t data = 0;

	while( TRUE) {
		// Si leo un dato de una UART lo envio a al otra (bridge)
		if(uartReadByte(UART_USB, &data)) {
			uartWriteByte(UART_232, data);
		}
		if(uartReadByte(UART_232, &data)) {
			uartWriteByte(UART_USB, data);
		}
	}

	return 0;
}

/*==================[definiciones de funciones internas]=====================*/

/*==================[definiciones de funciones externas]=====================*/

/*==================[fin del archivo]========================================*/
