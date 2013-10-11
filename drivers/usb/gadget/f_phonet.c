/*
 * f_phonet.c -- USB CDC Phonet function
 *
 * Copyright (C) 2007-2008 Nokia Corporation. All rights reserved.
 *
 * Author: Rémi Denis-Courmont
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/device.h>

#include <linux/netdevice.h>
#include <linux/if_ether.h>
#include <linux/if_phonet.h>
#include <linux/if_arp.h>
#include <linux/phonet.h>
#include <linux/rtnetlink.h>

#include <linux/usb/ch9.h>
#include <linux/usb/cdc.h>
#include <linux/usb/composite.h>

#include <net/phonet/pn_dev.h>
#include "u_phonet.h"

#define PN_NETDEV_AUTOCONF
#define PN_MEDIA_USB	0x1B
#define MAXPACKET	512
#if (PAGE_SIZE % MAXPACKET)
#error MAXPACKET must divide PAGE_SIZE!
#endif

/*-------------------------------------------------------------------------*/

struct phonet_port {
	struct f_phonet			*usb;
	spinlock_t			lock;
};

struct f_phonet {
	struct usb_function		function;
	u8				ctrl_id, data_id;
	struct {
		struct sk_buff		*skb;
		spinlock_t		lock;
	} rx;
	struct net_device		*dev;
	struct usb_ep			*in_ep, *out_ep;

	struct usb_request		*in_req;
	struct usb_request		*out_reqv[0];
};

static int phonet_rxq_size = 17;

static inline struct f_phonet *func_to_pn(struct usb_function *f)
{
	return container_of(f, struct f_phonet, function);
}

/*-------------------------------------------------------------------------*/

#define USB_CDC_SUBCLASS_PHONET	0xfe
#define USB_CDC_PHONET_TYPE	0xab

static struct usb_interface_assoc_descriptor
pn_iad_desc = {
	.bLength 	=	sizeof pn_iad_desc,
	.bDescriptorType = 	USB_DT_INTERFACE_ASSOCIATION,

	.bInterfaceCount = 	2,
	.bFunctionClass =	USB_CLASS_COMM,
	.bFunctionSubClass =	USB_CDC_SUBCLASS_PHONET,
	.bFunctionProtocol =	USB_CDC_PROTO_NONE,
};

static struct usb_interface_descriptor
pn_control_intf_desc = {
	.bLength =		sizeof pn_control_intf_desc,
	.bDescriptorType =	USB_DT_INTERFACE,

	/* .bInterfaceNumber =	DYNAMIC, */
	.bInterfaceClass =	USB_CLASS_COMM,
	.bInterfaceSubClass =	USB_CDC_SUBCLASS_PHONET,
	.bInterfaceProtocol =	USB_CDC_PROTO_NONE,
};

static const struct usb_cdc_header_desc
pn_header_desc = {
	.bLength =		sizeof pn_header_desc,
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_HEADER_TYPE,
	.bcdCDC =		cpu_to_le16(0x0110),
};

static const struct usb_cdc_header_desc
pn_phonet_desc = {
	.bLength =		sizeof pn_phonet_desc,
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_PHONET_TYPE,
	.bcdCDC =		cpu_to_le16(0x1505), /* ??? */
};

static struct usb_cdc_union_desc
pn_union_desc = {
	.bLength =		sizeof pn_union_desc,
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_UNION_TYPE,

	/* .bMasterInterface0 =	DYNAMIC, */
	/* .bSlaveInterface0 =	DYNAMIC, */
};

static struct usb_interface_descriptor
pn_data_nop_intf_desc = {
	.bLength =		sizeof pn_data_nop_intf_desc,
	.bDescriptorType =	USB_DT_INTERFACE,

	/* .bInterfaceNumber =	DYNAMIC, */
	.bAlternateSetting =	0,
	.bNumEndpoints =	0,
	.bInterfaceClass =	USB_CLASS_CDC_DATA,
};

static struct usb_interface_descriptor
pn_data_intf_desc = {
	.bLength =		sizeof pn_data_intf_desc,
	.bDescriptorType =	USB_DT_INTERFACE,

	/* .bInterfaceNumber =	DYNAMIC, */
	.bAlternateSetting =	1,
	.bNumEndpoints =	2,
	.bInterfaceClass =	USB_CLASS_CDC_DATA,
};

static struct usb_endpoint_descriptor
pn_fs_sink_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor
pn_hs_sink_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(MAXPACKET),
};

