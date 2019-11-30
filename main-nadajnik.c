
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
	uint8_t i, data[] = {"ALAMAKOTA TRALALALA"};

	//LCD_INIT();
	//Inicjalizacja
	nRF24_Init();
	//Ustaw adresy dla poszczeg�lnych kana��w
	SetTxAddress(RX0_Address);
	SetRxAddress(RX_ADDR_P1, RX1_Address);
	SetPxAddress(RX_ADDR_P2, 0x32);
	SetPxAddress(RX_ADDR_P3, 0x33);
	SetPxAddress(RX_ADDR_P4, 0x34);
	SetPxAddress(RX_ADDR_P5, 0x35);
	while(1)
	{
		SendData(data, sizeof(data), NOACK);	//wy�lij pakiet bez potwierdzenia odbioru
		while(IsSending());
		_delay_ms(500);
		data[0] = 'P';
		data[1] = PINB;
		SendData(data, 2, ACK);		//wy�lij 2 bajty pakietu ale tym razem z potwierdzeniem odbioru
		while(IsSending());
		i = 0x0F & RetransmissionCount();	//sprawd�my ile razy trzeba by�o powt�rzy� pakiet zanim dotar�
		if(i==0) LCD_STR("Pakiet dotar� za pierwszym razem :-)");
		else 	if(i<15) 
				{
					LCD_STR("Pakiet dotar� za podej�ciem nr: ");
					LCD_INT(i);
				}
				else
					LCD_STR("Pakiet nie dotar� :-(");
		_delay_ms(500);
	}
}			

