#include <platform/debug.h>
#include <target/debugconfig.h>
#include <stdio.h>
#include <dev/uart.h>

void platform_dputc(char c) {
    if (c == '\n')
        uart_putc(DEBUG_UART, '\r');
    uart_putc(DEBUG_UART, c);
}

int platform_dgetc(char *c, bool wait) {
    int ret = uart_getc(DEBUG_UART, wait);
    if (ret < 0)
        return -1;
    *c = ret;
    return 0;
}

