/*
* ----------------------------------------------------------------------------
* “THE COFFEEWARE LICENSE” (Revision 1):
* <ihsan@kehribar.me> wrote this file. As long as you retain this notice you
* can do whatever you want with this stuff. If we meet some day, and you think
* this stuff is worth it, you can buy me a coffee in return.
* -----------------------------------------------------------------------------
* This library is based on this library: 
*   https://github.com/aaronds/arduino-nrf24l01
* Which is based on this library: 
*   http://www.tinkerer.eu/AVRLib/nRF24L01
* -----------------------------------------------------------------------------
*/

#include "NRF24.h"
#include <avr/io.h>
#include <avr/pgmspace.h>

///////////////////////////////////////////////////////////////////////////////
//                  Register Settings                                        //
///////////////////////////////////////////////////////////////////////////////

const uint16_t nRF24_Config[] PROGMEM =
{
	(CONFIG<<8) 		| 	(1<<MASK_RX_DR) | (1<<MASK_TX_DS) | (1<MASK_MAX_RT) | (1<<EN_CRC) | (1<<CRC0) | (0<<PWR_UP) | (0<<PRIM_RX),	//CRC 2B, 
	(EN_AA<<8)  		| 	0x3F,											//All ACK
	(EN_RX_ADDR<<8) 	| 	0x3F,											//All enable
	(SETUP_AW<<8)		| 	0x03,											//5 bytes adress P0-P5
	(SETUP_RETR<<8)		|	(0x03<<ARD)|(0x0F<<ARC),						//repeat 15 x 1ms
	(RF_CH<<8)			|	2,												//channel 2
	(RF_SETUP<<8)		|	(1<<RF_DR_LOW)|(0<<RF_DR)|(0x03<<RF_PWR),		//250kbps, 0dB out.Pow.
	(STATUS<<8)			|	0xFF,											//Clear Status
	(OBSERVE_TX<<8)		|	0x00, 											//Read Only
	(CD<<8)				|	0x00, 											//Read Only
	(RX_ADDR_P0<<8)		|	0xE7, 											//
	(RX_ADDR_P1<<8)		|	0xC2, 											//
	(RX_ADDR_P2<<8)		|	0xC3, 											//
	(RX_ADDR_P3<<8)		|	0xC4, 											//
	(RX_ADDR_P4<<8)		|	0xC5, 											//
	(RX_ADDR_P5<<8)		|	0xC6, 											//
	(TX_ADDR<<8)		|	0xE7, 											//
	(RX_PW_P0<<8)		|	0x20, 											//
	(RX_PW_P1<<8)		|	0x20, 											//
	(RX_PW_P2<<8)		|	0x20, 											//
	(RX_PW_P3<<8)		|	0x20, 											//
	(RX_PW_P4<<8)		|	0x20, 											//
	(RX_PW_P5<<8)		|	0x20, 											//
	(FIFO_STATUS<<8)	|	0x00, 											//
	(DYNPD<<8)			|	0x3F, 											//
	(FEATURE<<8)		|	(1<<EN_DPL) | (1<<EN_ACK_PAY) | (1<<EN_DYN_ACK)	//
};

///////////////////////////////////////////////////////////////////////////////
//                  SPI access                                               //
///////////////////////////////////////////////////////////////////////////////
        
//Function: SPI_RW();                                               
uint8_t SPI_RW(uint8_t value)                                    
{  
	SPDR = value;					// send data via SPI - put in SPI Data Register
	while(!(SPSR & (1<<SPIF)));	// wait for SPI interrupt flag indicating transmit complete
	return SPDR;					// return value in SPI Data Register
}                                                        
       
//Function: SPI_RW_Reg();                                        
uint8_t SPI_RW_Reg(uint8_t reg, uint8_t value)                               
{                                                           
	CS_0;      				// CSN low, initialize SPI communication...
	SPI_RW(reg);            // Select register to read from..
	value = SPI_RW(value);    	// ..then read register value
	CS_1;         			// CSN high, terminate SPI communication
	
	return(value);        	// return register value
}                                                           
      
