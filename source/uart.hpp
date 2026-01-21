#ifndef UART_HPP
#define UART_HPP

#include "pico/stdlib.h"

#define UART_ID uart1
#define BAUD_RATE 9600
#define UART_TX_PIN 20
#define UART_RX_PIN 21

void uartInit(void);
void uartReInit(void);

#endif
