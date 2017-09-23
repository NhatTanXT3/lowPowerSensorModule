#ifndef MYSERIAL_H_
#define MYSERIAL_H_

void set_float_value (char *string,float *kp);
void set_int_value(unsigned char *string, unsigned char *kp);
void UartSendEvent(unsigned char a1,unsigned char a2);
/* ------------init uart module --------------------
* clock input to module: 16Mhz
* baudrate: 128000bps
* interrupt: enable or disable, you choise
*/
void Uart_Init();

/*------------ send a byte -----------------
*
*/
void UartCharPut(unsigned char data_put);

/*------------ get a byte -----------------
* wait untill uart module receive a byte
*/
unsigned char UartCharGet();

/*------------ send a string-----------------
* terminator characte: '\0' = 0x00h = 0b
*/
void UartStrPut(unsigned char *uart_str);

/*------------ get a string-----------------
* wait untill uart module get a string
* terminator characte: '\0' = 0x00h = 0b
*/
void UartStrGet(unsigned char *uart_str);




//===================================number display=========================================================

/*---------- hi?n th? s? th?p ph�n d?ng chu?i-------------------
 * s? c� d?ng xxx.xx (5 ch? s? c� d?u)
 * k� t? k?t th�c '\n'
 * khuy�n d�ng: h�m t?n r?t nhi?u th?i gian x? l� n�n b?n ph?i
 * ch?c ch?n r�ng k ?nh hu?ng d?n c�c h�m kh�c
 * gi?i h?n s? do bi?n k quy?t d?nh (2byte k d?u)
 * => max,min = +-655.36
 *-----------------------------------------------------------*/
extern void Uart_int_display(int num );

/*---------- hi?n th? s? nguy�n d?ng chu?i-------------------
 * s? c� d?ng xxxxx (5 ch? s? c� d?u)
 * k� t? k?t th�c '\n'
 * ch� �: s? 16bit c� d?u
 *-----------------------------------------------------------*/
void Uart_Float_Display(float num);

#endif

