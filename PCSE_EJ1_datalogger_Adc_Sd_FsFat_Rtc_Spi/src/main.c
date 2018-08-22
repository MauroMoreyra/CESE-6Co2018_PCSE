/*
 * Ejercicio: SPI, Tarjeta SD, FS FAT y RTC
 *
 * Utilizando los ejemplos provistos se desea guardar un log de datos de medi-
 * ciones tomadas por el ADC, en un único archivo. Además incluir en cada mues-
 * tra un time-stamp de cuando la misma fue obtenida utilizando el periférico
 * RTC. El archivo obtenido deberá contener líneas con el siguiente formato:
 *
 * CH1;CH2;CH3;YYYY/MM/DD_hh:mm:ss;
 *
 * Por ejemplo:
 * Archivo "Muestras.txt"
 * 155;1;24;2018/07/10_20:05:36;
 * 154;0;425;2018/07/10_20:05:37;
 */

/*==================[inlcusiones]============================================*/

#include "main.h"	// <= own header (optional)
#include "sapi.h"	// <= sAPI header
#include "ff.h"	// <= Biblioteca FAT FS
#include "uart.h"
#include "string.h"

/*==================[definiciones y macros]==================================*/

#define FILENAME "Muestras.txt"
#define RTC_BUFFER_LENGHT 20		// Tamaño buffer que almacena datos rtc
#define MUE_BUFFER_LENGHT 50		// Tamaño buffer que almacena datos muestras

/*==================[definiciones de datos internos]=========================*/

static FATFS fs;           // <-- FatFs work area needed for each volume
static FIL fp;             // <-- File object needed for each open file
static char uartBuff[10];
static uartData_t uartConsola;
static uint8_t rtcBuffer[RTC_BUFFER_LENGHT];

/*==================[definiciones de datos externos]=========================*/

/*==================[declaraciones de funciones internas]====================*/

void EJ_showDateAndTime(rtc_t * rtc);

/*==================[declaraciones de funciones externas]====================*/

// FUNCION que se ejecuta cada vezque ocurre un Tick
void diskTickHook(void * ptr);

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main(void) {

	rtc_t rtc;					// datos del RTC
	uint8_t ch;
	uint8_t i;
	uint8_t muestrasString[MUE_BUFFER_LENGHT];	// string a grabar en memoria SD
	uint16_t mulVec[4] = { 1, 10, 100, 1000 };	// constantes de multiplicacion
	delay_t delay1s;					// delay no bloqueante
	delay_t delay1;					// delay no bloqueante
	UINT nbytes;
	uint16_t muestra = 0; 	// Variable para almacenar valor leido del ADC CH1

	// Inicializar y configurar la plataforma
	boardConfig();
	// SPI configuration
	spiConfig(SPI0);
	// UART Consola Config
	UART_init(&uartConsola, UART_USB, 115200);
	UART_clearScreen(&uartConsola);
	UART_update(&uartConsola);

	/* Imprimo menu */
	UART_writeStringToBuffer("Configuracion RTC\n\r", &uartConsola);
	UART_writeStringToBuffer("Ingresar dia y hora actuales\n\r", &uartConsola);
	UART_writeStringToBuffer("Formato: YYYY/MM/DD hh:mm:ss\n\r", &uartConsola);
	UART_update(&uartConsola);

	/* Configuracion del RTC, user ingresa YYYY/MM/DD hh:mm:ss */
	for(i = 0; i < 14; i++) {
		while(UART_getCharFromBuffer(&ch, &uartConsola) == FALSE) {
			UART_update(&uartConsola);
		}
		UART_writeCharToBuffer(ch, &uartConsola);
		rtcBuffer[i] = ch;
	}
	UART_writeStringToBuffer("\n\rOK\n\r", &uartConsola);
	UART_update(&uartConsola);

	// Calculo año
	rtc.year = 0;
	for(i = 0; i < 4; i++) {
		rtc.year += (rtcBuffer[i] - '0') * mulVec[3 - i];
	}
	// Calculo mes
	rtc.month = 0;
	for(i = 0; i < 2; i++) {
		rtc.month += (rtcBuffer[4 + i] - '0') * mulVec[1 - i];
	}
	// Calculo dia
	rtc.mday = 0;
	for(i = 0; i < 2; i++) {
		rtc.mday += (rtcBuffer[6 + i] - '0') * mulVec[1 - i];
	}
	// Calculo hora
	rtc.hour = 0;
	for(i = 0; i < 2; i++) {
		rtc.hour += (rtcBuffer[8 + i] - '0') * mulVec[1 - i];
	}
	// Calculo min
	rtc.min = 0;
	for(i = 0; i < 2; i++) {
		rtc.min += (rtcBuffer[10 + i] - '0') * mulVec[1 - i];
	}
	// Calculo sec
	rtc.sec = 0;
	for(i = 0; i < 2; i++) {
		rtc.sec += (rtcBuffer[12 + i] - '0') * mulVec[1 - i];
	}
	// No importa
	rtc.wday = 1;

	/* Inicializar RTC: cuidado! tarda aprox 10 segundos en hacerlo */
	rtcConfig(&rtc);
	// El RTC tarda en setear la hora, por eso el delay
	delay(2000);

	/* Inicializar conteo de Ticks con resol. 10ms y tickHook: diskTickHook */
	tickConfig(10);
	tickCallbackSet(diskTickHook, NULL);

	/* Inicializar ADC */
	adcConfig(ADC_ENABLE);

	/* Inicializo File System */
	// Give a work area to the default drive
	if(f_mount(&fs, "", 0) != FR_OK) {
		// If this fails, it means that the function could
		// not register a file system object.
		// Check whether the SD card is correctly connected
	}

	/* Inicializar delay no bloqueante */
	delayConfig(&delay1, 1000);

	while(TRUE) {

		/* delayRead retorna TRUE cuando se cumple el tiempo de retardo */
		if(delayRead(&delay1)) {
			/* Leo la Entrada Analogica AI0 - ADC0 CH1 */
			muestra = adcRead(CH1);
			/* Conversion de muestra entera a ASCII con base decimal */
			itoa(muestra, uartBuff, 10);   // 10 significa decimal

			/* Genero string: "CH1;CH2;CH3;YYYY/MM/DD_hh:mm:ss;" */
			strcpy(muestrasString, uartBuff);
			strcat(muestrasString, ";");
			/* Leo la Entrada Analogica CH2 */
			muestra = adcRead(CH2);
			itoa(muestra, uartBuff, 10);
			strcat(muestrasString, uartBuff);
			strcat(muestrasString, ";");
			/* Leo la Entrada Analogica CH3 */
			muestra = adcRead(CH3);
			itoa(muestra, uartBuff, 10);
			strcat(muestrasString, uartBuff);
			strcat(muestrasString, ";");

			/* Leo fecha y hora */
			rtcRead(&rtc);
			/* Mostrar fecha y hora en formato "DD/MM/YYYY, HH:MM:SS" */
			EJ_showDateAndTime(&rtc);
			/* Armo string con datos de muestras + datos del RTC */
			strcat(muestrasString, rtcBuffer);

			// DEBUG
			UART_writeStringToBuffer(muestrasString, &uartConsola);
			UART_update(&uartConsola);

			/* Escribo datos en memoria SD */
			if(f_open(&fp, FILENAME, FA_WRITE | FA_OPEN_APPEND) == FR_OK) {
				/* Escribo datos */
				f_write(&fp, muestrasString, strlen(muestrasString), &nbytes);
				/* Cierro archivo */
				f_close(&fp);
				/* Verifico que se hayan grabado correctamente los datos */
				if(nbytes == strlen(muestrasString)) {
					// Turn ON LEDG if the write operation was successful
					gpioToggle(LEDG);
				}
			} else {
				// Turn ON LEDR if the write operation was fail
				gpioWrite(LEDG, OFF);
				gpioWrite(LEDR, ON);
			}
		}
	}

	return 0;
}

