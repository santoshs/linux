#include <linux/errno.h>
#include <linux/types.h>
#include <asm/atomic.h>
#include <asm/io.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/ioctl.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/scatterlist.h>
#include <mach/common.h>
#include <mach/r8a73734.h>

static dev_t rmc_loader_dev;

struct toc_str {
   int start;
  unsigned int size;
  unsigned int spare[2];
  unsigned int load_address;
  char     filename[12];
};

static int toc_index = 0;
static struct toc_str * toc = 0;
static char * toc_buffer = 0;
static char * toc_ptr = 0;
static char * data_ptr = 0;
static char * load_ptr = 0;
static char * data_buf = 0;
static int all_toc_entries_loaded = 0;

#define MAX_TOC_ENTRY  16
#define TOC_ENTRY_SIZE (sizeof(struct toc_str))
#define BUF_SIZE       256

#define HPB_OCPBRGWIN1_MDM2MEM		IO_ADDRESS(0xE6001200)
#define HPB_OCPBRGWIN2_MDM2APE		IO_ADDRESS(0xE6001204)
#define HPB_OCPBRGWIN3_APE2MDM		IO_ADDRESS(0xE6001208)

#define C4POWCR				IO_ADDRESS(0xE618004C)

#define WPMCIF_EPMU_BASE		IO_ADDRESS(0xE6190000)
#define WPMCIF_EPMU_START_CR		(WPMCIF_EPMU_BASE + 0x0000)
#define WPMCIF_EPMU_ACC_CR		(WPMCIF_EPMU_BASE + 0x0004)
#define WPMCIF_EPMU_RES_CR		(WPMCIF_EPMU_BASE + 0x0008)
#define WPMCIF_EPMU_PLL2_REALLY5_CR	(WPMCIF_EPMU_BASE + 0x0020)
#define WPMCIF_EPMU_RFCLK_CR		(WPMCIF_EPMU_BASE + 0x0024)
#define WPMCIF_EPMU_HPSSCLK_CR		(WPMCIF_EPMU_BASE + 0x0028)

#define CPG_SMSTPCR5			IO_ADDRESS(0xE6150144)
#define CPG_SMSTPCR3			IO_ADDRESS(0xE615013C)

