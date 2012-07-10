#ifndef __ARCH_ARMLINUX_H
#define __ARCH_ARMLINUX_H

#if defined CONFIG_CMD_BOOTM || defined CONFIG_CMD_BOOTZ || \
	defined CONFIG_CMD_BOOTU
void armlinux_set_bootparams(void *params);
void armlinux_set_architecture(int architecture);
void armlinux_add_dram(struct device_d *dev);
void armlinux_set_revision(unsigned int);
void armlinux_set_serial(u64);
#else
static inline void armlinux_set_bootparams(void *params)
{
}

static inline void armlinux_set_architecture(int architecture)
{
}

static inline void armlinux_add_dram(struct device_d *dev)
{
}

static inline void armlinux_set_revision(unsigned int rev)
{
}

static inline void armlinux_set_serial(u64 serial)
{
}
#endif

struct image_data;

void start_linux(void *adr, int swap, struct image_data *data);

#define BOOT_MAGIC "ANDROID!"
#define BOOT_MAGIC_SIZE 8
#define BOOT_NAME_SIZE 16
#define BOOT_ARGS_SIZE 512

struct boot_img_hdr
{
    unsigned char magic[BOOT_MAGIC_SIZE];

    unsigned kernel_size;  /* size in bytes */
    unsigned kernel_addr;  /* physical load addr */

    unsigned ramdisk_size; /* size in bytes */
    unsigned ramdisk_addr; /* physical load addr */

    unsigned second_size;  /* size in bytes */
    unsigned second_addr;  /* physical load addr */

    unsigned tags_addr;    /* physical addr for kernel tags */
    unsigned page_size;    /* flash page size we assume */
    unsigned unused[2];    /* future expansion: should be 0 */

    unsigned char name[BOOT_NAME_SIZE]; /* asciiz product name */

    unsigned char cmdline[BOOT_ARGS_SIZE];

    unsigned id[8]; /* timestamp / checksum / sha1 / etc */
};

int do_booti_linux (struct boot_img_hdr *hdr);
#endif /* __ARCH_ARMLINUX_H */
