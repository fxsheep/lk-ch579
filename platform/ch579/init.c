#include <platform.h>
#include <arch/arm/cm.h>
#include <target/debugconfig.h>
#include <CH57x_common.h>

extern void* vectab;

void platform_early_init(void) {
    SystemInit();
    // start the systick timer
    arm_cm_systick_init(32000000);

    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
}

void platform_init(void) {
}
