#ifndef TMU_DRIVER_H
#define TMU_DRIVER_H

/*
* tmu_driver.h
*/
#include <linux/ioctl.h>

/* 
* Magic number to use ioctl() for /dev/chardev
*
* Refer to /linux/Documentation/ioctl/ioctl-number.txt,
* choose what code do not overlap.
*/
#define TMU_IOCTL_MAGIC		0xAE

#define TMU_IOCTL_GET		_IOC( _IOC_READ,  TMU_IOCTL_MAGIC, 0, sizeof( int ) )
#define TMU_IOCTL_SET		_IOC( _IOC_WRITE, TMU_IOCTL_MAGIC, 1, sizeof( int ) )
#define TMU_IOCTL_MAX		_IOC( _IOC_READ,  TMU_IOCTL_MAGIC, 2, sizeof( int ) )

#endif
