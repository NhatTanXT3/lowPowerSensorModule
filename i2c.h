#ifndef MYI2C_H_
#define MYI2C_H_
#define SLAVE_ADDRESS_EEPROM 0x51

extern void i2c_min_speed();
extern void i2c_max_speed();
extern void StartTx();

extern void StartRx();

extern void StopCondition();

extern void I2C_Init();

extern unsigned char I2C_Write(unsigned char I2C_SlaveAddress, unsigned char I2C_StartAddress, unsigned char *I2C_Data, unsigned char Size);

extern unsigned char I2C_Read(unsigned char I2C_SlaveAddress, unsigned char I2C_StartAddress, unsigned char *I2C_Data, unsigned char Size);

extern unsigned char I2C_Write_DualAddress(unsigned char I2C_SlaveAddress, short I2C_StartAddress, unsigned char *I2C_Data, unsigned char Size);

extern unsigned char I2C_Read_DualAddress(unsigned char I2C_SlaveAddress, short I2C_StartAddress, unsigned char *I2C_Data, unsigned char Size);

extern unsigned char I2C_Write_1byte(unsigned char I2C_SlaveAddress, unsigned char I2C_StartAddress, unsigned char I2C_Data);

extern unsigned char I2C_Write_DualAddress_2(unsigned char I2C_SlaveAddress, short I2C_StartAddress, unsigned char *I2C_Data, unsigned char Size);

extern unsigned char *PTxData;                     // Pointer to TX data
extern unsigned char TXByteCtr;
extern unsigned char I2C_startAddressCount;
extern short dualAddress;
extern unsigned char i2cBusyFlag;

extern unsigned char I2C_Read_DualAddress_2(unsigned char I2C_SlaveAddress, short I2C_StartAddress, unsigned char Size);
extern volatile unsigned char RxBuffer;
extern unsigned char *PRxData;                     // Pointer to RX data
extern unsigned char RXByteCtr;
#endif
