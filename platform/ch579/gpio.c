#include <assert.h>
#include <lk/debug.h>
#include <dev/gpio.h>
#include <platform/gpio.h>
#include <CH57x_common.h>

int gpio_config(unsigned nr, unsigned flags) {
    if (flags & GPIO_INPUT) {
        if (nr <= 16) {
            if (flags & GPIO_PULLUP) {
                R32_PA_PD_DRV &= ~(1 << nr - 1);
                R32_PA_PU |= (1 << nr - 1);
            } else if (flags & GPIO_PULLDOWN) {
                R32_PA_PD_DRV |= (1 << nr - 1);
                R32_PA_PU &= ~(1 << nr - 1);
            } else { //FLOATING
                R32_PA_PD_DRV &= ~(1 << nr - 1);
                R32_PA_PU &= ~(1 << nr - 1);
            }

            if (flags & GPIO_LEVEL) {
                if (flags & GPIO_HIGH){
                    R16_PA_INT_MODE &= ~(1 << nr - 1);
                    R32_PA_OUT |= (1 << nr - 1);
                } else { //GPIO_LOW
                    R16_PA_INT_MODE &= ~(1 << nr - 1);
                    R32_PA_CLR |= (1 << nr - 1);
                }
                R16_PA_INT_EN |= (1 << nr - 1);
            } else if (flags & GPIO_EDGE) {
                if (flags & GPIO_RISING){
                    R16_PA_INT_MODE |= (1 << nr - 1);
                    R32_PA_OUT |= (1 << nr - 1);
                } else { //GPIO_FALLING
                    R16_PA_INT_MODE |= (1 << nr - 1);
                    R32_PA_CLR |= (1 << nr - 1);
                }
                R16_PA_INT_EN |= (1 << nr - 1);
            } else {
                R16_PA_INT_EN &= ~(1 << nr - 1);
            }

            R16_PA_INT_IF |= (1 << nr - 1);
            R32_PA_DIR &= ~(1 << nr - 1);
        } else {
            nr -= 16;
            if (flags & GPIO_PULLUP) {
                R32_PB_PD_DRV &= ~(1 << nr - 1);
                R32_PB_PU |= (1 << nr - 1);
            } else if (flags & GPIO_PULLDOWN) {
                R32_PB_PD_DRV |= (1 << nr - 1);
                R32_PB_PU &= ~(1 << nr - 1);
            } else { //FLOATING
                R32_PB_PD_DRV &= ~(1 << nr - 1);
                R32_PB_PU &= ~(1 << nr - 1);
            }

            if (flags & GPIO_LEVEL) {
                if (flags & GPIO_HIGH){
                    R16_PB_INT_MODE &= ~(1 << nr - 1);
                    R32_PB_OUT |= (1 << nr - 1);
                } else { //GPIO_LOW
                    R16_PB_INT_MODE &= ~(1 << nr - 1);
                    R32_PB_CLR |= (1 << nr - 1);
                }
                R16_PB_INT_EN |= (1 << nr - 1);
            } else if (flags & GPIO_EDGE) {
                if (flags & GPIO_RISING){
                    R16_PB_INT_MODE |= (1 << nr - 1);
                    R32_PB_OUT |= (1 << nr - 1);
                } else { //GPIO_FALLING
                    R16_PB_INT_MODE |= (1 << nr - 1);
                    R32_PB_CLR |= (1 << nr - 1);
                }
                R16_PB_INT_EN |= (1 << nr - 1);
            } else {
                R16_PB_INT_EN &= ~(1 << nr - 1);
            }

            R16_PB_INT_IF |= (1 << nr - 1);
            R32_PB_DIR &= ~(1 << nr - 1);
        }
    } else if (flags & GPIO_OUTPUT) {
        if (nr <= 16) {
            if (flags & GPIO_OUTPUT_PP_20)
                R32_PA_PD_DRV |= (1 << nr - 1);
            else
                R32_PA_PD_DRV &= ~(1 << nr - 1);
            R32_PA_DIR |= (1 << nr - 1);
        } else {
            nr -= 16;
            if (flags & GPIO_OUTPUT_PP_20)
                R32_PB_PD_DRV |= (1 << nr - 1);
            else
                R32_PB_PD_DRV &= ~(1 << nr - 1);
            R32_PB_DIR |= (1 << nr - 1);
        }
    }
    return 0;
}

void gpio_set(unsigned nr, unsigned on) {
    if (on) {
        if (nr <= 16) {
            R32_PA_OUT |= (1 << nr - 1);
        } else {
            nr -= 16;
            R32_PB_OUT |= (1 << nr - 1);
        }
    } else {
        if (nr <= 16) {
            R32_PA_CLR |= (1 << nr - 1);
        } else {
            nr -= 16;
            R32_PB_CLR |= (1 << nr - 1);
        }
    }
}

int gpio_get(unsigned nr) {
    if (nr <= 16) {
        return (R32_PA_PIN & (1 << nr - 1)) >> (nr - 1);
    } else {
        nr -= 16;
        return (R32_PB_PIN & (1 << nr - 1)) >> (nr - 1);
    }
}

void gpio_pin_remap(unsigned on, unsigned periph) {
    if (on)
        R16_PIN_ALTERNATE |= periph;
    else
        R16_PIN_ALTERNATE &= ~periph;
}

void gpio_analog_only(unsigned on, unsigned periph) {
    if (on)
        R16_PIN_ANALOG_IE |= periph;
    else
        R16_PIN_ANALOG_IE &= ~periph;
}