/*==================[definiciones de funciones internas]=====================*/

/* Guarda en rtcBuffer los datos del RTC en formato "YYYY/MM/DD_hh:mm:ss;" */
void EJ_showDateAndTime(rtc_t * rtc) {

	/* Conversion de entero a ascii con base decimal */
	itoa((int) (rtc->year), (char*) uartBuff, 10);
	/* Guardo el año */
	strcpy(rtcBuffer, uartBuff);
	strcat(rtcBuffer, "/");
	/* Guardo el mes */
	itoa((int) (rtc->month), (char*) uartBuff, 10);
	if((rtc->month) < 10)
		strcat(rtcBuffer, "0");
	strcat(rtcBuffer, uartBuff);
	strcat(rtcBuffer, "/");
	/* Guardo el dia */
	itoa((int) (rtc->mday), (char*) uartBuff, 10);
	if((rtc->mday) < 10)
		strcat(rtcBuffer, "0");
	strcat(rtcBuffer, uartBuff);
	strcat(rtcBuffer, "_");
	/* Guardo la hora */
	itoa((int) (rtc->hour), (char*) uartBuff, 10);
	if((rtc->hour) < 10)
		strcat(rtcBuffer, "0");
	strcat(rtcBuffer, uartBuff);
	strcat(rtcBuffer, ":");
	/* Guardo los minutos */
	itoa((int) (rtc->min), (char*) uartBuff, 10);
	// uartBuff[2] = 0;    /* NULL */
	if((rtc->min) < 10)
		strcat(rtcBuffer, "0");
	strcat(rtcBuffer, uartBuff);
	strcat(rtcBuffer, ":");
	/* Guardo los segundos */
	itoa((int) (rtc->sec), (char*) uartBuff, 10);
	if((rtc->sec) < 10)
		strcat(rtcBuffer, "0");
	strcat(rtcBuffer, uartBuff);
	strcat(rtcBuffer, ";");

	// Envio un ENTER
	strcat(rtcBuffer, "\r\n");
}

/*==================[definiciones de funciones externas]=====================*/

// FUNCION que se ejecuta cada vezque ocurre un Tick
void diskTickHook(void * ptr) {
	disk_timerproc();   // Disk timer process
}

/*==================[fin del archivo]========================================*/
