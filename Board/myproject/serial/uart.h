#ifndef __UART_H
#define	__UART_H

#include "nrf51.h"
#include <stdio.h>

#define RX_PIN_NUMBER  11    // UART RX pin number.
#define TX_PIN_NUMBER  9    // UART TX pin number.
#define CTS_PIN_NUMBER 8    // UART Clear To Send pin number. Not used if HWFC is set to false
#define RTS_PIN_NUMBER 10    // Not used if HWFC is set to false
#define HWFC           false // UART hardware flow control


void USART_Configuration(void);
int fputc(int ch, FILE *f);
void UART_send_byte(uint8_t byte);
void UART_Send(uint8_t *Buffer, uint32_t Length);
uint8_t UART_Recive(void);

#endif /* __UART_H */