/* HostCPU CoreSight ETR for modem STM traces to SDRAM */
#define CPU_ETR_BASE			IO_ADDRESS(0xE6FA5000)
#define CPU_ETR_RSZ			(CPU_ETR_BASE + 0x004)
#define CPU_ETR_STS			(CPU_ETR_BASE + 0x00C)
#define CPU_ETR_RRD			(CPU_ETR_BASE + 0x010)
#define CPU_ETR_RRP			(CPU_ETR_BASE + 0x014)
#define CPU_ETR_RWP                     (CPU_ETR_BASE + 0x018)
#define CPU_ETR_TRG			(CPU_ETR_BASE + 0x01C)
#define CPU_ETR_CTL			(CPU_ETR_BASE + 0x020)
#define CPU_ETR_RWD			(CPU_ETR_BASE + 0x024)
#define CPU_ETR_MODE			(CPU_ETR_BASE + 0x028)
#define CPU_ETR_LBUFLEVEL		(CPU_ETR_BASE + 0x02C)
#define CPU_ETR_CBUFLEVEL		(CPU_ETR_BASE + 0x030)
#define CPU_ETR_BUFWM			(CPU_ETR_BASE + 0x034)
#define CPU_ETR_RRPHI			(CPU_ETR_BASE + 0x038)
#define CPU_ETR_RWPHI			(CPU_ETR_BASE + 0x03C)
#define CPU_ETR_AXICTL			(CPU_ETR_BASE + 0x110)
#define CPU_ETR_DBALO			(CPU_ETR_BASE + 0x118)
#define CPU_ETR_DBAHI			(CPU_ETR_BASE + 0x11C)
#define CPU_ETR_FFSR			(CPU_ETR_BASE + 0x300)
#define CPU_ETR_FFCR			(CPU_ETR_BASE + 0x304)
#define CPU_ETR_PSCR			(CPU_ETR_BASE + 0x308)
#define CPU_ETR_ITATBMDATA0		(CPU_ETR_BASE + 0xED0)
#define CPU_ETR_ITATBMCTR2		(CPU_ETR_BASE + 0xED4)
#define CPU_ETR_ITATBMCTR1		(CPU_ETR_BASE + 0xED8)
#define CPU_ETR_ITATBMCTR0		(CPU_ETR_BASE + 0xEDC)
#define CPU_ETR_ITMISCOP0		(CPU_ETR_BASE + 0xEE0)
#define CPU_ETR_ITTRFLIN		(CPU_ETR_BASE + 0xEE8)
#define CPU_ETR_ITATBDATA0		(CPU_ETR_BASE + 0xEEC)
#define CPU_ETR_ITATBCTR2		(CPU_ETR_BASE + 0xEF0)
#define CPU_ETR_ITATBCTR1		(CPU_ETR_BASE + 0xEF4)
#define CPU_ETR_ITATBCTR0		(CPU_ETR_BASE + 0xEF8)
#define CPU_ETR_ItCtrl			(CPU_ETR_BASE + 0xF00)
#define CPU_ETR_ClaimSet		(CPU_ETR_BASE + 0xFA0)
#define CPU_ETR_ClaimClear		(CPU_ETR_BASE + 0xFA4)
#define CPU_ETR_LockAccess		(CPU_ETR_BASE + 0xFB0)
#define CPU_ETR_LockStatus		(CPU_ETR_BASE + 0xFB4)
#define CPU_ETR_AuthStatus		(CPU_ETR_BASE + 0xFB8)
#define CPU_ETR_DevId			(CPU_ETR_BASE + 0xFC8)
#define CPU_ETR_DevType			(CPU_ETR_BASE + 0xFCC)
#define CPU_ETR_PeripheralID4		(CPU_ETR_BASE + 0xFD0)
#define CPU_ETR_PeripheralID5		(CPU_ETR_BASE + 0xFD4)
#define CPU_ETR_PeripheralID6		(CPU_ETR_BASE + 0xFD8)
#define CPU_ETR_PeripheralID7		(CPU_ETR_BASE + 0xFDC)
#define CPU_ETR_PeripheralID0		(CPU_ETR_BASE + 0xFE0)
#define CPU_ETR_PeripheralID1		(CPU_ETR_BASE + 0xFE4)
#define CPU_ETR_PeripheralID2		(CPU_ETR_BASE + 0xFE8)
#define CPU_ETR_PeripheralID3		(CPU_ETR_BASE + 0xFEC)
#define CPU_ETR_ComponentID0		(CPU_ETR_BASE + 0xFF0)
#define CPU_ETR_ComponentID1		(CPU_ETR_BASE + 0xFF4)
#define CPU_ETR_ComponentID2		(CPU_ETR_BASE + 0xFF8)
#define CPU_ETR_ComponentID3		(CPU_ETR_BASE + 0xFFC)

static uint32_t stm_sdram_base;
static uint32_t stm_sdram_read_ptr;
static uint32_t stm_sdram_write_ptr;
static uint32_t stm_sdram_size;

static void ape5r_modify_register32(unsigned long addr, unsigned long set, unsigned long clear)
{
  unsigned long data = 0;
  data = __raw_readl(addr);
  data &= ~clear;
  data |= set;
  __raw_writel(data, addr);
}


static int __devexit rmc_loader_remove(struct device *dev)
{
	printk(KERN_ALERT "rmc_loader_remove()\n");
	return 0;
}

