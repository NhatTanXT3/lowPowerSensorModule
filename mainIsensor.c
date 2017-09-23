#include <msp430.h> 
#include "serial.h"
#include "adc.h"
#include "i2c.h"

/*
 * main.c
 */

unsigned int count_10Hz=0;
unsigned char flag_10Hz=0;
unsigned int adc;

void clock_init()
{
	//	if (CALBC1_16MHZ ==0xFF || CALDCO_16MHZ == 0xFF)
	//	{
	//		while(1); // If cal constants erased, trap CPU!!
	//	}
	//	BCSCTL1 = CALBC1_16MHZ; 	// Set range
	//	DCOCTL = CALDCO_16MHZ; 		// Set DCO step + modulation
	//	IFG1 &= ~OFIFG;				// disable oscillator-fault flag
	//	BCSCTL2 |= SELM_0 + DIVM_0; //select DCO/8 for MCLK

	if (CALBC1_1MHZ ==0xFF || CALDCO_1MHZ == 0xFF)
	{
		while(1); // If cal constants erased, trap CPU!!
	}
	BCSCTL1 = CALBC1_1MHZ; 		// Set range
	DCOCTL = CALDCO_1MHZ; 		// Set DCO step + modulation
	IFG1 &= ~OFIFG;				// disable oscillator-fault flag
	BCSCTL2 |= SELM_0 + DIVM_0; //select DCO/8 for MCLK
}
void io_init()
{
	P1DIR |= BIT0 ;
	P1OUT &=~ BIT0;
}

//========================================================
void Timer_A0_Init()
{
	//	TA0CCTL0 = CCIE;
	TA0CCTL1 = CCIE;
	//	TA0CCR0 = 800;
	TA0CCR1 = 64000;
	TA0CTL = TASSEL_2 + MC_2 +TAIE;
}

unsigned char ic2Data[11]={0,1,2,3,4,5,6,7,8,9,10,11};



void main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	clock_init();
	io_init();
	Uart_Init();
	Timer_A0_Init();
	adc_init();
	I2C_Init();
	//	_delay_cycles(1000000);
	UartStrPut("hello");


	while(1){
		__bis_SR_register(CPUOFF+GIE);        // Enter LPM0, enable interrupts
		if(flag_10Hz ==1)
		{
			flag_10Hz=0;

		}

	}

}
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USI_NACK(void)
{
	StopCondition();
	//	UartCharPut(UCB0STAT);
	TXByteCtr=0;
	I2C_startAddressCount=0;
	i2cBusyFlag=0;
}


#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
	if(IFG2 & UCB0TXIFG){
		P1OUT |= BIT0;
		switch(I2C_startAddressCount){
		case 0:
			UCB0TXBUF=(unsigned char)dualAddress>>8;
			I2C_startAddressCount++;
			break;
		case 1:
			UCB0TXBUF=(unsigned char)dualAddress;
			I2C_startAddressCount++;
			break;
		default:
			if (TXByteCtr)                            // Check TX byte counter
			{
				UCB0TXBUF = *PTxData++;                 // Load TX buffer
				TXByteCtr--;                            // Decrement TX byte counter
			}
			else
			{
				UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
				IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag
				//    __bic_SR_register_on_exit(CPUOFF);      // Exit LPM0
				I2C_startAddressCount=0;
				i2cBusyFlag=0;
			}
			break;
		}
		P1OUT &=~ BIT0;
	}
	else if(IFG2 & UCB0RXIFG){
		RXByteCtr--;                              // Decrement RX byte counter
		if (RXByteCtr)
		{
			*PRxData++ = UCB0RXBUF;                 // Move RX data to address PRxData
			if (RXByteCtr == 1)                     // Only one byte left?
				UCB0CTL1 |= UCTXSTP;                  // Generate I2C stop condition
		}
		else
		{
			*PRxData = UCB0RXBUF;                   // Move final RX data to PRxData
			i2cBusyFlag=0;
		}
	}
}

unsigned char task=0;
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer_A1 (void)
{
	switch( TA0IV )
	{
	case  2:
	{
		//		TA0CCR1 += 64000;// Add Offset to CCR1
		TA0CCR1 += 4000;// Add Offset to CCR1
		count_10Hz++;
		if (count_10Hz==25)
		{
			count_10Hz=0;
			flag_10Hz=1;

			switch (task){
			case 0:
				if(i2cBusyFlag==0)
				{
					I2C_Write_DualAddress_2(SLAVE_ADDRESS_EEPROM,0,ic2Data,0);
					//				I2C_Read_DualAddress_2(SLAVE_ADDRESS_EEPROM,0,10);
				}
				task++;
				break;
			case 1:
				if(i2cBusyFlag==0)
				{
					//					I2C_Write_DualAddress_2(SLAVE_ADDRESS_EEPROM,0,ic2Data,10);
					I2C_Read_DualAddress_2(SLAVE_ADDRESS_EEPROM,0,10);
				}
				task++;
				break;
			default:
				task=0;
				break;
			}




		}
		//		P1OUT ^= 0x10;
		break;
	}
	case 10: {  break;}                 // Timer_A3 overflow
	}

}