//Function: SPI_Read_Buf();                                        
void SPI_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t length)     
{                                                           
	uint8_t byte_ctr;                              

	CS_0;                    	// Set CSN l
	SPI_RW(reg);       		// Select register to write, and read status uint8_t
	for(byte_ctr=0; byte_ctr < length; byte_ctr++)           
		pBuf[byte_ctr] = SPI_RW(0);    	// Perform SPI_RW to read uint8_t from RFM73 
	CS_1;                  	// Set CSN high again
}                                                           
    
//Function: SPI_Write_Buf();                                        
void SPI_Write_Buf(uint8_t reg, uint8_t *pBuf, uint8_t length)    
{                                                           
	uint8_t byte_ctr;                              

	CS_0;           			// Set CSN low, init SPI tranaction
	SPI_RW(reg);    			// Select register to write to and read status uint8_t
	for(byte_ctr=0; byte_ctr < length; byte_ctr++) // then write all uint8_t in buffer(*pBuf) 
		SPI_RW(pBuf[byte_ctr]);                                    
	CS_1;       				// Set CSN high again    
}

///////////////////////////////////////////////////////////////////////////////
//                  Register Access                                          //
///////////////////////////////////////////////////////////////////////////////

//Returns status
uint8_t GetStatus()
{
    uint8_t rv;
	
    CS_0;
    rv = SPI_RW(NOP);
    CS_1;
	
    return rv;
}

/* Set channel number */
void SetChannelNum(uint8_t ch)
{
	SPI_RW_Reg(WRITE_REG | RF_CH, ch);
}

/* Set the RX address P2-P5 */
void SetPxAddress(uint8_t pipe, uint8_t adr)
{
	SPI_RW_Reg(WRITE_REG | pipe, adr); 
}

/* Set the RX address */
void SetRxAddress(uint8_t pipe, uint8_t * adr) 
{
    SPI_Write_Buf(WRITE_REG | pipe, adr, 5);  //nrf24_ADDR_LEN = 5
}

/* Set the TX address */
void SetTxAddress(uint8_t* adr)
{
    /* RX_ADDR_P0 must be set to the sending addr for auto ack to work. */
	SPI_Write_Buf(WRITE_REG | RX_ADDR_P0, adr, 5);
    SPI_Write_Buf(WRITE_REG | TX_ADDR, adr, 5);
}

/* Checks if receive FIFO is empty or not */
uint8_t RxFifoEmpty()
{
    uint8_t fifoStatus;

    fifoStatus = SPI_RW_Reg(FIFO_STATUS, 0);
    fifoStatus = fifoStatus & FIFO_STATUS_RX_EMPTY;
	
    return fifoStatus;
}

/* Returns the length of data waiting in the RX fifo */
uint8_t PayloadLength()
{
    uint8_t status;
	
    status = SPI_RW_Reg(R_RX_PL_WID, 0);

    return status;
}

/* Returns the number of retransmissions occured for the last message */
uint8_t RetransmissionCount()
{
    uint8_t rv;
	
    rv = SPI_RW_Reg(OBSERVE_TX, 0);
	
    return rv;
}

//Is sending?
uint8_t IsSending()
{
    uint8_t status;

    /* read the current status */
    status = GetStatus();
    /* if sending successful (TX_DS) or max retries exceded (MAX_RT). */
    if(status & (STATUS_TX_DS | STATUS_MAX_RT))
    {        
        CE_0;
		return 0; /* false */
    }
    return 1; /* true */
}

/* Checks if data is available for reading */
/* Returns 1 if data is ready ... */
uint8_t DataReady() 
{
    // See note in getData() function - just checking RX_DR isn't good enough
    uint8_t status;

    status = GetStatus();
	// We can short circuit on RX_DR, but if it's not set, we still need
    // to check the FIFO for any pending packets
    if (status & STATUS_RX_DR) 
    {
        return 1;
    }
	status = !RxFifoEmpty();
	
    return status;
}

