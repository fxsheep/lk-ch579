#include <target.h>
#include <target/debugconfig.h>
#include <lib/sysparam.h>
#include <platform/gpio.h>
#include <platform/chipflash.h>

#define PARAM_BDEV_NAME DATA_FLASH_NAME
#define PARAM_OFFSET 0
#define PARAM_SIZE DATA_FLASH_SIZE

void target_early_init(void) {
}

void target_set_debug_led(unsigned int led, bool on) {
}

void target_init(void) {
    bdev_t *flash = bio_open(PARAM_BDEV_NAME);
    sysparam_scan(flash, PARAM_OFFSET, PARAM_SIZE);
}

