/**
 * Samsung debug inform.
 *
 * @author kumhyun.cho@samsung.com
 * @since 2012.05.29
 */
#ifndef SEC_DEBUG_INFORM
#define SEC_DEBUG_INFORM

/**
 * Default address for SEC DEBUG information.
 *
 * If CONFIG_SEC_DEBUG_INFORM_IOTABLE was enabled, virtual address will be used.
 * If CONFIG_SEC_DEBUG_INFORM_IOREMAP was enabled, virtual address will be not used.
 */
#if defined(CONFIG_ARCH_SHMOBILE)
#define SEC_DEBUG_INFORM_PHYS 0x477f6000
#define SEC_DEBUG_INFORM_VIRT 0xf7000000
#endif

/**
 * +0x00000000: magic
 * +0x00000004: inform 0 (offset)
 */
#define SEC_DEBUG_INFORM_OFFSET 0x00000004

struct sec_debug_inform {
	unsigned long phys;
	void __iomem* virt;
	unsigned int size;
};

void* sec_debug_inform_init(void);

void sec_debug_inform_write(unsigned magic, unsigned index);

void sec_debug_inform_magic_write(unsigned magic);

#endif
