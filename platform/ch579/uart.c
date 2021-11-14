#include <lk/reg.h>
#include <lib/cbuf.h>
#include <dev/uart.h>
#include <CH57x_common.h>

void uart_init_early(void) {
}

void uart_init(void) {
}

int uart_putc(int port, char c) {
    switch (port) {
        case 0:
            while (R8_UART0_TFC == UART_FIFO_SIZE);
            R8_UART0_THR = c;
            break;
        case 1:
            while (R8_UART1_TFC == UART_FIFO_SIZE);
            R8_UART1_THR = c;
            break;
        case 2:
            while (R8_UART2_TFC == UART_FIFO_SIZE);
            R8_UART2_THR = c;
            break;
        case 3:
            while (R8_UART3_TFC == UART_FIFO_SIZE);
            R8_UART3_THR = c;
            break;
        default:
            return -1;
    }
	return 1;
}

int uart_getc(int port, bool wait) {
    switch (port) {
        case 0:
            if (!R8_UART0_RFC) {
                return -1;
            } else {
                return R8_UART0_RBR;
            }
        case 1:
            if (!R8_UART1_RFC) {
                return -1;
            } else {
                return R8_UART1_RBR;
            }
        case 2:
            if (!R8_UART2_RFC) {
                return -1;
            } else {
                return R8_UART2_RBR;
            }
        case 3:
            if (!R8_UART3_RFC) {
                return -1;
            } else {
                return R8_UART3_RBR;
            }
        default:
            return -1;
    }
}

void uart_flush_tx(int port) {
}

void uart_flush_rx(int port) {
}

void uart_init_port(int port, uint baud) {}
