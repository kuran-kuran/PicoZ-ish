#ifndef UART_H
#define UART_H

#include "pico/stdlib.h"

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

#define UART_ID uart1
#define BAUD_RATE 9600
#define UART_TX_PIN 20
#define UART_RX_PIN 21

void uartInit(void);
void uartReInit(void);

#ifdef __cplusplus
}
#endif

#endif
