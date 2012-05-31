/*
* File: tmu_driver.c
* Description: 
*/
#include <linux/kernel.h>	// for printk()
#include <linux/module.h>	// this is module type device driver
#include <linux/moduleparam.h>
#include <linux/slab.h>		// for kmalloc()...
#include <linux/errno.h>	// use error codes
#include <linux/types.h>	// size_t define
#include <linux/fcntl.h>
#include <linux/cdev.h>		// this is character device
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/ioport.h>	// I/O drive
#include <asm/io.h>			// I/O drive
#include <asm/uaccess.h>	// user/kernel access API
#include <linux/ioctl.h>	// needed for the _IOW etc stuff used later
#include <linux/miscdevice.h>
#include "tmu_driver.h"


#define UNIT_TEST				0
#define DEBUG					0			/* for debug */

typedef void					VOID;
typedef int						INT;
typedef unsigned int			UINT;
typedef unsigned long long		UWW;		/* unsigned long long int */

#define SUCCESS					(0)
#define FAILURE					(-1)

#define TMU_MP_CLOCK						(48 * 1000000)				/* MP:48MHz (system-dependent) */
#if 0
#define TMC_COUNTER_TIME(time, sysclk)      (((UWW)(time) * (sysclk))/1000000)
#define TMC_MAX_COUNTER_TIME(sysclk)		(((UWW)0xFFFFFFFF * 1000000) / sysclk)
#define TMC_COUNTER_VALUE(tmrcnt,sysclk)	(((UWW)tmrcnt * 1000000) / sysclk)
#else
#define TMC_COUNTER_TIME(time, sysclk)      My_do_div(((UWW)(time) * (sysclk)), 1000000)
#define TMC_MAX_COUNTER_TIME(sysclk)		My_do_div(((UWW)0xFFFFFFFF * 1000000), (sysclk))
#define TMC_COUNTER_VALUE(tmrcnt,sysclk)	My_do_div(((UWW)(tmrcnt) * 1000000), (sysclk))
#endif

#if 1
#define APB_TMU2_BASE_ADDRESS	(0xFFF70000UL)	/* TMU of RT */
#else
#define APB_TMU2_BASE_ADDRESS	(0xE61E0000UL)	/* TPU of SYS "for debug" */
#endif
#define REG_TSTR				(0x0004UL)		/* Timer start register2		 8 */
#define REG_TCOR				(0x0020UL)		/* Timer constant register2_2	32 */
#define REG_TCNT				(0x0024UL)		/* Timer counter2_2				32 */
#define REG_TCR					(0x0028UL)		/* Timer control register2_2	16 */
#define BIT_TSTR_STR			(0x04)			/* Counter Start 2 */
#define BIT_TCR_TPSC			(0x01)			/* Count on MP/16 */
#define BIT_TCR_UNIE			(0x20)			/* Underflow Interrupt Control */

#if (DEBUG == 1)
#define MSTPSR1					(0xE6150000UL)
#define MSTPSR1_1				(0x0038UL)
#define MPCKCR					(0x0080UL)
#define SMSTPCR1_1				(0x0134UL)

#define PSTR_BASE				(0xE6180000UL)
#define PSTR					(0x0080UL)
//#define SWUCR_BASE				(0xE6180000UL)
//#define SWUCR					(0014UL)
#endif

#define KERNEL_DRIVER_NAME "tmu"

/*
* static variable
*/
static void *pmap	= 0;		/* I/O base address, translated MMU */

#if (DEBUG == 1)
void* mspmap = 0;
void* pspmap = 0;
#endif


/*
* local function
*/
static UINT My_do_div(UWW n, UINT base)
{
	UWW wk;

	wk = n;
	do_div(wk, base);

//	printk(KERN_ERR "tmu: My_do_div() n=0x%llx base=0x%x div=0x%llx \n", n, base, wk);
	return (UINT) wk;
}