static struct usb_endpoint_descriptor
pn_fs_source_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor
pn_hs_source_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};

static struct usb_descriptor_header *fs_pn_function[] = {
	(struct usb_descriptor_header *) &pn_iad_desc,
	(struct usb_descriptor_header *) &pn_control_intf_desc,
	(struct usb_descriptor_header *) &pn_header_desc,
	(struct usb_descriptor_header *) &pn_phonet_desc,
	(struct usb_descriptor_header *) &pn_union_desc,
	(struct usb_descriptor_header *) &pn_data_nop_intf_desc,
	(struct usb_descriptor_header *) &pn_data_intf_desc,
	(struct usb_descriptor_header *) &pn_fs_sink_desc,
	(struct usb_descriptor_header *) &pn_fs_source_desc,
	NULL,
};

static struct usb_descriptor_header *hs_pn_function[] = {
	(struct usb_descriptor_header *) &pn_iad_desc,
	(struct usb_descriptor_header *) &pn_control_intf_desc,
	(struct usb_descriptor_header *) &pn_header_desc,
	(struct usb_descriptor_header *) &pn_phonet_desc,
	(struct usb_descriptor_header *) &pn_union_desc,
	(struct usb_descriptor_header *) &pn_data_nop_intf_desc,
	(struct usb_descriptor_header *) &pn_data_intf_desc,
	(struct usb_descriptor_header *) &pn_hs_sink_desc,
	(struct usb_descriptor_header *) &pn_hs_source_desc,
	NULL,
};

/* string descriptors: */
#define PN_CTRL_IDX	0
#define PN_DATA_IDX	1
#define PN_IAD_IDX	2

/* static strings, in UTF-8 */
static struct usb_string pn_string_defs[] = {
	[PN_CTRL_IDX].s = "Phonet Control",
	[PN_DATA_IDX].s = "Phonet Data",
	[PN_IAD_IDX ].s = "Phonet",
	{  /* ZEROES END LIST */ },
};

static struct usb_gadget_strings pn_string_table = {
	.language =	0x0409,	/* en-us */
	.strings =	pn_string_defs,
};

static struct usb_gadget_strings *pn_strings[] = {
	&pn_string_table,
	NULL,
};
/*-------------------------------------------------------------------------*/

static int pn_net_open(struct net_device *dev)
{
	netif_wake_queue(dev);
	return 0;
}

static int pn_net_close(struct net_device *dev)
{
	netif_stop_queue(dev);
	return 0;
}

static void pn_tx_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct f_phonet *fp = ep->driver_data;
	struct net_device *dev = fp->dev;
	struct sk_buff *skb = req->context;

	switch (req->status) {
	case 0:
		dev->stats.tx_packets++;
		dev->stats.tx_bytes += skb->len;
		break;

	case -ESHUTDOWN: /* disconnected */
	case -ECONNRESET: /* disabled */
		dev->stats.tx_aborted_errors++;
	default:
		dev->stats.tx_errors++;
	}

	dev_kfree_skb_any(skb);
	netif_wake_queue(dev);
}

static int pn_net_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct phonet_port *port = netdev_priv(dev);
	struct f_phonet *fp;
	struct usb_request *req;
	unsigned long flags;

	if (skb->protocol != htons(ETH_P_PHONET))
		goto out;

	spin_lock_irqsave(&port->lock, flags);
	fp = port->usb;
	if (unlikely(!fp)) /* race with carrier loss */
		goto out_unlock;

	req = fp->in_req;
	req->buf = skb->data;
	req->length = skb->len;
	req->complete = pn_tx_complete;
	req->zero = 1;
	req->context = skb;

	if (unlikely(usb_ep_queue(fp->in_ep, req, GFP_ATOMIC)))
		goto out_unlock;

	netif_stop_queue(dev);
	skb = NULL;

out_unlock:
	spin_unlock_irqrestore(&port->lock, flags);
out:
	if (unlikely(skb)) {
		dev_kfree_skb(skb);
		dev->stats.tx_dropped++;
	}
	return NETDEV_TX_OK;
}

static int pn_net_mtu(struct net_device *dev, int new_mtu)
{
	if ((new_mtu < PHONET_MIN_MTU) || (new_mtu > PHONET_MAX_MTU))
		return -EINVAL;
	dev->mtu = new_mtu;
	return 0;
}

