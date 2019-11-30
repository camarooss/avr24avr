
/*
 * M328_NRF24.c
 * Created: 2014-08-10 14:51:16
 */ 

#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <string.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "NRF24.h"
//#include "LCD.h"

/* ------------------------------------------------------------------------- */
uint8_t RX0_Address[5] = {0x30,0x71,0x12,0xE2,0x30};
uint8_t RX1_Address[5] = {0x31,0x71,0x12,0xE2,0x30};
/* ------------------------------------------------------------------------- */

int main()
{
	uint8_t i, data[33];

	//LCD_INIT();
	//DDRB = 0xFF;
	//Inicjalizacja
	nRF24_Init();
	//Ustaw adresy dla poszczególnych kana³ów
	SetTxAddress(RX0_Address);	
	SetRxAddress(RX_ADDR_P1, RX1_Address);
	SetPxAddress(RX_ADDR_P2, 0x32);
	SetPxAddress(RX_ADDR_P3, 0x33);
	SetPxAddress(RX_ADDR_P4, 0x34);
	SetPxAddress(RX_ADDR_P5, 0x35);
	RxMode();
	while(1)
	{
		if(DataReady())
		{
			i= PayloadLength();		//iloœæ bajtów do odebrania
			GetData(data);			//pobierz dane z FIFO
			if(data[0]=='P') PORTB = data[1]; //zróbmy cos z danymi..
			else LCD_STR(data);	//..wyœwietl odebrane dane
		}
	}
}
