#pragma once

#define CODE_FLASH_BASE 0
#define CODE_FLASH_SIZE 0x3E800
#define CODE_FLASH_NAME "codeflash"
#define DATA_FLASH_BASE 0x3E800
#define DATA_FLASH_SIZE 0x800
#define DATA_FLASH_NAME "dataflash"
#define INFO_FLASH_BASE 0x40000
#define INFO_FLASH_SIZE 0x400
#define INFO_FLASH_NAME "infoflash"

void ch579_chipflash_init(void);
int ch579_infoflash_read_macaddr(uint8_t macaddr[]);