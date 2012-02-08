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

static void ape5r_modify_register32(unsigned long addr, unsigned long set, unsigned long clear)
{
  unsigned long data = 0;
  data = *(volatile unsigned long *)addr;
  data &= ~clear;
  data |= set;
  *(volatile unsigned long *)addr = data;
}


static int __devexit rmc_loader_remove(struct device *dev)
{
	printk(KERN_ALERT "rmc_loader_remove()\n");
	return 0;
}

static ssize_t rmc_loader_read(struct file *file, char __user *buf,
						size_t len, loff_t *ppos)
{
	printk(KERN_ALERT "rmc_loader_read()\n");
	return 0;
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
#if 0
	printk(KERN_ALERT "rmc_loader_open(), ToDo: Initialize modem to receive boot data\n");
#else
	printk(KERN_ALERT "rmc_loader_open(), Initialize modem to receive boot data >>\n");
	ape5r_modify_register32(0xE618004C, 0, (1<<7)); /* read-modify clear C4POWCR.MDMSEL to allow modem requests */
    printk(KERN_ALERT "T001\n");
        *(volatile uint32_t *)0xE6190028 = 0x03000000; /* PLL lock count for PLL5 */
    printk(KERN_ALERT "T002\n");

        *(volatile uint32_t *)0xE6190020 = 0x32000100; /* PLL5 (PLL2 in EPMU doc) control register */
                                                       /*     bits [31:25] is MULFAC[6:0]           */
                                                       /*     bits [14:8]  is DIVFAC[6:0]           */
                                                       /*     HFCLKC = (MULFAC+1)*?38.4?/(DIVFAC+1) MHz */
                                                       /* Now MULFAC=25 and DIVFAC=1, i.e. HFCLK=38.4*26/2 MHz=499.2MHz */
    printk(KERN_ALERT "T003\n");

	*(volatile uint32_t *)0xE6190024 = 0x30000000; /* RF Clock Control Register (RFCLK_CR)        */
                                                       /*     bits [31:26] is RFCLK_DIV[5:0]        */
                                                       /*     RFCLK = HFCLK/(RFCLK_DIV+1)               */
                                                       /* Now RFCLK_DIV=HFCLK/13 = 38.4 MHz          */
                                                       /* RFCLKC divider for 38.4 MHz */
    printk(KERN_ALERT "T004\n");

	*(volatile uint32_t *)0xE6001200 = 0x40000000; /* Configure upper 8 bits of OCP bus for modem */
	                                               /* This should be deferred until we know where to laod modem! */
    printk(KERN_ALERT "T005\n");

        *(volatile uint32_t *)0xE6001208 = 0x06000000; /* OCP Bridge Window Reg3: Configure upper 7 bits of modem peripheral OCP address for APE to access */
                                                            /* FOR APE, range 0xE2000000--0xE3FFFFFF maps to THIS+(0x00000000--0x01FFFFFF). */
                                                            /* So to e.g. for APE to access Modem SCU_AD base address would be */
                                                            /*   0xE3D40000 ==> OCP Bus address 0x07D40000, and */
                                                            /* SCU_AD.CCR.OUTPUT would be mapped like this: */
                                                            /*   0xE3D40410 ==> 0x07D0410            */
    printk(KERN_ALERT "T006\n");
	*(volatile uint32_t *)0xE6190000 = 0x00000001; /* Enable Modem */
    printk(KERN_ALERT "T007\n");

	while (0x00000000 != *(volatile uint32_t *)0xE6190000) {
		/* Wait until WGM is up, how long this takes? Should we usleep? */
	}
    printk(KERN_ALERT "T008\n");

	*(volatile uint32_t *)0xE6190004 = 0x00000002; /* Host Access request */
    printk(KERN_ALERT "T009\n");

	while (0x00000003 != *(volatile uint32_t *)0xE6190004) {
		/* Wait until Access OK, how long this takes? Should we usleep? */
	}
    printk(KERN_ALERT "T010\n");

	ape5r_modify_register32(0xE6150144, 0, (1<<25) | (0x0F << 16)); /* Module stop control register 5 (SMSTPCR5) */
                                                              /* Supply clocks to OCP2SuperHiWay and OCP2Memory and SuperHiWay2OCP0/1 instances */
                                                              /* bit25: MSTP525=0, IICB0 operates */
                                                              /* bit19: MSTP519=0, O2S operates */
                                                              /* bit18: MSTP519=0, O2M operates */
                                                              /* bit17: MSTP517=0, S2O0 operates */
                                                              /* bit16: MSTP516=0, S2O1 operates */
        ape5r_modify_register32(0xE615013C, 0, (1<<25));
                               /* SMSTPCR3 Bit25: MSTP325=0, HSI1 operates */


// IF I WANT TO RESET MODEM, THIS WILL DO IT:
// EPMU RES_CR registger, wriet 1. TO CHECK: how to know when modem is finished with resetting so that I can access TCM memories?	
	printk(KERN_ALERT "rmc_loader_open(), Initialize modem to receive boot data <<\n");
#endif
	return ret;
}

static int rmc_loader_release(struct inode *inode, struct file *file)
{
        void __iomem * remapped_mdm_io=0;
	int retval=0;

	if (all_toc_entries_loaded) {
#if 0
		printk(KERN_ALERT "rmc_loader_relase(), ToDo: Release Modem L2 CPU from Prefetch Hold\n");
#else
		printk(KERN_ALERT "rmc_loader_relase(), Release Modem L2 CPU from Prefetch Hold >>\n");
                remapped_mdm_io = ioremap(0xE3D40410,4); 
                __raw_writel(2, ((void __iomem *)((uint32_t)remapped_mdm_io)));
//		*(volatile uint32_t *)remapped_mdm_io = 0x2; /* Release Cortex R4 L23 from pre-fetch hold */
                                               /* bit 5: 0=Internal TCM boot, 1=External ROM boot (VINTHI) */
                                               /* bit 4: 0=HF clock, 1=RF Clock forced to be used */
                                               /* bit 3: 0=PSS Clk Req is NOT masked, 1=PSS Clkc Req is MASKED */
                                               /* bit 2: 0=EModem MA_Int is selected, 1=EModem MA_Int is NOT selected */
                                               /* bit 1: 0=Cortex R4 L23 in pre-fetch hold, 1=run */
                                               /* bit 0: 0=Cortex R4 L1  in pre-fetch hold, 1=run */ 
                iounmap(remapped_mdm_io);
		printk(KERN_ALERT "rmc_loader_relase(), Release Modem L2 CPU from Prefetch Hold <<\n");
#endif
	} else {
                printk(KERN_ALERT "rmc_loader_release(), load_ptr=0x%08x\n", (unsigned int)load_ptr);
		printk(KERN_ALERT "rmc_loader_release(), Not all TOC entries loaded, NOT releasing modem!\n");
		retval = -EFAULT;
	}
  	toc_ptr = toc_buffer;
	data_ptr = 0;
	toc_index = 0;
	load_ptr = 0;
	all_toc_entries_loaded = 0;

	return retval;
}

static const struct file_operations rmc_loader_fops = {
	.owner		= THIS_MODULE,
	.read		= rmc_loader_read,
	.write		= rmc_loader_write,
	.open		= rmc_loader_open,
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
