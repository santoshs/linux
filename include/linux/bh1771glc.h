/******************************************************************************
 * MODULE     : bh1771glc.h
 * FUNCTION   : BH1771 header of Proximity Sensor and Ambient Light Sensor IC
 *****************************************************************************/
#ifndef _BH1771GLC_H_
#define _BH1771GLC_H_

/************ command definition of ioctl ************/
/* IOCTL for Proximity & Light sensor */
#define ENABLE				1
#define DISABLE				0

#define IOCTL_ALS_ENABLE        _IOW(0xA2, 0x11, char)
#define IOCTL_ALS_SET_DELAY     _IOW(0xA2, 0x12, short)

#define IOCTL_PS_ENABLE         _IOW(0xA2, 0x13, char)
#define IOCTL_PS_SET_DELAY      _IOW(0xA2, 0x14, short)

#define IOCTL_GET_NV_ADDRESS    _IOW(0xA2, 0x15, unsigned long)

#endif /* _BH1771GLC_H_ */
