#pragma once

#define GPIO_OUTPUT_PP_5 ((1 << 16) << 1)
#define GPIO_OUTPUT_PP_20 ((1 << 16) << 2)

//PA[0]-PA[15] -> GPIO 1-16, PB[0]-PB[23] -> GPIO 17 - 40
#define GPIO(port, pin) (16 * port + pin + 1)

#define GPIO_PORT_A 0
#define GPIO_PORT_B 1
