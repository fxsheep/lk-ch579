#include <lk/debug.h>
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

    dprintf(SPEW, "Reboot reason: ");
    switch(R8_RESET_STATUS & RB_RESET_FLAG) {
        case RST_FLAG_SW:
            dprintf(SPEW, "software reset\n");
            break;
        case RST_FLAG_RPOR:
            dprintf(SPEW, "real power on reset\n");
            break;
        case RST_FLAG_WTR:
            dprintf(SPEW, "watchdog reset\n");
            break;
        case RST_FLAG_MR:
            dprintf(SPEW, "manual reset\n");
            break;
        case RST_FLAG_GPWSM:
            dprintf(SPEW, "wakeup from shutdown\n");
            break;
        case 0x4:
            dprintf(SPEW, "wakeup from standby, last time software reset\n");
            break;
        case 0x6:
            dprintf(SPEW, "wakeup from standby, last time watchdog reset\n");
            break;
        case 0x7:
            dprintf(SPEW, "wakeup from standby, last time manual reset\n");
            break;
    }
}

void platform_init(void) {
    ch579_chipflash_init();
    ch579_eth_init();
}
