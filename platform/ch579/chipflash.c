#include <lk/err.h>
#include <lk/debug.h>
#include <lk/trace.h>
#include <platform.h>
#include <platform/chipflash.h>
#include <lib/bio.h>
#include <lk/reg.h>
#include <CH57x_common.h>

static bio_erase_geometry_info_t cf_geometry[1] = {0, CODE_FLASH_SIZE, 512, 9};
static bio_erase_geometry_info_t df_geometry[1] = {0, DATA_FLASH_SIZE, 512, 9};
//static bio_erase_geometry_info_t if_geometry[1] = {0, INFO_FLASH_SIZE, 512, 9};

static ssize_t ch579_codeflash_bdev_read(struct bdev *bdev, void *buf, off_t offset, size_t len) {
    len = bio_trim_range(bdev, offset, len);
    if (len == 0)
        return 0;
    memcpy(buf, offset + CODE_FLASH_BASE, len);
    return len;
}

static ssize_t ch579_dataflash_bdev_read(struct bdev *bdev, void *buf, off_t offset, size_t len) {
    len = bio_trim_range(bdev, offset, len);
    if (len == 0)
        return 0;
    memcpy(buf, offset + DATA_FLASH_BASE, len);
    return len;
}

static ssize_t ch579_infoflash_bdev_read(struct bdev *bdev, void *buf, off_t offset, size_t len) {
    len = bio_trim_range(bdev, offset, len);
    if (len == 0)
        return 0;
    memcpy(buf, offset + INFO_FLASH_BASE, len);
    return len;
}

static ssize_t ch579_codeflash_bdev_read_block(struct bdev *bdev, void *buf, bnum_t block, uint count) {
    count = bio_trim_block_range(bdev, block, count);
    if (count == 0)
        return 0;
    memcpy(buf, block * bdev->block_size + CODE_FLASH_BASE, count * bdev->block_size);
    return count * bdev->block_size;
}

static ssize_t ch579_dataflash_bdev_read_block(struct bdev *bdev, void *buf, bnum_t block, uint count) {
    count = bio_trim_block_range(bdev, block, count);
    if (count == 0)
        return 0;
    memcpy(buf, block * bdev->block_size + DATA_FLASH_BASE, count * bdev->block_size);
    return count * bdev->block_size;
}

static ssize_t ch579_infoflash_bdev_read_block(struct bdev *bdev, void *buf, bnum_t block, uint count) {
    count = bio_trim_block_range(bdev, block, count);
    if (count == 0)
        return 0;
    memcpy(buf, block * bdev->block_size + INFO_FLASH_BASE, count * bdev->block_size);
    return count * bdev->block_size;
}

static int ch579_codeflash_bdev_ioctl(struct bdev *bdev, int request, void *argp) {
    int ret = ERR_NOT_SUPPORTED;
    switch (request) {
        case BIO_IOCTL_GET_MAP_ADDR:
        case BIO_IOCTL_GET_MEM_MAP:
            if (argp)
                *(void **)argp = (void *)CODE_FLASH_BASE;
            break;
        case BIO_IOCTL_PUT_MEM_MAP:
            break;
    }
    return ret;
}

static int ch579_dataflash_bdev_ioctl(struct bdev *bdev, int request, void *argp) {
    int ret = ERR_NOT_SUPPORTED;
    switch (request) {
        case BIO_IOCTL_GET_MAP_ADDR:
        case BIO_IOCTL_GET_MEM_MAP:
            if (argp)
                *(void **)argp = (void *)DATA_FLASH_BASE;
            break;
        case BIO_IOCTL_PUT_MEM_MAP:
            break;
    }
    return ret;
}

static int ch579_infoflash_bdev_ioctl(struct bdev *bdev, int request, void *argp) {
    int ret = ERR_NOT_SUPPORTED;
    switch (request) {
        case BIO_IOCTL_GET_MAP_ADDR:
        case BIO_IOCTL_GET_MEM_MAP:
            if (argp)
                *(void **)argp = (void *)INFO_FLASH_BASE;
            break;
        case BIO_IOCTL_PUT_MEM_MAP:
            break;
    }
    return ret;
}

static ssize_t ch579_codeflash_bdev_write_block(struct bdev *bdev, const void *buf, bnum_t block, uint count) {
    ssize_t written_bytes;
    const uint32_t *buf32 = (const uint32_t *)buf;

    count = bio_trim_block_range(bdev, block, count);
    written_bytes = count * bdev->block_size;
    if (count == 0)
        return 0;
    if (FlashWriteBuf(block * bdev->block_size + CODE_FLASH_BASE, buf32, written_bytes)) {
        written_bytes = ERR_IO;
    }
    return written_bytes;
}

