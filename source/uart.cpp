#include "uart.hpp"

// GPIO20 UART TX
// GPIO21 UART RX

// init UART
void uartInit(void)
{
	uart_init(UART_ID, BAUD_RATE);
	gpio_init(UART_TX_PIN);
	gpio_init(UART_RX_PIN);
	gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
	gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
	uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
	uart_set_fifo_enabled(UART_ID, true);
	gpio_pull_up(UART_RX_PIN);
}

// reinit UART
void uartReInit(void)
{
    uart_deinit(UART_ID);
    uartInit();
}