static UINT Tmc_Get_CounterValue(VOID)
{
	UINT uiSysClk;
	UINT uiCounter;

//	printk(KERN_ERR "tmu: Tmc_Get_CounterValue>>>\n");
	// get system clock
	if ( !(ioread8(pmap + REG_TSTR) & BIT_TSTR_STR) )
	{
		iowrite16(BIT_TCR_TPSC, pmap + REG_TCR );		// Count on 1/16
//		printk(KERN_ERR "tmu: Tmc_Get_CounterValue() Count on 1/16\n");
	}
	uiSysClk = TMU_MP_CLOCK;
	uiSysClk /= (4 << ((ioread16(pmap + REG_TCR) & 0x7) * 2));

	// get tmu counter
	uiCounter = (UINT) (TMC_COUNTER_VALUE(ioread32(pmap + REG_TCOR) - ioread32(pmap + REG_TCNT), uiSysClk));
//	printk(KERN_ERR "tmu: Tmc_Get_CounterValue() REG_TCNT = 0x%08x\n", ioread32(pmap + REG_TCNT));

//	printk(KERN_ERR "tmu: Tmc_Get_CounterValue<<<\n");
	return (uiCounter);
}

static VOID Tmc_Init_1ch (VOID)
{
	
//	printk(KERN_ERR "tmu: Tmc_Init_1ch>>>\n");
	
	//Timer counter disable
	iowrite8( ioread8(pmap + REG_TSTR) & (~BIT_TSTR_STR), pmap + REG_TSTR );

	//disable CountFinish interrupt
	iowrite16( 0x0000, pmap + REG_TCR );
	// 000: Count on
//	printk(KERN_ERR "tmu: Tmc_Init_1ch<<<\n");

	return;
}

static UINT Tmc_Get_CounterTime_Max (VOID)
{
	UINT uiSysClk;

//	printk(KERN_ERR "tmu: Tmc_Get_CounterTime_Max>>>\n");
	// get system clock
	if ( !(ioread8(pmap + REG_TSTR) & BIT_TSTR_STR) )
	{
		iowrite16(BIT_TCR_TPSC, pmap + REG_TCR );		// Count on 1/16
	}
	uiSysClk = TMU_MP_CLOCK;
	uiSysClk /= (4 << ((ioread16(pmap + REG_TCR) & 0x7) * 2));

//	printk(KERN_ERR "tmu: Tmc_Get_CounterTime_Max<<<  %08x\n", (UINT) TMC_MAX_COUNTER_TIME(uiSysClk));
	// return max counter time
	return (UINT) TMC_MAX_COUNTER_TIME(uiSysClk);
}

static INT Tmc_Set_CounterTime (UINT uiTime)
{
	UINT uiClk;
	UINT uiSysClk;

//	printk(KERN_ERR "tmu: Tmc_Set_CounterTime>>>\n");
	// get system clock
	if ( !(ioread8(pmap + REG_TSTR) & BIT_TSTR_STR) )
	{
		iowrite16(BIT_TCR_TPSC, pmap + REG_TCR );		// Count on 1/16
	}
	uiSysClk = TMU_MP_CLOCK;
	uiSysClk /= (4 << ((ioread16(pmap + REG_TCR) & 0x7) * 2));

	// parameter check
	if (uiTime > (UINT) TMC_MAX_COUNTER_TIME(uiSysClk))
	{
//		printk(KERN_ERR "tmu: Tmc_Set_CounterTime<<< parameter check fail\n");
		return FAILURE;
	}

	//Transform from time (uS) to clk number
	uiClk = (UINT) (TMC_COUNTER_TIME(uiTime, uiSysClk));

//	printk(KERN_ERR "tmu: Tmc_Set_CounterTime() uiClk 0x%8x\n", uiClk);
	//Set clk number to counter reset register
	if ( !(ioread8(pmap + REG_TSTR) & BIT_TSTR_STR) )
	{
//		printk(KERN_ERR "tmu: Tmc_Set_CounterTime() Set clk number to counter reset register\n");
		iowrite32( uiClk, pmap + REG_TCOR );
		iowrite32( uiClk, pmap + REG_TCNT );
//		printk(KERN_ERR "tmu: Tmc_Set_CounterTime() REG_TCOR = 0x%08X\n", ioread32(pmap + REG_TCOR));
//		printk(KERN_ERR "tmu: Tmc_Set_CounterTime() REG_TCNT = 0x%08X\n", ioread32(pmap + REG_TCNT));
	}

//	printk(KERN_ERR "tmu: Tmc_Set_CounterTime<<<\n");
	return SUCCESS;
}

