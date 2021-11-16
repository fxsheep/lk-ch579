#pragma once

#define CODE_FLASH_BASE 0
#define CODE_FLASH_SIZE 0x3E800
#define DATA_FLASH_BASE 0x3E800
#define DATA_FLASH_SIZE 0x800
#define INFO_FLASH_BASE 0x40000
#define INFO_FLASH_SIZE 0x400

void ch579_chipflash_init(void);