static int
pn_net_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	/* struct if_phonet_req *req = (struct if_phonet_req *)ifr; */
	int ret;

	switch (cmd)
	{
	    case SIOCPNGAUTOCONF:
#ifdef PN_NETDEV_AUTOCONF
		ret = phonet_address_add(dev, PN_MEDIA_USB);
		if (ret)
			return ret;

		phonet_address_notify(RTM_NEWADDR, dev, PN_MEDIA_USB);
		phonet_route_add(dev, PN_DEV_PC);

		dev_open(dev);
#endif
		/* Return NOIOCTLCMD so Phonet won't do it again */
		return -ENOIOCTLCMD;
	}

	return -ENOIOCTLCMD;
}

static const struct net_device_ops pn_netdev_ops = {
	.ndo_open	= pn_net_open,
	.ndo_stop	= pn_net_close,
	.ndo_start_xmit	= pn_net_xmit,
	.ndo_do_ioctl   = pn_net_ioctl,
	.ndo_change_mtu	= pn_net_mtu,
};

static void pn_net_setup(struct net_device *dev)
{
	dev->features		= 0;
	dev->type		= ARPHRD_PHONET;
	dev->flags		= IFF_POINTOPOINT | IFF_NOARP;
	dev->mtu		= PHONET_DEV_MTU;
	dev->hard_header_len	= 1;
	dev->dev_addr[0]	= PN_MEDIA_USB;
	dev->addr_len		= 1;
	dev->tx_queue_len	= 5;

	dev->netdev_ops		= &pn_netdev_ops;
	dev->destructor		= free_netdev;
	dev->header_ops		= &phonet_header_ops;
}

/*-------------------------------------------------------------------------*/

/*
 * Queue buffer for data from the host
 */
static int
pn_rx_submit(struct f_phonet *fp, struct usb_request *req, gfp_t gfp_flags)
{
	struct page *page;
	int err;

	page = __skb_alloc_page(gfp_flags | __GFP_NOMEMALLOC, NULL);
	if (!page)
		return -ENOMEM;

	req->buf = page_address(page);
	req->length = PAGE_SIZE;
	req->context = page;

	err = usb_ep_queue(fp->out_ep, req, gfp_flags);
	if (unlikely(err))
		put_page(page);
	return err;
}

static void pn_rx_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct f_phonet *fp = ep->driver_data;
	struct net_device *dev = fp->dev;
	struct page *page = req->context;
	struct sk_buff *skb;
	unsigned long flags;

	if (likely(req->status == 0))
	{
		spin_lock_irqsave(&fp->rx.lock, flags);
		{
		skb = fp->rx.skb;
		if (!skb)
			skb = netdev_alloc_skb(dev, 12);
		if (req->actual < req->length) /* Last fragment */
			fp->rx.skb = NULL;
		else
			fp->rx.skb = skb;
		}
		spin_unlock_irqrestore(&fp->rx.lock, flags);

		if (unlikely(!skb))
			goto cont;

		if (skb->len == 0) { /* First fragment */
			skb->dev = dev;
			skb->protocol = htons(ETH_P_PHONET);
			skb_reset_mac_header(skb);
			/* Can't use pskb_pull() on page in IRQ */
			memcpy(skb_put(skb, 1), page_address(page), 1);
			skb_add_rx_frag(skb, skb_shinfo(skb)->nr_frags, page,
					1, req->actual-1, PAGE_SIZE);
		} else {
			skb_add_rx_frag(skb, skb_shinfo(skb)->nr_frags, page,
					0, req->actual, PAGE_SIZE);
		}
		page = NULL;

		if (req->actual < req->length) {
			dev->stats.rx_packets++;
			dev->stats.rx_bytes += skb->len;
			__skb_pull(skb, 1);
			netif_rx(skb);
		}
cont:
		pn_rx_submit(fp, req, GFP_ATOMIC | __GFP_COLD);
	} else {
		switch (req->status) {

	/* Do not resubmit in these cases: */
		case 0:
	case -ESHUTDOWN: /* disconnect */
	case -ECONNABORTED: /* hw reset */
	case -ECONNRESET: /* dequeued (unlink or netif down) */
		break;

	/* Do resubmit in these cases: */
	case -EOVERFLOW: /* request buffer overflow */
		dev->stats.rx_over_errors++;
	default:
		dev->stats.rx_errors++;
			pn_rx_submit(fp, req, GFP_ATOMIC | __GFP_COLD);
		break;
		}
	}

	if (page)
		put_page(page);
}

/*-------------------------------------------------------------------------*/