static VOID Tmc_Start_Counter_1ch (VOID)
{
//	printk(KERN_ERR "tmu: Tmc_Start_Counter_1ch>>>\n");
	//Timer Counter value Reset
	iowrite8( ioread8(pmap + REG_TSTR) & (~BIT_TSTR_STR), pmap + REG_TSTR );
	iowrite16( (ioread16(pmap + REG_TCR) & 0x7) | BIT_TCR_UNIE, pmap + REG_TCR );
	iowrite32( ioread32(pmap + REG_TCOR), pmap + REG_TCNT);

	Tmc_Set_CounterTime(Tmc_Get_CounterTime_Max());

	//Timer Counter Enable
	iowrite8( ioread8(pmap + REG_TSTR) | BIT_TSTR_STR, pmap + REG_TSTR );

//	printk(KERN_ERR "tmu: Tmc_Start_Counter_1ch() REG_TSTR = 0x%02X\n", ioread8(pmap + REG_TSTR));
//	printk(KERN_ERR "tmu: Tmc_Start_Counter_1ch() REG_TCR  = 0x%04X\n", ioread16(pmap + REG_TCR));
//	printk(KERN_ERR "tmu: Tmc_Start_Counter_1ch() REG_TCOR = 0x%08X\n", ioread32(pmap + REG_TCOR));
//	printk(KERN_ERR "tmu: Tmc_Start_Counter_1ch() REG_TCNT = 0x%08X\n", ioread32(pmap + REG_TCNT));

//	printk(KERN_ERR "tmu: Tmc_Start_Counter_1ch<<<\n");
	return;
}

static VOID Tmc_Stop_Counter_1ch (VOID)
{
//	printk(KERN_ERR "tmu: Tmc_Stop_Counter_1ch>>>\n");
	// stop counter
	iowrite8( ioread8(pmap + REG_TSTR) & (~BIT_TSTR_STR), pmap + REG_TSTR );
	iowrite16( ioread16(pmap + REG_TCR) & 0x7, pmap + REG_TCR );

//	printk(KERN_ERR "tmu: Tmc_Stop_Counter_1ch<<<\n");
	return;
}

/*
* open method : no operation, return zero all times
*/
int tmu_open (struct inode *inode, struct file *fp)
{
//	printk(KERN_ERR "tmu: open\n");
	
	pmap = ioremap_nocache(APB_TMU2_BASE_ADDRESS, 256);
//	printk(KERN_ERR "tmu: pmap=%08x\n", (UINT)pmap);
	return 0;
}

/*
* release method : no operation, return zero all times
*/
int tmu_release (struct inode *inode, struct file *fp)
{
//	printk(KERN_ERR "tmu: release\n");
	iounmap(pmap);
	return 0;
}

/*
* read method
*/
ssize_t tmu_read(struct file *fp, char __user *buf, size_t count, loff_t *f_pos)
{
	UINT uiCounter;
	UINT ret;

//	printk(KERN_ERR "tmu: tmu_read>>>\n");
	if( count < 4 ) {
//		printk(KERN_ERR "tmu: tmu_read() count < 4<<<\n");
		return 0;
	}

	uiCounter = Tmc_Get_CounterValue();
	ret = copy_to_user((void *)buf, (const void *)&uiCounter, 4);
//	printk(KERN_ERR "tmu: tmu_read<<<\n");

	return 4;
}

/*
* write method
*/
ssize_t tmu_write(struct file *fp,  const char __user *buf, size_t count, loff_t *f_pos)
{
//	printk(KERN_ERR "tmu: tmu_write>>>\n");
//	printk(KERN_ERR "tmu: tmu_write<<<\n");
	return 0;
}

/*
* ioctl method
*/
long tmu_unlocked_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
	UINT uiCounter;

//	printk(KERN_ERR "tmu: ioctl\n");
	
	switch( cmd ){
		case TMU_IOCTL_GET :
//			printk(KERN_ERR "tmu: ioctl() IOCTL_GET" );
			uiCounter = Tmc_Get_CounterValue();
			if ( put_user( uiCounter, ( UINT* )arg ) )
			    return -EFAULT;
			break;
		case TMU_IOCTL_SET :
//			printk(KERN_ERR "tmu: ioctl() IOCTL_SET" );
			if ( get_user( uiCounter, ( UINT* )arg ) )
			    return -EFAULT;
			uiCounter = Tmc_Set_CounterTime( uiCounter );
			break;
		case TMU_IOCTL_MAX :
//			printk(KERN_ERR "tmu: ioctl() IOCTL_MAX" );
			uiCounter = Tmc_Get_CounterTime_Max();
			if ( put_user( uiCounter, ( UINT* )arg ) )
			    return -EFAULT;
			break;
		default :
			return -EINVAL;
	}
	
	return 0;
}

