#ifndef _UART_H_
#define _UART_H_

int  UART_Tstc(void);
int  UART_ReadByte(void);
void UART_WriteByte(const char c);
void UART_Init(int ch, unsigned int clock);
void UART_ConsoleRegister(int ch, unsigned int clock);

#endif