static void __pn_reset(struct usb_function *f)
{
	struct f_phonet *fp = func_to_pn(f);
	struct net_device *dev = fp->dev;
	struct phonet_port *port = netdev_priv(dev);

	netif_carrier_off(dev);
	port->usb = NULL;

	usb_ep_disable(fp->out_ep);
	usb_ep_disable(fp->in_ep);
	if (fp->rx.skb) {
		dev_kfree_skb_irq(fp->rx.skb);
		fp->rx.skb = NULL;
	}
}

static int pn_set_alt(struct usb_function *f, unsigned intf, unsigned alt)
{
	struct f_phonet *fp = func_to_pn(f);
	struct usb_gadget *gadget = fp->function.config->cdev->gadget;

	if (intf == pn_control_intf_desc.bInterfaceNumber)
		/* control interface, no altsetting */
		return (alt > 0) ? -EINVAL : 0;

	if (intf == pn_data_intf_desc.bInterfaceNumber) {
		struct net_device *dev = fp->dev;
		struct phonet_port *port = netdev_priv(dev);

		/* data intf (0: inactive, 1: active) */
		if (alt > 1)
			return -EINVAL;

		spin_lock(&port->lock);
		__pn_reset(f);
		if (alt == 1) {
			int i;

			if (config_ep_by_speed(gadget, f, fp->in_ep) ||
			    config_ep_by_speed(gadget, f, fp->out_ep)) {
				fp->in_ep->desc = NULL;
				fp->out_ep->desc = NULL;
				spin_unlock(&port->lock);
				return -EINVAL;
			}
			usb_ep_enable(fp->out_ep);
			usb_ep_enable(fp->in_ep);

			port->usb = fp;
			fp->out_ep->driver_data = fp;
			fp->in_ep->driver_data = fp;

			for (i = 0; i < phonet_rxq_size; i++)
				pn_rx_submit(fp, fp->out_reqv[i], GFP_ATOMIC | __GFP_COLD);
			netif_carrier_on(dev);
		}
		spin_unlock(&port->lock);
		return 0;
	}

	return -EINVAL;
}

static int pn_get_alt(struct usb_function *f, unsigned intf)
{
	struct f_phonet *fp = func_to_pn(f);

	if (intf == pn_control_intf_desc.bInterfaceNumber)
		return 0;

	if (intf == pn_data_intf_desc.bInterfaceNumber) {
		struct phonet_port *port = netdev_priv(fp->dev);
		int alt;

		spin_lock(&port->lock);
		alt = (port->usb != NULL);
		spin_unlock(&port->lock);
		return alt;
	}

	return -EINVAL;
}

static void pn_disconnect(struct usb_function *f)
{
	struct f_phonet *fp = func_to_pn(f);
	struct phonet_port *port = netdev_priv(fp->dev);
	unsigned long flags;

	/* remain disabled until set_alt */
	spin_lock_irqsave(&port->lock, flags);
	__pn_reset(f);
	spin_unlock_irqrestore(&port->lock, flags);
}

/*-------------------------------------------------------------------------*/


static int pn_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct usb_gadget *gadget = cdev->gadget;
	struct f_phonet *fp = func_to_pn(f);
	struct usb_ep *ep;
	int status, i;

	/* Reserve interface IDs */
	status = usb_interface_id(c, f);
	if (status < 0)
		goto err;
	fp->ctrl_id = status;
	pn_control_intf_desc.bInterfaceNumber = status;
	pn_union_desc.bMasterInterface0 = status;

	pn_iad_desc.bFirstInterface = status;

	status = usb_interface_id(c, f);
	if (status < 0)
		goto err;
	fp->data_id = status;
	pn_data_nop_intf_desc.bInterfaceNumber = status;
	pn_data_intf_desc.bInterfaceNumber = status;
	pn_union_desc.bSlaveInterface0 = status;

	/* Reserve endpoints */
	status = -ENODEV;
	ep = usb_ep_autoconfig(gadget, &pn_fs_sink_desc);
	if (!ep)
		goto err;
	fp->out_ep = ep;
	ep->driver_data = fp; /* Claim */

	ep = usb_ep_autoconfig(gadget, &pn_fs_source_desc);
	if (!ep)
		goto err;
	fp->in_ep = ep;
	ep->driver_data = fp; /* Claim */

	pn_hs_sink_desc.bEndpointAddress = pn_fs_sink_desc.bEndpointAddress;
	pn_hs_source_desc.bEndpointAddress = pn_fs_source_desc.bEndpointAddress;

	/* Do not try to bind Phonet twice... */
	status = usb_assign_descriptors(f, fs_pn_function, hs_pn_function,
			NULL);
	if (status)
		goto err;

	/* Incoming USB requests */
	status = -ENOMEM;
	for (i = 0; i < phonet_rxq_size; i++) {
		struct usb_request *req;

		req = usb_ep_alloc_request(fp->out_ep, GFP_KERNEL);
		if (!req)
			goto err_req;

		req->complete = pn_rx_complete;
		fp->out_reqv[i] = req;
	}

	/* Outgoing USB requests */
	fp->in_req = usb_ep_alloc_request(fp->in_ep, GFP_KERNEL);
	if (!fp->in_req)
		goto err_req;

	INFO(cdev, "USB CDC Phonet function\n");
	INFO(cdev, "using %s, OUT %s, IN %s\n", cdev->gadget->name,
		fp->out_ep->name, fp->in_ep->name);
	return 0;