static ssize_t rmc_loader_read(struct file *file, char __user *buf,
						size_t len, loff_t *ppos)
{
	size_t retval = 0;
	void __iomem * remapped_phys_addr=0;

	if (0 == stm_sdram_size) {
		return -ENOMEM;
	}

	len = len & ~3; /* Align length to 4-byte boundary */

	stm_sdram_write_ptr = __raw_readl(CPU_ETR_RWP);
	if (stm_sdram_read_ptr > stm_sdram_write_ptr) {
		stm_sdram_write_ptr = stm_sdram_base + stm_sdram_size;
	}
	
	if (len + stm_sdram_read_ptr >= stm_sdram_write_ptr) {
		len = stm_sdram_write_ptr - stm_sdram_read_ptr;
	}

	if (len) {
		remapped_phys_addr = ioremap(stm_sdram_read_ptr, len);
		if (unlikely(!remapped_phys_addr)) {
			retval = -EFAULT;
		} else if (unlikely(copy_to_user(/*to_user_ptr*/ buf, /*from_kernel_mem*/ remapped_phys_addr, /*length*/ len))) {
			retval = -EFAULT;
		} else {
			*ppos += len;
			retval = len;
			stm_sdram_read_ptr += len;
			if (stm_sdram_read_ptr >= stm_sdram_base + stm_sdram_size) {
				stm_sdram_read_ptr = stm_sdram_base;
			}
		}
		if (remapped_phys_addr) {
			iounmap(remapped_phys_addr);
		}
	}
	return retval;
}

static int find_next_toc_index(char *ptr)
{
	int min_diff = -1;
	int min_index = -1;
	int index;
	
	for (index=0; index<MAX_TOC_ENTRY; index++)
	{
		if (toc[index].start == -1) { // End of TOC
			break;
		}
		if (toc[index].filename[0] == (char)0xFF) { // Used already
			continue;
		}
		if (min_diff == -1) { // Initial value
			min_diff = toc[index].start - (unsigned int)ptr;
			min_index = index;
			continue;
		}
		if (toc[index].start - (unsigned int)ptr < min_diff) {
			min_diff = toc[index].start - (unsigned int)ptr;
			min_index = index;
		}
	}
	if (min_index >= 0) { // Mark THIS as used one
		toc[min_index].filename[0] = (char)0xFF;
	}
	return min_index;
}

