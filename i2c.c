#include <msp430g2553.h>
#include "serial.h"

void i2c_min_speed()
{
	UCB0BR0 = 10;
	_delay_cycles(20);
}
void i2c_max_speed()
{
	UCB0BR0 =1;
	_delay_cycles(20);
}
/* ------ start condition for transmitting ----------
 *
 */
void StartTx()
{
	UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
}

/* ------ ------start condition for receiving ----------
 *
 */
void StartRx()
{
	UCB0CTL1 &= ~UCTR;                      // I2C RX
	UCB0CTL1 |= UCTXSTT;                    // I2C start condition
}

/* ----------stop condition -------------------------
 *
 */
void StopCondition()
{
	UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
}


/* ----------- init i2c module------------------------
 * 100khz at 16Mhz clock
 *
 */
void I2C_Init()
{
	UCB0CTL1 |= UCSWRST;                      // Enable SW reset


	P1SEL |= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
	P1SEL2|= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
	UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode


	UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
	i2c_min_speed();                           // fSCL = SMCLK/160 = 100kHz (SMCLK = 16MHz)
	UCB0BR1 = 0;



	UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
	UCB0I2CIE|=UCNACKIE;
	IE2 |= UCB0TXIE+ UCB0RXIE; ;                          // Enable TX interrupt

}


unsigned char I2C_Write_1byte(unsigned char I2C_SlaveAddress, unsigned char I2C_StartAddress, unsigned char I2C_Data)
{
	UCB0I2CSA = I2C_SlaveAddress;            // Set slave address
	while (UCB0CTL1 & UCTXSTP);				// Ensure stop condition got sent, ensure the last transmit is complete

	StartTx();

	while (!(IFG2 & UCB0TXIFG));			//wait for TX buffer is free
	UCB0TXBUF= I2C_StartAddress;

	//------wait and check ack after slave address ---
	while(UCB0CTL1 & UCTXSTT);
	//		{if (UCB0STAT & UCNACKIFG);return 1;}
	if (UCB0STAT & UCNACKIFG)
		return 1;
	//------------------------- end-------------------

	while (!(IFG2 & UCB0TXIFG));			//wait for TX buffer is free
	UCB0TXBUF= I2C_Data;
	while (!(IFG2 & UCB0TXIFG));			//wait for TX buffer is free, data transmission has begun, stop condition can be set now
	StopCondition();

	return 0;

}
unsigned char I2C_Write(unsigned char I2C_SlaveAddress, unsigned char I2C_StartAddress, unsigned char *I2C_Data, unsigned char Size)
{
	UCB0I2CSA = I2C_SlaveAddress;            // Set slave address
	while (UCB0CTL1 & UCTXSTP);				// Ensure stop condition got sent, ensure the last transmit is complete

	StartTx();

	while (!(IFG2 & UCB0TXIFG));			//wait for TX buffer is free
	UCB0TXBUF= I2C_StartAddress;

	//------wait and check ack after slave address ---
	while(UCB0CTL1 & UCTXSTT);
	if (UCB0STAT & UCNACKIFG)
		return 1;
	//------------------------- end-------------------

	unsigned char iii;
	for (iii=1;iii<=Size;iii++)
	{
		while (!(IFG2 & UCB0TXIFG));			//wait for TX buffer is free
		UCB0TXBUF= *I2C_Data++;
	}
	while (!(IFG2 & UCB0TXIFG));			//wait for TX buffer is free, data transmission has begun, stop condition can be set now
	StopCondition();

	return 0;

}

unsigned char I2C_Write_DualAddress(unsigned char I2C_SlaveAddress, short I2C_StartAddress, unsigned char *I2C_Data, unsigned char Size)
{
	UCB0I2CSA = I2C_SlaveAddress;            // Set slave address
	while (UCB0CTL1 & UCTXSTP);				// Ensure stop condition got sent, ensure the last transmit is complete
	StartTx();

	//	P1OUT |= BIT0;
	//		while (!(IFG2 & UCB0TXIFG));			//wait for TX buffer is free
	//	P1OUT &=~ BIT0;

	UCB0TXBUF= (unsigned char)(I2C_StartAddress>>8);


	//------wait and check ack after slave address ---
	while(UCB0CTL1 & UCTXSTT);
	if (UCB0STAT & UCNACKIFG)
	{
		StopCondition();
		return 1;
	}

	//---------------------wait for byte 1 tranfer to shift resister and buffer free-------------------
	P1OUT |= BIT0;
	while (!(IFG2 & UCB0TXIFG));			//wait for TX buffer is free
	P1OUT &=~ BIT0;
	//	UCB0TXBUF= (unsigned char)(I2C_StartAddress>>8);

	UCB0TXBUF= (unsigned char)(I2C_StartAddress);



	unsigned char iii;
	for (iii=1;iii<=Size;iii++)
	{
		//------wait and check ack after slave address ---
		P1OUT |= BIT0;
		while ((!(IFG2 & UCB0TXIFG)))	//wait for TX buffer is free
		{
			if (UCB0STAT & UCNACKIFG)
			{
				//				UCB0CTL1 |= UCSWRST;                      // Enable SW reset
				StopCondition();
				return 2;
			}
		}

		if (UCB0STAT & UCNACKIFG)
		{
			StopCondition();
			return 3;
		}
		P1OUT &=~ BIT0;
		UCB0TXBUF= *I2C_Data++;

	}
	//	while (!(IFG2 & UCB0TXIFG));			//wait for TX buffer is free, data transmission has begun, stop condition can be set now
	StopCondition();

	return 0;

}

