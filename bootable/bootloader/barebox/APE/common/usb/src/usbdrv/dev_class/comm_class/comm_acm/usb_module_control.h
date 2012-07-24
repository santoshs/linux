/* usb_module_control.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __USB_MODULE_CONTROL_H__
#define __USB_MODULE_CONTROL_H__


#define USBDEV_MODE_NONE                0x00    // usb driver stop
#define USBDEV_MODE_COMM_ACM            0x02    // comm class open

#define USBDEV_STATE_NORMAL             0x00    // usb driver normal state

typedef void (*MODCHG_CALLBACK)(void);
typedef void (*ACCESSMOD_CALLBACK)(unsigned char bAccessMode);

// struct _st_USBDEV_MODCHG_GLOBALS
// brief mode change struct
typedef struct _st_USBDEV_MODCHG_GLOBALS {
    unsigned long           uUsbDevModchgFlagID;            // FlagID
    unsigned long           uUsbDevModchgTaskID;            // TaskID
    unsigned char           bUsbDevModchgOpenFlag;          // ModeChange OpenFlag
    unsigned char           bUsbDevModchgDevStkOpenCount;   // DeviceStuck OpenCount
    unsigned char           bAccessMode;                    // Access Mode
    unsigned char           bUsbmoduleState;                // USBModule State(0:noemal state 1:stop state)
    unsigned char           bMediaAccessState;              // MediaAccessState(0:not access 1:access state)
    MODCHG_CALLBACK         ModChgCallback;                 // ModeChange CallBack
    ACCESSMOD_CALLBACK      AccessModeCallback;             // AccessModeChange CallBack
    unsigned long           uMsBotDevTaskID;                // DeviceTaskID
    unsigned long           uMsBotDevMediaStatusCheckTaskID;// MediaCheckTaskID
    unsigned long           uCommAcmDevTaskID;              // CommAcmTaskID
} st_USBDEV_MODCHG_GLOBALS, *pst_USBDEV_MODCHG_GLOBALS;

#define USBDEV_MODCHG_GLOBAL()      (&stUsbDevModchgGlobal)

#endif /* __USB_MODULE_CONTROL_H__ */