err_req:
	for (i = 0; i < phonet_rxq_size && fp->out_reqv[i]; i++)
		usb_ep_free_request(fp->out_ep, fp->out_reqv[i]);
err:
	usb_free_all_descriptors(f);
	if (fp->out_ep)
		fp->out_ep->driver_data = NULL;
	if (fp->in_ep)
		fp->in_ep->driver_data = NULL;
	ERROR(cdev, "USB CDC Phonet: cannot autoconfigure\n");
	return status;
}

static void
pn_unbind(struct usb_configuration *c, struct usb_function *f)
{
	struct f_phonet *fp = func_to_pn(f);
	int i;

	/* We are already disconnected */
	if (fp->in_req)
		usb_ep_free_request(fp->in_ep, fp->in_req);
	for (i = 0; i < phonet_rxq_size; i++)
		if (fp->out_reqv[i])
			usb_ep_free_request(fp->out_ep, fp->out_reqv[i]);

	usb_free_all_descriptors(f);
	kfree(fp);
}

/*-------------------------------------------------------------------------*/

static struct net_device *dev;

int phonet_bind_config(struct usb_configuration *c)
{
	struct f_phonet *fp;
	int err, size, status;

	printk(KERN_DEBUG "phonet_bind_config");

// #ifdef CONFIG_USB_ANDROID_PHONET
	/* maybe allocate device-global string IDs */
	if (pn_string_defs[PN_CTRL_IDX].id == 0) {

		/* control interface label */
		status = usb_string_id(c->cdev);
		if (status < 0)
			return status;
		pn_string_defs[PN_CTRL_IDX].id = status;
		pn_control_intf_desc.iInterface = status;

		/* data interface label */
		status = usb_string_id(c->cdev);
		if (status < 0)
			return status;
		pn_string_defs[PN_DATA_IDX].id = status;
		pn_data_intf_desc.iInterface = status;

		/* IAD iFunction label */
		status = usb_string_id(c->cdev);
		if (status < 0)
			return status;
		pn_string_defs[PN_IAD_IDX].id = status;
		pn_iad_desc.iFunction = status;
	}
// #endif
	size = sizeof(*fp) + (phonet_rxq_size * sizeof(struct usb_request *));
	fp = kzalloc(size, GFP_KERNEL);
	if (!fp)
		return -ENOMEM;

	fp->dev = dev;
	fp->function.name = "phonet";
	fp->function.strings = pn_strings;
	fp->function.bind = pn_bind;
	fp->function.unbind = pn_unbind;
	fp->function.set_alt = pn_set_alt;
	fp->function.get_alt = pn_get_alt;
	fp->function.disable = pn_disconnect;
	spin_lock_init(&fp->rx.lock);

	err = usb_add_function(c, &fp->function);
	if (err)
		kfree(fp);
	return err;
}

int gphonet_setup(struct usb_gadget *gadget)
{
	struct phonet_port *port;
	int err;

	/* Create net device */
	BUG_ON(dev);
	dev = alloc_netdev(sizeof(*port), "upnlink%d", pn_net_setup);
	if (!dev)
		return -ENOMEM;

	port = netdev_priv(dev);
	spin_lock_init(&port->lock);
	netif_carrier_off(dev);
	SET_NETDEV_DEV(dev, &gadget->dev);

	err = register_netdev(dev);
	if (err) {
		free_netdev(dev);
		dev = NULL;
	}
	return err;
}

void gphonet_cleanup(void)
{
	if (dev) {
		unregister_netdev(dev);
		dev = NULL;
	}
}
