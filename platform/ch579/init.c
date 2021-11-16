#include <platform.h>
#include <arch/arm/cm.h>
#include <platform/chipflash.h>
#include <platform/eth.h>
#include <dev/gpio.h>
#include <platform/gpio.h>
#include <target/debugconfig.h>
#include <CH57x_common.h>

extern void* vectab;

void platform_early_init(void) {
    /* SYS_PLL must be enabled to get ETH working */
    PWR_UnitModCfg(ENABLE, UNIT_SYS_PLL);
    DelayMs(3); 
    SetSysClock(CLK_SOURCE_HSE_32MHz);

    // start the systick timer
    arm_cm_systick_init(32000000);

    gpio_set(GPIO(GPIO_PORT_A, 9), 1);
    gpio_config(GPIO(GPIO_PORT_A, 8), GPIO_INPUT | GPIO_PULLUP);
    gpio_config(GPIO(GPIO_PORT_A, 9), GPIO_OUTPUT);
    UART1_DefInit();
}

void platform_init(void) {
    ch579_chipflash_init();
    ch579_eth_init();
}