static ssize_t rmc_loader_write(struct file *file, const char __user *buf,
						size_t len, loff_t *ppos)
{
	int i=0;
	uint32_t align=0;
	uint32_t al_data=0;
	void __iomem * remapped_phys_addr=0;
	void __iomem * remapped_phys_addr_mem=0;
	
//	printk(KERN_ALERT "rmc_loader_write(len=%d)\n", (int)len);
	
	if (!data_ptr) {
	        if (len > TOC_ENTRY_SIZE) {
			len = TOC_ENTRY_SIZE;
		}
		if ((toc_ptr - toc_buffer + len) >= TOC_ENTRY_SIZE) {
			len = TOC_ENTRY_SIZE - (toc_ptr - toc_buffer);
		}
		if (copy_from_user(toc_ptr, buf, len)) {
			goto fault_reset;
		} else {
			toc_ptr += len;
			*ppos += len;
			if ((toc_ptr - toc_buffer) >= TOC_ENTRY_SIZE) {
				toc[toc_index].start        = *( int *)(&toc_buffer[0*4]);
				toc[toc_index].size         = *(unsigned int *)(&toc_buffer[1*4]);
				toc[toc_index].spare[0]     = *(unsigned int *)(&toc_buffer[2*4]);
				toc[toc_index].spare[1]     = *(unsigned int *)(&toc_buffer[3*4]);
				toc[toc_index].load_address = *(unsigned int *)(&toc_buffer[4*4]);
				memcpy(toc[toc_index].filename, (char *)(&toc_buffer[5*4]), 12);
				toc[toc_index].filename[11] = '\0';
				if (toc[toc_index].start == -1) {
					toc_index++;
					data_ptr = data_buf;
					printk(KERN_ALERT "End of TOC reached.");
					toc_ptr = (char *)(TOC_ENTRY_SIZE*toc_index); // This on relative
					toc_index = find_next_toc_index(toc_ptr);
					load_ptr = (char *)toc[toc_index].load_address;
					printk(KERN_ALERT "Next nearest TOC index is %d", toc_index);
				} else {
					printk(KERN_ALERT "Read TOC[%d].start=0x%x, size=0x%x, load_address=0x%x, filename=%s",
						toc_index, toc[toc_index].start, toc[toc_index].size,
						toc[toc_index].load_address, toc[toc_index].filename);
					toc_index++;
					toc_ptr = toc_buffer;
				}
			}
			return len;
		}
	} else {
		if (toc_index == -1) {
			all_toc_entries_loaded = 1;
			len = 0;
			return len;
		}
		if (len > BUF_SIZE) {
			len = BUF_SIZE;
		}
		if ((unsigned int)toc_ptr < toc[toc_index].start) {
			if ((unsigned int)toc_ptr + len > toc[toc_index].start) {
				len = toc[toc_index].start - (unsigned int)toc_ptr;
				*ppos += len;
				toc_ptr += len;
//				printk(KERN_ALERT "Skipped (a) %d padding bytes.", len);
				return len;
			} else {
				*ppos += len;
				toc_ptr += len;
//				printk(KERN_ALERT "Skipped (b) %d padding bytes.", len);
				return len;
			}
		}
//		printk(KERN_ALERT "Now reading TOC[%d] data content", toc_index);
		if ((unsigned int)load_ptr - toc[toc_index].load_address + len > toc[toc_index].size) {
			len = toc[toc_index].size - (unsigned int)load_ptr + toc[toc_index].load_address;
		}
		if (copy_from_user(data_ptr, buf, len)) {
			goto fault_reset;
		} else {
#if 1 /* Really physically write */
//			printk(KERN_ALERT "WRITING data at phys addr 0x%x for %d bytes: 0x%x, 0x%x, 0x%x, 0x%x, ...",
//			        (uint32_t)load_ptr, len, data_ptr[0], data_ptr[1], data_ptr[2], data_ptr[3]);
			remapped_phys_addr = ioremap((uint32_t)load_ptr, len);   /* Assume DEVICE access, not cached */
                        remapped_phys_addr_mem = remapped_phys_addr;
			if (!remapped_phys_addr) {
				goto fault_reset;
			}
			align = ((uint32_t)load_ptr) & 3;
			i=0;
			if (!align) {
				al_data = (__raw_readl((void __iomem *)((uint32_t)remapped_phys_addr & ~3)));
				al_data <<= ((4-align)*8); /* Assume little endian, 4-byte alignment */

				for (; i<len && i<(4-align); i++) {
					al_data = (al_data >> 8) | (data_ptr[i]<<(3*8));
				}
				al_data >>= 8*(i - (4-align)); /* Shift empty bytes in if len is too small */

				__raw_writel(al_data, ((void __iomem *)((uint32_t)remapped_phys_addr & ~3)));
				remapped_phys_addr += (4-align);
			}
	                for(; i < (len&~3); i += 4) {
				/* Write 4-byte aligned data, potentially writes 1-3 extra bytes but never mind */
				al_data = (data_ptr[0+i] << (0*8)) | 
				          (data_ptr[1+i] << (1*8)) |
				          (data_ptr[2+i] << (2*8)) |
				          (data_ptr[3+i] << (3*8));
					  
	                        __raw_writel(al_data, remapped_phys_addr);
				remapped_phys_addr += 4;
			}
			switch (len - i) {
				case 0:
					break;
				case 1: 
					al_data = data_ptr[0+i] << (0*8);
					__raw_writel(al_data, remapped_phys_addr);
					break;
				case 2:
					al_data = (data_ptr[0+i] << (0*8)) | 
					          (data_ptr[1+i] << (1*8));
					__raw_writel(al_data, remapped_phys_addr);
					break;
				case 3:
					al_data = (data_ptr[0+i] << (0*8)) | 
					          (data_ptr[1+i] << (1*8)) |
				        	  (data_ptr[2+i] << (2*8));
					__raw_writel(al_data, remapped_phys_addr);
					break;
				default:
					break;
			}
			iounmap(remapped_phys_addr_mem);
//			printk(KERN_ALERT "WRITING to remapped_phys_addr done, iounmapped");
#else /* Just print message, skip real write */
			printk(KERN_ALERT "NOT WRITING data at phys addr 0x%x for %d bytes: 0x%x, 0x%x, 0x%x, 0x%x, ...",
			        (uint32_t)load_ptr, len, data_ptr[0], data_ptr[1], data_ptr[2], data_ptr[3]);
#endif
			load_ptr += len;
			toc_ptr += len;
			*ppos += len;
			if ((unsigned int)load_ptr - toc[toc_index].load_address == toc[toc_index].size) {
				data_ptr = data_buf;
				toc_index = find_next_toc_index(toc_ptr);
				printk(KERN_ALERT "Next nearest TOC index is %d", toc_index);
                                if (toc_index == -1) {
                                  all_toc_entries_loaded = 1;
                                }
				load_ptr = (char *)toc[toc_index].load_address;
			}
			return len;
		}
	}
	
  fault_reset:
	printk(KERN_ALERT "modem loader fault_reset");
  	toc_ptr = toc_buffer;
	data_ptr = 0;
	toc_index = 0;
	load_ptr = 0;
  	return -EFAULT;
}