/*
* register driver points
* open, read, write, ioctl, and close
*
*/
struct file_operations tmu_fops = {
	.owner	= THIS_MODULE,
	.open	= tmu_open,
	.release= tmu_release,
	.read	= tmu_read,
	.write	= tmu_write,
	.unlocked_ioctl	= tmu_unlocked_ioctl,
};

struct miscdevice TmuDevs;

/*
* tmu_init - function to insert this module into kernel space
*
* This is the first of two exported functions to handle inserting this
* code into a running kernel
*
* Returns 0 if successfull, otherwise -1
*/
static int __init tmu_init(void)
{
//	int result;
#if (UNIT_TEST == 1)
	UINT count = 0;
	UINT timer = 0;
#endif
	
//	printk(KERN_ERR "tmu: tmu_init\n");
//	printk(KERN_ERR "tmu: tmu_init() TmuDevs.name = %s\n", KERNEL_DRIVER_NAME);
//	printk(KERN_ERR "tmu: tmu_init() TmuDevs.minor = MISC_DYNAMIC_MINOR\n");
	TmuDevs.name = KERNEL_DRIVER_NAME;
	TmuDevs.fops = &tmu_fops;
	TmuDevs.minor = MISC_DYNAMIC_MINOR;	
	misc_register(&TmuDevs);
	
#if (DEBUG == 1)
	mspmap = ioremap_nocache(MSTPSR1, 512);
	pspmap = ioremap_nocache(PSTR_BASE, 256);
	
	iowrite32( 0x0000018CUL, mspmap + MPCKCR );
	
//	printk(KERN_ERR "tmu: Tmc_Init_1ch() PSTR = 0x%08X\n",ioread32( pspmap + PSTR ));
	
//	printk(KERN_ERR "tmu: Tmc_Init_1ch() B_MSTPSR1_1 = 0x%08X\n",ioread32( mspmap + MSTPSR1_1 ));
//	printk(KERN_ERR "tmu: Tmc_Init_1ch() B_MPCKCR = 0x%08X\n",ioread32( mspmap + MPCKCR ));
	iowrite32( ioread32( mspmap + MSTPSR1_1 ) & (~0x2000000), mspmap + SMSTPCR1_1);
//	printk(KERN_ERR "tmu: Tmc_Init_1ch() SMSTPCR1_1 = 0x%08X\n",ioread32( mspmap + SMSTPCR1_1 ));
	
	while( ioread32( mspmap + MSTPSR1_1 )&(0x2000000) )
	{
			;
	}
//	printk(KERN_ERR "tmu: Tmc_Init_1ch() A_MSTPSR1_1 = 0x%08X\n",ioread32( mspmap + MSTPSR1_1 ));
//	printk(KERN_ERR "tmu: Tmc_Init_1ch() A_MPCKCR = 0x%08X\n",ioread32( mspmap + MPCKCR ));
#endif

#if (UNIT_TEST == 1)

	tmu_open(NULL, NULL);
	Tmc_Init_1ch();
	Tmc_Start_Counter_1ch();
	
	for ( count = 0; count < 3; count++ )
	{
		timer = Tmc_Get_CounterValue();
//		printk(KERN_ERR "tmu: Tmc_Get_CounterValue timer = %d(us)\n", timer);
	}
	
#endif
	
	return 0;
}

/*
* tmu_cleanup - function to cleanup this module from kernel space
*
* This is the second of two exported functions to handle cleanup this
* code from a running kernel
*/
static void __exit tmu_cleanup(void)
{
//	printk(KERN_ERR "tmu: tmu_cleanup\n");

	misc_deregister(&TmuDevs);

#if (UNIT_TEST == 1)
	Tmc_Stop_Counter_1ch();
	tmu_release(NULL, NULL);
#endif
}

module_init(tmu_init);
module_exit(tmu_cleanup);

MODULE_DESCRIPTION("TMU driver.");
MODULE_AUTHOR ("");
MODULE_LICENSE("GPL");
MODULE_ALIAS("");