static ssize_t ch579_dataflash_bdev_write_block(struct bdev *bdev, const void *buf, bnum_t block, uint count) {
    ssize_t written_bytes;
    const uint32_t *buf32 = (const uint32_t *)buf;

    count = bio_trim_block_range(bdev, block, count);
    written_bytes = count * bdev->block_size;
    if (count == 0)
        return 0;
    if (FlashWriteBuf(block * bdev->block_size + DATA_FLASH_BASE, buf32, written_bytes)) {
        written_bytes = ERR_IO;
    }
    return written_bytes;
}

static ssize_t ch579_codeflash_bdev_erase(struct bdev *bdev, off_t offset, size_t len) {
    ssize_t erased_bytes = 0;

    len = bio_trim_range(bdev, offset, len);
    if (len == 0)
        return 0;
    while (len >= bdev->block_size){
        if (FlashBlockErase(offset + CODE_FLASH_BASE)) {
            erased_bytes = ERR_IO;
            break;
        }
        offset += bdev->block_size;
        len -= bdev->block_size;
        erased_bytes += bdev->block_size;
    }
    return erased_bytes;
}

static ssize_t ch579_dataflash_bdev_erase(struct bdev *bdev, off_t offset, size_t len) {
    ssize_t erased_bytes = 0;

    len = bio_trim_range(bdev, offset, len);
    if (len == 0)
        return 0;
    while (len >= bdev->block_size){
        if (FlashBlockErase(offset + DATA_FLASH_BASE)) {
            erased_bytes = ERR_IO;
            break;
        }
        offset += bdev->block_size;
        len -= bdev->block_size;
        erased_bytes += bdev->block_size;
    }
    return erased_bytes;
}

/* MAC address in big endian, read from infoflash*/
int ch579_infoflash_read_macaddr(uint8_t macaddr[]) {
    int i;

    for (i = 5; i >= 0; i--) {
        macaddr[5 - i] = *(uint8_t *)(ROM_MAC_ADDR + i);
    }
    if (macaddr[0] != 0x84 || macaddr[1] != 0xC2 || macaddr[2] != 0xE4) {
        /* Not a valid MAC address of WCH */
        return -1;
    }
    return 0;
}

void ch579_chipflash_init(void) {
    static bdev_t cf_dev, df_dev, if_dev;

    bio_initialize_bdev(&cf_dev, "codeflash", 512, CODE_FLASH_SIZE / 512, 1, cf_geometry,
                        BIO_FLAGS_NONE);

    cf_dev.erase_byte = 0xff;
    cf_dev.read = &ch579_codeflash_bdev_read;
    cf_dev.read_block = &ch579_codeflash_bdev_read_block;
    cf_dev.write_block = &ch579_codeflash_bdev_write_block;
    cf_dev.erase = &ch579_codeflash_bdev_erase;
    cf_dev.ioctl = &ch579_codeflash_bdev_ioctl;

    bio_register_device(&cf_dev);

    bio_initialize_bdev(&df_dev, "dataflash", 512, DATA_FLASH_SIZE / 512, 1, df_geometry,
                        BIO_FLAGS_NONE);

    df_dev.erase_byte = 0xff;
    df_dev.read = &ch579_dataflash_bdev_read;
    df_dev.read_block = &ch579_dataflash_bdev_read_block;
    df_dev.write_block = &ch579_dataflash_bdev_write_block;
    df_dev.erase = &ch579_dataflash_bdev_erase;
    df_dev.ioctl = &ch579_dataflash_bdev_ioctl;

    bio_register_device(&df_dev);

    bio_initialize_bdev(&if_dev, "infoflash", 512, INFO_FLASH_SIZE / 512, 0, NULL,
                        BIO_FLAGS_NONE);

//    if_dev.erase_byte = 0xff;
    if_dev.read = &ch579_infoflash_bdev_read;
    if_dev.read_block = &ch579_infoflash_bdev_read_block;
//    if_dev.write_block = &ch579_infoflash_bdev_write_block;
//    if_dev.erase = &ch579_infoflash_bdev_erase;
    if_dev.ioctl = &ch579_infoflash_bdev_ioctl;

    bio_register_device(&if_dev);
}