static int rmc_loader_open(struct inode *inode, struct file *file)
{
	int ret = 0;
	
	if ((file->f_flags & O_ACCMODE) == O_WRONLY) {

		/* Opened for WRITING, i.e. for Loading modem firware image */

		printk(KERN_ALERT "rmc_loader_open(), O_WRONLY Initialize modem to receive boot data >>\n");
		ape5r_modify_register32(C4POWCR, 0, (1<<7)); /* read-modify clear C4POWCR.MDMSEL to allow modem requests */
		printk(KERN_ALERT "T001 HPSSCLK\n");
	    	__raw_writel(0x03000000, WPMCIF_EPMU_HPSSCLK_CR); /* PLL lock count for PLL5 */
		printk(KERN_ALERT "T002 PLL5\n");
		__raw_writel(0x32000100, WPMCIF_EPMU_PLL2_REALLY5_CR); /* PLL5 (PLL2 in EPMU doc) control register */
                                                       /*     bits [31:25] is MULFAC[6:0]           */
                                                       /*     bits [14:8]  is DIVFAC[6:0]           */
                                                       /*     HFCLKC = (MULFAC+1)*?38.4?/(DIVFAC+1) MHz */
                                                       /* Now MULFAC=25 and DIVFAC=1, i.e. HFCLK=38.4*26/2 MHz=499.2MHz */
		printk(KERN_ALERT "T003 RFCLK\n");
		__raw_writel(0x30000000, WPMCIF_EPMU_RFCLK_CR); /* RF Clock Control Register (RFCLK_CR)        */
                                                       /*     bits [31:26] is RFCLK_DIV[5:0]        */
                                                       /*     RFCLK = HFCLK/(RFCLK_DIV+1)               */
                                                       /* Now RFCLK_DIV=HFCLK/13 = 38.4 MHz          */
                                                       /* RFCLKC divider for 38.4 MHz */
		printk(KERN_ALERT "T004 OCPBRGWIN1\n");
		__raw_writel(0x40000000,HPB_OCPBRGWIN1_MDM2MEM); /* Configure upper 8 bits of OCP bus for modem */
	                                               /* This should be deferred until we know where to laod modem! */
		printk(KERN_ALERT "T005 OCPBRGWIN3\n");
		__raw_writel(0x06000000, HPB_OCPBRGWIN3_APE2MDM); /* OCP Bridge Window Reg3: Configure upper 7 bits of modem peripheral OCP address for APE to access */
                                                            /* FOR APE, range 0xE2000000--0xE3FFFFFF maps to THIS+(0x00000000--0x01FFFFFF). */
                                                            /* So to e.g. for APE to access Modem SCU_AD base address would be */
                                                            /*   0xE3D40000 ==> OCP Bus address 0x07D40000, and */
                                                            /* SCU_AD.CCR.OUTPUT would be mapped like this: */
                                                            /*   0xE3D40410 ==> 0x07D0410            */
							    
		/* TODO: Should we configure HPB_OCPBRGWIN3_MDM2APE to something away from reset value? */
		/* That would allow modem to access some APE peripherals, maybe Modem can configure that register by itself later... */

		printk(KERN_ALERT "T006 START\n");
		__raw_writel(0x00000001, WPMCIF_EPMU_START_CR); /* WGEM3.1 Start bit, 1: Start modem operations */
							/* Reading != 0 confirms Modem Power on sequence is completed */
							/* NOTE: This is NOT reset to modem! */
							/* Currently, we assume that power on reset has reset the modem already. */
		printk(KERN_ALERT "T007 busy-poll START\n");
		while (0x00000000 != __raw_readl(WPMCIF_EPMU_START_CR) ) {
			/* Wait until WGM is up, should be very quick. */
		}
		printk(KERN_ALERT "T008 ACC\n");
		__raw_writel(0x00000002, WPMCIF_EPMU_ACC_CR);  /* Host Access request */
							/* NOTE: When ACCREQ is set, modem cannot enter into deep sleep. */
							/* Once host has completed intended operations, it should clear this bit. */
		(void) __raw_readl(WPMCIF_EPMU_ACC_CR);		/* Dummy read, make sure write went through to the device... */
		printk(KERN_ALERT "T009 busy-poll ACC\n");

		while (0x00000003 != __raw_readl(WPMCIF_EPMU_ACC_CR) ) {
			/* Wait until Access OK, should be very quick. */
		}
		printk(KERN_ALERT "T010\n");

		ape5r_modify_register32(CPG_SMSTPCR5, 0, (1<<25) | (0x0F << 16)); /* Module stop control register 5 (SMSTPCR5) */
                                                              /* Supply clocks to OCP2SuperHiWay and OCP2Memory and SuperHiWay2OCP0/1 instances */
                                                              /* bit25: MSTP525=0, IICB0 operates */
                                                              /* bit19: MSTP519=0, O2S operates */
                                                              /* bit18: MSTP519=0, O2M operates */
                                                              /* bit17: MSTP517=0, S2O0 operates */
                                                              /* bit16: MSTP516=0, S2O1 operates */


		// IF I WANT TO RESET MODEM, THIS WILL DO IT:
		// EPMU RES_CR, write 1. TO CHECK: how to know when modem is finished with resetting so that I can access TCM memories?	
		printk(KERN_ALERT "rmc_loader_open(), Initialize modem to receive boot data <<\n");

	} else {

		/* Opened for READING, i.e. for dumping STM trace from SDRAM buffer written by HostCPU CoreSight ETR */

		printk(KERN_ALERT "rmc_loader_open(), O_RDONLY Dump STM >><< size=0x%x, base=0x%x, wptr=0x%x, rptr=0x%x\n", stm_sdram_size, stm_sdram_base, stm_sdram_write_ptr,
		stm_sdram_read_ptr);

	}
	return ret;
}