//Flush Rx FIFO
void FlushRx()
{
	SPI_RW_Reg(WRITE_REG | FLUSH_RX, 0);		//flush Rx
}

//Function: SwitchToRxMode();
void RxMode()
{
	uint8_t value;

	SPI_RW_Reg(WRITE_REG | FLUSH_RX, 0);		//flush Rx
	//PRX
	CE_0;
	value = GetStatus();				// read register STATUS's value
	value = value | STATUS_RX_DR;
	value = value & ~(STATUS_TX_DS | STATUS_MAX_RT);
	SPI_RW_Reg(WRITE_REG | STATUS, value); // Set Status
	value = SPI_RW_Reg(CONFIG, 0);	// read register CONFIG's value
	value = value | (1<<PRIM_RX) | (1<<PWR_UP);//set bit 1
	SPI_RW_Reg(WRITE_REG | CONFIG, value); // Set PWR_UP bit, enable CRC(2 length) & Prim:RX. RX_DR enabled..
	CE_1;
}

//Function: SwitchToTxMode();
void TxMode()
{
	uint8_t value;
	
	SPI_RW_Reg(WRITE_REG | FLUSH_TX, 0);		//flush Tx
	//PTX
	CE_0;
	value = GetStatus();	// read register STATUS's value
	value = value | STATUS_TX_DS | STATUS_MAX_RT;
	value = value & ~STATUS_RX_DR;
	SPI_RW_Reg(WRITE_REG | STATUS, value); // Set Status
	value = SPI_RW_Reg(CONFIG, 0);	// read register CONFIG's value
	value = value & ~(1<<PRIM_RX);			//set bit 0
	value = value | (1<<PWR_UP);
	SPI_RW_Reg(WRITE_REG | CONFIG, value); // Set PWR_UP bit, enable CRC(2 length) & Prim:RX. RX_DR enabled.
	CE_1;
}

//Function: SwitchToPowerDown();
void PowerDown()
{
	uint8_t value;
	
	CE_0;
	SPI_RW_Reg(WRITE_REG | FLUSH_RX, 0);//flush Rx
	SPI_RW_Reg(WRITE_REG | FLUSH_TX, 0);//flush Tx
	value = SPI_RW_Reg(CONFIG, 0);	// read register CONFIG's value
	value = value & ~(1<<PWR_UP);
	SPI_RW_Reg(WRITE_REG | CONFIG, value);
}

/* Reads payload bytes into data array */
void GetData(uint8_t* data) 
{
    uint8_t length;
	
	length = SPI_RW_Reg(R_RX_PL_WID, 0); //Read Payload Length
    //read data
	SPI_Read_Buf(RD_RX_PAYLOAD, data, length);
    /* Reset status register */
    SPI_RW_Reg(WRITE_REG | STATUS, STATUS_RX_DR);   
	FlushRx();
}

// Sends a data package to the default address. Be sure to send the correct
// amount of bytes as configured as payload on the receiver.
void SendData(uint8_t* data, uint8_t amount, uint8_t ack) 
{    
    /* Go to Standby-I first */
    CE_0;
    /* Set to transmitter mode , Power up if needed */
    if(ack) ack = WR_TX_PAYLOAD;
    else ack = W_TX_PAYLOAD_NOACK;

	TxMode();
	//Write data to buffer

	SPI_Write_Buf(ack, data, amount); //WR_TX_PAYLOAD 
    /* Start the transmission */
    CE_1;    
}    

//Function: nRF24_Init();                                   
void nRF24_Init()
{
	uint8_t i;
 	uint16_t temp;

	PORT_INIT;
	DDR_INIT;
	SPCR |= (1<<SPE) | (1<<MSTR) | (1<<SPR0);
	//Init registers
	for(i=0;i<(sizeof(nRF24_Config)/2);i++) 
	{
	  temp = pgm_read_word(&nRF24_Config[i]);
	  SPI_RW_Reg(WRITE_REG | (temp>>8), temp & 0xFF);
	}
}	