unsigned char *PTxData;                     // Pointer to TX data
unsigned char TXByteCtr;
unsigned char I2C_startAddressCount=0;
short dualAddress=0;
unsigned char i2cBusyFlag=0;
unsigned char I2C_Write_DualAddress_2(unsigned char I2C_SlaveAddress, short I2C_StartAddress, unsigned char *I2C_Data, unsigned char Size)
{
	UCB0I2CSA = I2C_SlaveAddress;            // Set slave address
	dualAddress=I2C_StartAddress;
	PTxData=I2C_Data;    // TX array start address
	TXByteCtr= Size;              // Load TX byte counter
	if(UCB0CTL1 & UCTXSTP)             // Ensure stop condition got sent
		return 1;
	UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
	i2cBusyFlag=1;
	return 0;
}

unsigned char I2C_Read(unsigned char I2C_SlaveAddress, unsigned char I2C_StartAddress, unsigned char *I2C_Data, unsigned char Size)
{
	UCB0I2CSA = I2C_SlaveAddress;            // Set slave address
	while (UCB0CTL1 & UCTXSTP);				// Ensure stop condition got sent, ensure the last transmit is complete

	StartTx();

	while (!(IFG2 & UCB0TXIFG));			//wait for TX buffer is free
	UCB0TXBUF= I2C_StartAddress;

	//------wait and check ack after slave address ---
	while(UCB0CTL1 & UCTXSTT);
	if (UCB0STAT & UCNACKIFG)
		return 1;
	//------------------------- end-------------------



	StartRx();								//repeat condition for read

	//------wait and check ack after slave address---
	while(UCB0CTL1 & UCTXSTT);
	if (UCB0STAT & UCNACKIFG)
		return 1;
	//------------------------- end-------------------


	if (Size==1)
	{
		StopCondition();
		while(!(IFG2 & UCB0RXIFG));				//wait for data received
		*I2C_Data++ = UCB0RXBUF;

	}
	else
	{
		unsigned char iiii;
		for (iiii=1;iiii<Size;iiii++)
		{
			while(!(IFG2 & UCB0RXIFG));				//wait for data received
			*I2C_Data++ = UCB0RXBUF;
		}
		StopCondition();
		while(!(IFG2 & UCB0RXIFG));				//wait for data received
		*I2C_Data++ = UCB0RXBUF;

	}
	return 0;
}


volatile unsigned char RxBuffer[128];       // Allocate 128 byte of RAM
unsigned char *PRxData= (unsigned char *)RxBuffer;    // Start of RX buffer);                     // Pointer to RX data
unsigned char RXByteCtr;
unsigned char I2C_Read_DualAddress_2(unsigned char I2C_SlaveAddress, short I2C_StartAddress, unsigned char Size)
{
	UCB0I2CSA = I2C_SlaveAddress;            // Set slave address
	RXByteCtr = Size;
	UCB0CTL1 &= ~UCTR;                      // I2C RX
	UCB0CTL1 |= UCTXSTT;                    // I2C start condition
	i2cBusyFlag=1;
	return 0;
}
unsigned char I2C_Read_DualAddress(unsigned char I2C_SlaveAddress, short I2C_StartAddress, unsigned char *I2C_Data, unsigned char Size)
{
	UCB0I2CSA = I2C_SlaveAddress;            // Set slave address
	while (UCB0CTL1 & UCTXSTP);				// Ensure stop condition got sent, ensure the last transmit is complete

	StartTx();

	while (!(IFG2 & UCB0TXIFG));			//wait for TX buffer is free
	UCB0TXBUF= (unsigned char)(I2C_StartAddress>>8);

	//------wait and check ack after slave address ---
	while(UCB0CTL1 & UCTXSTT);
	if (UCB0STAT & UCNACKIFG)
		return 1;

	while (!(IFG2 & UCB0TXIFG));			//wait for TX buffer is free
	UCB0TXBUF= (unsigned char)(I2C_StartAddress);

	//	while(UCB0CTL1 & UCTXSTT);
	//	if (UCB0STAT & UCNACKIFG)
	//		return 1;

	while (!(IFG2 & UCB0TXIFG));			//wait for TX buffer is free


	StartRx();								//repeat condition for read
	//------wait and check ack after slave address---
	while(UCB0CTL1 & UCTXSTT);
	if (UCB0STAT & UCNACKIFG)
		return 1;
	//------------------------- end-------------------


	if (Size==1)
	{
		StopCondition();
		while(!(IFG2 & UCB0RXIFG));				//wait for data received
		*I2C_Data++ = UCB0RXBUF;

	}
	else
	{
		unsigned char iiii;
		for (iiii=1;iiii<Size;iiii++)
		{
			while(!(IFG2 & UCB0RXIFG));				//wait for data received
			*I2C_Data++ = UCB0RXBUF;
		}
		StopCondition();
		while(!(IFG2 & UCB0RXIFG));				//wait for data received
		*I2C_Data++ = UCB0RXBUF;

	}
	return 0;
}