static int rmc_loader_flush(struct file *file, fl_owner_t id)
{
        void __iomem * remapped_mdm_io=0;
	int retval=0;

	if ((file->f_flags & O_ACCMODE) == O_WRONLY) {

		/* Opened for WRITING, i.e. for Loading modem firware image */

		if (all_toc_entries_loaded) {
			printk(KERN_ALERT "rmc_loader_flush(), Release Modem L2 CPU from Prefetch Hold >>\n");
	                remapped_mdm_io = ioremap(0xE3D40410,4); 
	                __raw_writel(2, ((void __iomem *)((uint32_t)remapped_mdm_io)));
                                               /* bit 5: 0=Internal TCM boot, 1=External ROM boot (VINTHI) */
                                               /* bit 4: 0=HF clock, 1=RF Clock forced to be used */
                                               /* bit 3: 0=PSS Clk Req is NOT masked, 1=PSS Clkc Req is MASKED */
                                               /* bit 2: 0=EModem MA_Int is selected, 1=EModem MA_Int is NOT selected */
                                               /* bit 1: 0=Cortex R4 L23 in pre-fetch hold, 1=run */
                                               /* bit 0: 0=Cortex R4 L1  in pre-fetch hold, 1=run */ 
	                iounmap(remapped_mdm_io);
		  	toc_ptr = toc_buffer;
			data_ptr = 0;
			toc_index = 0;
			load_ptr = 0;
			all_toc_entries_loaded = 0;
			printk(KERN_ALERT "rmc_loader_flush(), Release Modem L2 CPU from Prefetch Hold <<\n");
		} else {
			printk(KERN_ALERT "rmc_loader_flush(), Not all TOC entries loaded, yet >><<.\n");
			/* retval = -EFAULT; */
		}
	} else {

		/* Opened for READING, i.e. for dumping STM trace from SDRAM buffer written by HostCPU CoreSight ETR */
		printk(KERN_ALERT "rmc_loader_flush(), O_RDONLY Dump STM >><<\n");
	}

	return retval;
}


