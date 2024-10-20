#include "uart.h"
#include "tools.h"

void uart_puts(char *str)
{
	for (int i = 0; str[i] != '\0'; i++)
		uart_putc((char)str[i]);
}

void uart_putc(char ch)
{
	/* Wait until there is space in the FIFO or device is disabled */
	while (get32((unsigned long)PL011_UART0_BASE + UART_FR) &
	       UART_FR_TXFF) {
	}
	/* Send the character */
	put32((unsigned long)PL011_UART0_BASE + UART_DR, (unsigned int)ch);
}

void uart_init(void)
{
	/* Clear all errors */
	put32(PL011_UART0_BASE + UART_RSR_ECR, 0);

	/* Disable everything */
	put32(PL011_UART0_BASE + UART_CR, 0);

	/* Configure TX to 8 bits, 1 stop bit, no parity, fifo disabled. */
	put32(PL011_UART0_BASE + UART_LCR_H, UART_LCRH_WLEN_8);

	/* Enable UART and RX/TX */
	put32(PL011_UART0_BASE + UART_CR,
	      UART_CR_UARTEN | UART_CR_TXE | UART_CR_RXE);
}
