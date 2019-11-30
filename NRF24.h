/*

*/

#ifndef NRF24
#define NRF24

#include "NRF24L01.h"
#include <stdint.h>

#define	IRQ		(1<<0)	//PB
#define	CE		(1<<1)	//PB
#define	CS		(1<<2)	//PB
#define MOSI	(1<<3)	//PB
#define MISO	(1<<4)	//PB
#define SCK		(1<<5)	//PB

#define DDR_INIT	DDRB |= CS; DDRB |= CE; DDRB &= ~IRQ; DDRB |= MOSI; DDRB |= SCK; DDRB &= ~MISO; DDRB |= 1<<2;
#define PORT_INIT	PORTB &= ~CE; PORTB |= CS; PORTB |= IRQ; PORTB &= ~MOSI; PORTB |= MISO; PORTB &= ~SCK;

#define CE_0	PORTB &= ~CE
#define CS_0	PORTB &= ~CS
#define CE_1	PORTB |= CE
#define CS_1	PORTB |= CS

uint8_t SPI_RW(uint8_t value);                                       
uint8_t SPI_RW_Reg(uint8_t reg, uint8_t value);          
void SPI_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t length);                                     
void SPI_Write_Buf(uint8_t reg, uint8_t *pBuf, uint8_t length);
uint8_t GetStatus();
void SetChannelNum(uint8_t ch);
void SetPxAddress(uint8_t pipe, uint8_t adr);
void SetRxAddress(uint8_t pipe, uint8_t * adr);
void SetTxAddress(uint8_t* adr);
uint8_t RxFifoEmpty();
uint8_t PayloadLength();
uint8_t RetransmissionCount();
uint8_t IsSending();
uint8_t DataReady();
void FlushRx();
void RxMode();
void TxMode();
void PowerDown();
void GetData(uint8_t* data);
void SendData(uint8_t* data, uint8_t amount, uint8_t ack); 
void nRF24_Init();


#endif