static int rmc_loader_release(struct inode *inode, struct file *file)
{
	int retval=0;

	return retval;
}

static const struct file_operations rmc_loader_fops = {
	.owner		= THIS_MODULE,
	.read		= rmc_loader_read,
	.write		= rmc_loader_write,
	.open		= rmc_loader_open,
	.flush		= rmc_loader_flush,
	.release	= rmc_loader_release,
};

static struct cdev rmc_loader_cdev;

static int __init rmc_loader_init(void)
{
	char devname[] = "rmc_mdm_loader";
	int ret = -ENOMEM;

        printk(KERN_ALERT "rmc_loader_init()\n");
	
	toc_buffer = kmalloc(TOC_ENTRY_SIZE*2, GFP_KERNEL);
	toc = kmalloc(TOC_ENTRY_SIZE*MAX_TOC_ENTRY*2, GFP_KERNEL);
	data_buf = kmalloc(BUF_SIZE*2, GFP_KERNEL);
	if (!toc_buffer || !toc) {
		printk(KERN_ALERT "rmc_loader_init() kmalloc failed!\n");
		return ret;	
	}
	toc_ptr = toc_buffer;
	
	ret = alloc_chrdev_region(&rmc_loader_dev, 0, 1, devname);
	if (ret < 0) {
		printk(KERN_ALERT "rmc_loader_init() alloc_chrdev_region failed!\n");
		return ret;
	}

	cdev_init(&rmc_loader_cdev, &rmc_loader_fops);
	ret = cdev_add(&rmc_loader_cdev, rmc_loader_dev, 1);
	if (ret) {
		printk(KERN_ALERT "rmc_loader_init() cdev_init/cdev_add failed!\n");
		return ret;
	}

#ifdef CONFIG_U2_STM_ETR_TO_SDRAM 
	stm_sdram_size = 	__raw_readl(CPU_ETR_RSZ) * 4;
	stm_sdram_base =	__raw_readl(CPU_ETR_DBALO);
	stm_sdram_write_ptr = 	__raw_readl(CPU_ETR_RWP);
	stm_sdram_read_ptr =	stm_sdram_base;
#else
	stm_sdram_size = 	0;
	stm_sdram_base =	0;
	stm_sdram_write_ptr = 	0;
	stm_sdram_read_ptr =	0;
#endif		
	printk(KERN_ALERT "rmc_loader_init() STM SDRAM size=0x%x, base=0x%x, wptr=0x%x, rptr=0x%x\n", stm_sdram_size, stm_sdram_base, stm_sdram_write_ptr,
		stm_sdram_read_ptr);


	pr_info("rmc_loader_init OK\n");

	return 0;
}
module_init(rmc_loader_init);

static void __exit rmc_loader_exit(void)
{
	printk(KERN_ALERT "rmc_loader_exit()\n");
        cdev_del(&rmc_loader_cdev);
        unregister_chrdev_region(rmc_loader_dev, 1);
	if (toc_buffer) kfree(toc_buffer);
	if (toc) kfree(toc);
	if (data_buf) kfree(data_buf);
}
module_exit(rmc_loader_exit);

MODULE_AUTHOR("Tommi.Kaikkonen@renesasmobile.com");
MODULE_ALIAS("rmc_mdm_loader");
MODULE_DESCRIPTION("Renesas SoC Modem Loader");
MODULE_LICENSE("GPL v2");
