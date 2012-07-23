/*	usb.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __USB_H__
#define __USB_H__

// bitmap masks for bmRequestType
#define REQ_DEVICE_TO_HOST				0x80

#define REQ_TYPE_CLASS					0x20
#define REQ_TYPE_VENDOR					0x40

#define REQ_RECIPIENT_INTERFACE			0x01
#define REQ_RECIPIENT_MASK				0x03

#pragma pack(push,1)
typedef struct _st_DEVICE_REQUEST {
	UCHAR	bmRequestType;			// Characteristics of request:
									//	D7: Data transfer direction
									//	0 = Host-to-device
									//	1 = Device-to-host
									//	D6...5: Type
									//	0 = Standard
									//	1 = Class
									//	2 = Vendor
									//	3 = Reserved
									//	D4...0: Recipient
									//	0 = Device
									//	1 = Interface
									//	2 = Endpoint
									//	3 = Other
									//	4...31 = Reserved
	UCHAR	bRequest;				// Specific request
	USHORT	wValue;					// according to request
	USHORT	wIndex;					// according to request; typically used to pass an index or offset
	USHORT	wLength;				// Number of bytes to transfer if there is a Data stage
} st_DEVICE_REQUEST, *pst_DEVICE_REQUEST;
#pragma pack(pop)

// Descriptor types
#define DESC_TYPE_EP					0x05
#define DESC_TYPE_DEVICE_QUAL			0x06

#pragma pack(push,1)
typedef struct _st_DEVICE_DESCRIPTOR {
	UCHAR	bLength;				// Number Size of this descriptor in bytes
	UCHAR	bDescriptorType;		// Constant DEVICE Descriptor Type
	USHORT	bcdUSB;					// BCD USB Specification Release Number
	UCHAR	bDeviceClass;			// Class code
	UCHAR	bDeviceSubClass;		// Subclass code
	UCHAR	bDeviceProtocol;		// Protocol code
	UCHAR	bMaxPacketSize0;		// Number Maximum packet size for endpoint zero
	USHORT	idVendor;				// Vendor ID
	USHORT	idProduct;				// Product ID
	USHORT	bcdDevice;				// BCD Device release number
	UCHAR	iManufacturer;			// Index of string descriptor describing manufacturer
	UCHAR	iProduct;				// Index of string descriptor describing product
	UCHAR	iSerialNumber;			// Number of possible configurations
	UCHAR	bNumConfigurations;
} st_DEVICE_DESCRIPTOR, *pst_DEVICE_DESCRIPTOR;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct _st_DEVICE_QUALIFIER {
	UCHAR	bLength;				// Number Size of descriptor
	UCHAR	bDescriptorType;		// Constant Device Qualifier Type
	USHORT	bcdUSB;					// BCD USB specification version number
	UCHAR	bDeviceClass;			// Class Code
	UCHAR	bDeviceSubClass;		// SubClass Code
	UCHAR	bDeviceProtocol;		// Protocol Code
	UCHAR	bMaxPacketSize0;		// Number Maximum packet size for other speed
	UCHAR	bNumConfigurations;		// Number of Other-speed Configurations
	UCHAR	bReserved;				// Zero Reserved for future use, must be zero
} st_DEVICE_QUALIFIER, *pst_DEVICE_QUALIFIER;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct _st_CONFIG_DESCRIPTOR {
	UCHAR	bLength;				// Number Size of descriptor
	UCHAR	bDescriptorType;		// Constant Device Qualifier Type
	USHORT	wTotalLength;			// Number Total length of data returned for this configuration
	UCHAR	bNumInterfaces;			// Number of interfaces supported by this configuration
	UCHAR	bConfigurationValue;	// Number Value to use as an argument to the SetConfiguration()
	UCHAR	iConfiguration;			// Index of string descriptor describing this configuration
	UCHAR	bmAttributes;			// Bitmap Configuration characteristics
									//	D7: Reserved (set to one)
									//	D6: Self-powered
									//	D5: Remote Wakeup
									//	D4...0: Reserved (reset to zero)
	UCHAR	bMaxPower;				// mA Maximum power consumption of the USB device from the bus
									//	Expressed in 2 mA units
} st_CONFIG_DESCRIPTOR, *pst_CONFIG_DESCRIPTOR;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct _st_ENDPOINT_DESCRIPTOR {
	UCHAR	bLength;				// Number Size of this descriptor in bytes
	UCHAR	bDescriptorType;		// Constant ENDPOINT Descriptor Type
	UCHAR	bEndpointAddress;		// Endpoint The address of the endpoint on the USB device
									//	Bit 3...0: The endpoint number
									//	Bit 6...4: Reserved, reset to zero
									//	Bit 7: Direction, ignored for control endpoints
									//	0 = OUT endpoint
									//	1 = IN endpoint
	UCHAR	bmAttributes;			// Bitmap This field describes the endpoint's attributes
									//	Bits 1..0: Transfer Type
									//	00 = Control
									//	01 = Isochronous
									//	10 = Bulk
									//	11 = Interrupt
									//	If not an isochronous endpoint, bits 5..2 are reserved
									//	Bits 3..2: Synchronization Type
									//	00 = No Synchronization
									//	01 = Asynchronous
									//	10 = Adaptive
									//	11 = Synchronous
									//	Bits 5..4: Usage Type
									//	00 = Data endpoint
									//	01 = Feedback endpoint
									//	10 = Implicit feedback Data endpoint
									//	11 = Reserved
	USHORT	wMaxPacketSize;			// Number Maximum packet size for this endpoint
	UCHAR	bInterval;				// Number Interval for polling endpoint for data transfers
									//	For full-/low-speed interrupt endpoints, the value of
									//	this field may be from 1 to 255.
									//	For high-speed interrupt endpoints, the bInterval value
									//	is used as the exponent for a 2^(bInterval-1) value; e.g., a
									//	bInterval of 4 means a period of 8 (24-1). This value
									//	must be from 1 to 16.
} st_ENDPOINT_DESCRIPTOR, *pst_ENDPOINT_DESCRIPTOR;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct _st_LANGUAGE_ID {
	UCHAR	bLength;				// N+2 Size of this descriptor in bytes
	UCHAR	bDescriptorType;		// Constant STRING Descriptor Type
	USHORT	wLangID;				// Number LANGID code zero
} st_LANGUAGE_ID, *pst_LANGUAGE_ID;
#pragma pack(pop)

#define MAX_STRING_LEN					254

#pragma pack(push,1)
typedef struct _st_STRING_DESCRIPTOR {
	UCHAR	bLength;				// Number Size of this descriptor in bytes
	UCHAR	bDescriptorType;		// Constant STRING Descriptor Type
	USHORT	wString[MAX_STRING_LEN];// UNICODE encoded string
} st_STRING_DESCRIPTOR, *pst_STRING_DESCRIPTOR;
#pragma pack(pop)

#endif /* __USB_H__ */
