/*
* File: ld_pnonet.c
*
* Phonet device TTY line discipline
*
* Copyright (C) 2011 Renesas Mobile Corporation. All rights reserved.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. ÂSee the GNU
* General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
* 02110-1301 USA
*
*/

#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/tty.h>

#include <asm/unaligned.h>
#include <net/sock.h>
#include <linux/errno.h>

#include <linux/if_arp.h>
#include <linux/if_phonet.h>
#include <linux/phonet.h>
#include <net/phonet/phonet.h>
#include <net/phonet/pn_dev.h>
#include <linux/switch.h> /* AT-ISI Separation */
#include <linux/tsu6712.h>

MODULE_AUTHOR("david RMC");
MODULE_DESCRIPTION("Phonet TTY line discipline");
MODULE_LICENSE("GPL");
MODULE_ALIAS_LDISC(N_PHONET);

#define SEND_QUEUE_LOW 10
#define SEND_QUEUE_HIGH 100
#define PHONET_SENDING	        1 /* Bit 1 = 0x02*/
#define PHONET_FLOW_OFF_SENT	4 /* Bit 4 = 0x10 */
#define MAX_WRITE_CHUNK	       8192
#define ISI_MSG_HEADER_SIZE 6
/*#define MAX_BUFF_SIZE 20000*/
#define MAX_BUFF_SIZE 65535

#define LD_PHONET_SWITCH	  4
#define LD_PHONET_NEW_ISI_MSG     0
#define LD_PHONET_ISI_MSG_LEN     1
#define LD_PHONET_ISI_MSG_NO_LEN  2
extern struct switch_dev switch_dock;

struct ld_phonet {
	struct tty_struct *tty;
	wait_queue_head_t wait;
	spinlock_t lock;
	unsigned long flags;
	struct sk_buff *skb;
	unsigned long len;
	unsigned long lentorcv;
	unsigned long datarcv ;
	unsigned long state;
	struct net_device *dev;
	struct list_head node;
	struct sk_buff_head head;
	char *tty_name;
	int  ld_phonet_state;
	int n_Data_Processed;
	int n_Data_Sent;
	int n_Remaining_Data;
	bool link_up;
	int nb_try_to_tx;
};

/* AT-ISI Separation starts */

#define ISI_CLOSED	101
#define AT_CLOSED	102
#define	SWITCH_AT	103
#define	SWITCH_ISI	104


struct switch_dev ld_pt_dev = {
	.name = "ldatisi",
	.state = 0
};


/* AT-ISI Separation ends */
#define LD_PHONET_DEBUG 0
#if LD_PHONET_DEBUG
	#define dbg(fmt, ...) printk(fmt,  ## __VA_ARGS__)
#else
	#define dbg(fmt, ...)
#endif


static ssize_t ld_show_manualsw(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return 0;
}

static ssize_t ld_set_manualsw1(struct device *dev,
			       struct device_attribute *attr,
				    const char *buf, size_t count)
{
	if (0 == strncmp(buf, "switch at", 9)) {
		dbg("SWITCH FOR ATATATATATATATATATATA\n");
		switch_set_state(&switch_dock, SWITCH_AT);		
  	}
	if (0 == strncmp(buf, "switch isi", 10))
		switch_set_state(&switch_dock, SWITCH_ISI);		
	return count;
}

static ssize_t ld_show_at_closed(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return 0;
}

static ssize_t ld_set_at_closed(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	struct tsu6712_usbsw *usbsw = dev_get_drvdata(dev);
	unsigned int value;
  
	if (0 == strncmp(buf, "at closed", 9))
		switch_set_state(&switch_dock, AT_CLOSED);		
  
	return count;
}

static ssize_t ld_show_isi_closed(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return 0;
}

static ssize_t ld_set_isi_closed(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	struct tsu6712_usbsw *usbsw = dev_get_drvdata(dev);
	unsigned int value;

	if (0 == strncmp(buf, "isi closed", 10))
		switch_set_state(&switch_dock, ISI_CLOSED);

	return count;
}

static DEVICE_ATTR(at_isi_switch, S_IRUGO | S_IWUSR,
		ld_show_manualsw, ld_set_manualsw1);

static DEVICE_ATTR(at_closed_ind, S_IRUGO | S_IWUSR,
		ld_show_at_closed, ld_set_at_closed);

static DEVICE_ATTR(isi_closed_ind, S_IRUGO | S_IWUSR,
		ld_show_isi_closed, ld_set_isi_closed);
		
static struct attribute *ld_attributes[] = {
	&dev_attr_at_isi_switch.attr,
	&dev_attr_at_closed_ind.attr,
	&dev_attr_isi_closed_ind.attr,
	NULL
};

static const struct attribute_group ld_group = {
	.attrs = ld_attributes,
};

/* AT-ISI Separation ends */

static int ld_pn_net_open(struct net_device *dev)
{
	netif_wake_queue(dev);
	return 0;
}

static int ld_pn_net_close(struct net_device *dev)
{
	netif_stop_queue(dev);
	return 0;
}

static int ld_pn_handle_tx(struct ld_phonet *ld_pn)
{
	struct tty_struct *tty = ld_pn->tty;
	struct sk_buff *skb;
	int tty_wr, len, room, i;

	if (tty == NULL)
		return 0;
	/* Enter critical section */
	if (test_and_set_bit(PHONET_SENDING, &ld_pn->state))
		return 0;

	/* skb_peek is safe because handle_tx is called after skb_queue_tail */
	while ((skb = skb_peek(&ld_pn->head)) != NULL) {

		/* Make sure you don't write too much */
		len = skb->len;
		room = tty_write_room(tty);

		if (!room) {
			if (ld_pn->nb_try_to_tx++ > 40) {
				ld_pn->link_up = false;
				/* Flush TX queue */
				while ((skb = \
				skb_dequeue(&ld_pn->head)) != NULL) {
					skb->dev->stats.tx_dropped++;
					dbg("Flush TX queue tx_dropped = %d", skb->dev->stats.tx_dropped);
					if (in_interrupt())
						dev_kfree_skb_irq(skb);
					else
						kfree_skb(skb);
				}
			}
			else { /* FALLBACK TRIAL */
				dbg("ld_pn_handle_tx no room, waiting for \
				previous to be sent..:\n");				
				
				if (!test_bit(TTY_DO_WRITE_WAKEUP, \
					 &tty->flags)) {
					/* wakeup bit is not set, set it */
					dbg("ld_pn_handle_tx Setting \
					TTY_DO_WRITE_WAKEUP bit...\n");
					set_bit(TTY_DO_WRITE_WAKEUP, \
						 &tty->flags);
				} else {
					dbg("ld_pn_handle_tx \
					TTY_DO_WRITE_WAKEUP bit already \
					set!...\n");
				}
			}
			break;
		}

		/* Get room => reset nb_try_to_tx counter */
		ld_pn->nb_try_to_tx = 0;

		if (room > MAX_WRITE_CHUNK)
			room = MAX_WRITE_CHUNK;
		if (len > room)
			len = room;


		tty_wr = tty->ops->write(tty, skb->data, len);
		ld_pn->dev->stats.tx_packets++;
		ld_pn->dev->stats.tx_bytes += tty_wr;
		/*dbg(" === data write in tty :\n");*/
		dbg(" Response start\n");
		for (i = 1; i <= len; i++) {
			dbg(" %02x", skb->data[i-1]);
              	if ((i%8) == 0)
				dbg("\n");
		}
		dbg("\n");
		dbg(" Response stop\n");
		/* Error on TTY ?! */
		if (tty_wr < 0)
			goto error;
		/* Reduce buffer written, and discard if empty */
		skb_pull(skb, tty_wr);
		if (skb->len == 0) {
			struct sk_buff *tmp = skb_dequeue(&ld_pn->head);
			BUG_ON(tmp != skb);
			if (in_interrupt())
				dev_kfree_skb_irq(skb);
			else
				kfree_skb(skb);
		}
	}
	/* Send flow off if queue is empty */
	clear_bit(PHONET_SENDING, &ld_pn->state);

	return 0;
error:
	clear_bit(PHONET_SENDING, &ld_pn->state);
	return tty_wr;
}



static int ld_pn_net_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct ld_phonet *ld_pn;
	u8 *ptr;

	BUG_ON(dev == NULL);
	ld_pn = netdev_priv(dev);

	ptr = skb_push(skb, 6);
	ptr[0] = 0xdd;
	ptr[1] = 0x7f;
	ptr[2] = 0x21;
	ptr[3] = 0x9a;
	ptr[4] = skb->data[10];
	ptr[5] = skb->data[11];

	if (ld_pn->link_up == true) {
		skb_queue_tail(&ld_pn->head, skb);
		return ld_pn_handle_tx(ld_pn);
	} else {
		if (tty_write_room(ld_pn->tty)) {
			/* link is up again */
			ld_pn->link_up = true;
			ld_pn->nb_try_to_tx = 0;

			skb_queue_tail(&ld_pn->head, skb);
			return ld_pn_handle_tx(ld_pn);
		} else {
			if (in_interrupt())
				dev_kfree_skb_irq(skb);
			else
				kfree_skb(skb);
			dev->stats.tx_dropped++;
			dbg("tx_dropped = %d", dev->stats.tx_dropped);
			return NETDEV_TX_OK;
			}
		}
}

static int
ld_pn_net_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	int ret = 0;
	switch (cmd) {
	case SIOCPNGAUTOCONF:
		ret = phonet_address_add(dev, PN_MEDIA_USB);
		if (ret)
			return ret;
		phonet_address_notify(RTM_NEWADDR, dev, PN_MEDIA_USB);
		phonet_route_add(dev, PN_DEV_PC);
		dev_open(dev);
		netif_carrier_on(dev);
		/* Return NOIOCTLCMD so Phonet won't do it again */
		return -ENOIOCTLCMD;
	}
	return -ENOIOCTLCMD;
}

static int ld_pn_net_mtu(struct net_device *dev, int new_mtu)
{
	if ((new_mtu < PHONET_MIN_MTU) || (new_mtu > PHONET_MAX_MTU))
		return -EINVAL;
	dev->mtu = new_mtu;
	return 0;
}



static const struct net_device_ops ld_pn_netdev_ops = {
	.ndo_open	= ld_pn_net_open,
	.ndo_stop	= ld_pn_net_close,
	.ndo_start_xmit	= ld_pn_net_xmit,
	.ndo_do_ioctl   = ld_pn_net_ioctl,
	.ndo_change_mtu	= ld_pn_net_mtu,
};





static void ld_pn_net_setup(struct net_device *dev)
{
	dev->features		= 0;
	dev->type		= ARPHRD_PHONET;
	dev->flags		= IFF_POINTOPOINT | IFF_NOARP;
	dev->mtu		= PHONET_DEV_MTU;
	dev->hard_header_len	= 1;
	dev->dev_addr[0]	= PN_MEDIA_USB;
	dev->addr_len		= 1;
	dev->tx_queue_len	= 5;

	dev->netdev_ops		= &ld_pn_netdev_ops;
	dev->destructor		= free_netdev;
	dev->header_ops		= &phonet_header_ops;
};


/*****************************************
*** TTY
******************************************/
static int ld_phonet_ldisc_open(struct tty_struct *tty)
{

	struct ld_phonet *ld_pn;
	struct net_device *dev;
	int err = 0;
	dbg("ld_phonet_ldisc_open starts\n");

	/* Create net device */
	dev = alloc_netdev(sizeof(*ld_pn), "upnlink%d", ld_pn_net_setup);
	if (!dev)
		return -ENOMEM;

	ld_pn = netdev_priv(dev);
	spin_lock_init(&ld_pn->lock);
	netif_carrier_off(dev);
	skb_queue_head_init(&ld_pn->head);
	ld_pn->tty = tty;
	tty->disc_data = ld_pn;
	tty->receive_room = 65536;
	ld_pn->dev = dev;
	ld_pn->skb = NULL;
	ld_pn->len = 0;
	ld_pn->lentorcv = 0;
	ld_pn->datarcv = 0 ;
	ld_pn->ld_phonet_state = LD_PHONET_NEW_ISI_MSG;
	ld_pn->n_Data_Processed = 0;
	ld_pn->n_Data_Sent = 0;
	ld_pn->n_Remaining_Data = 0;
	ld_pn->link_up = true;
	ld_pn->nb_try_to_tx = 0;

	err = register_netdev(dev);

	if (err)
		free_netdev(dev);

	dbg("ld_phonet_ldisc_open exits err = %d\n", err);
	return err;

}



static void ld_phonet_ldisc_close(struct tty_struct *tty)
{
	struct ld_phonet *ld_pn = tty->disc_data;

	tty->disc_data = NULL;
	ld_pn->tty = NULL;
	unregister_netdev(ld_pn->dev);
	/*free_netdev(ld_pn->dev); David a checker*/
}

static void ld_phonet_ldisc_initiate_transfer \
(struct ld_phonet *ld_pn, const unsigned char *cp, int count)
{

	struct sk_buff *skb = NULL;
	unsigned int msglen = 0;

	struct phonethdr *ph = NULL;
	int x, i;

	dbg("ld_phonet: initiate transfer Data Sent = %d ", \
	ld_pn->n_Data_Sent);
	dbg("Data Processed = %d ", ld_pn->n_Data_Processed);
	dbg("Data Remaining = %d\n", ld_pn->n_Remaining_Data);

	/* Check if there is still data in cp */
	while (ld_pn->n_Data_Processed < count) {
		/* Check if extract length is possible */
		if ((count - ld_pn->n_Data_Processed) > ISI_MSG_HEADER_SIZE) {
			/* Extract length */
			/* Move 1 byte since media parameter is not there in 
			phonethdr structure */
			ph = (struct phonethdr *) \
			(cp + ld_pn->n_Data_Processed + sizeof(char));
			msglen = get_unaligned_be16(&ph->pn_length);
			ld_pn->len = msglen + ISI_MSG_HEADER_SIZE;

			/* Alloc SKBuff */
			skb = netdev_alloc_skb(ld_pn->dev, ld_pn->len);
			if (NULL == skb) {
				/* TBD handle error */
				return;
			}

			skb->dev = ld_pn->dev;
			skb->protocol = htons(ETH_P_PHONET);
			skb_reset_mac_header(skb);
			ld_pn->skb = skb;

			/* check if we receive complete data in this 
			usb frame */
			if (ld_pn->len <= (count - ld_pn->n_Data_Processed)) {
				/* We received complete data in this usb 
				frame */
				/* copy the ISI buffer */
				memcpy(skb_put(skb, ld_pn->len), \
				cp + ld_pn->n_Data_Processed, ld_pn->len);
				ld_pn->n_Data_Processed += ld_pn->len;

				/* Send to Phonet */
				ld_pn->dev->stats.rx_packets++;
				ld_pn->dev->stats.rx_bytes += skb->len;
				__skb_pull(skb, 1);
				dbg("Request buffer start\n");
				for (i = 1; i <= skb->len; i++) {
					dbg("%02x", skb->data[i-1]);
					if (i%8 == 0)
						dbg("\n");
				}
		
				dbg("Request buffer end\n");
				dbg("calling netif_rx inside \
				initiate_transfer ld_pn->len=%d\n", \
				ld_pn->len);
				netif_rx(skb);
				ld_pn->n_Data_Sent += ld_pn->len;

				/* TBD : Reset pointers */
			} else {
				/* We receive only partial ISI message */
				/* Copy the partial ISI message */
				memcpy(skb_put(skb, count - \
				ld_pn->n_Data_Processed), cp + \
				ld_pn->n_Data_Processed, count - \
				ld_pn->n_Data_Processed);
				ld_pn->ld_phonet_state = LD_PHONET_ISI_MSG_LEN;
				ld_pn->n_Remaining_Data = ld_pn->len - \
				(count - ld_pn->n_Data_Processed);
				ld_pn->n_Data_Processed += count - \
				ld_pn->n_Data_Processed;

				return;
			}
		} else {
			/* Not able to extract length since received
				 usb frame length is
				less than ISI message header size */

			/* Alloc SKBuff with max size */
			skb = netdev_alloc_skb(ld_pn->dev, MAX_BUFF_SIZE);
			if (NULL == skb) {
				/* TBD handle error */
				return;
			}

			skb->dev = ld_pn->dev;
			skb->protocol = htons(ETH_P_PHONET);
			skb_reset_mac_header(skb);
			ld_pn->skb = skb;

			/* Copy available data */
			memcpy(skb_put(skb, count - ld_pn->n_Data_Processed), \
			cp + ld_pn->n_Data_Processed, count - \
			ld_pn->n_Data_Processed);
			ld_pn->ld_phonet_state = LD_PHONET_ISI_MSG_NO_LEN;
			
			ld_pn->len += count - ld_pn->n_Data_Processed;
			ld_pn->n_Data_Processed += \ 
			count - ld_pn->n_Data_Processed;

			return;
		}
	}
	/* No more data in cp */
	ld_pn->ld_phonet_state = LD_PHONET_NEW_ISI_MSG;
	ld_pn->len = 0;
	ld_pn->n_Data_Processed = 0;
	ld_pn->n_Data_Sent = 0;
	ld_pn->n_Remaining_Data = 0;

	return;
}

/* AT-ISI Message Separation Starts */

extern ssize_t ld_set_manualsw(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf, size_t count);

int stop_isi;

/* AT-ISI Message Separation Ends */
static void ld_phonet_ldisc_receive
(struct tty_struct *tty, const unsigned char *cp, char *fp, int count)
{
	struct ld_phonet *ld_pn = tty->disc_data;
	struct sk_buff *skb = ld_pn->skb;
	unsigned long flags = 0;
	unsigned int msglen = 0, i;
	int check_at = 27;
	struct phonethdr *ph = NULL;

	if (ld_pn->link_up == false) {
		/* data received from PC => can TX */
		ld_pn->link_up = true;

		ld_pn->nb_try_to_tx = 0;
	}

	spin_lock_irqsave(&ld_pn->lock, flags);

	/*Whenever you receive a new USB frame Data Processed should be reset*/
	ld_pn->n_Data_Processed = 0;

	while (1) {
		switch (ld_pn->ld_phonet_state) {
		case LD_PHONET_SWITCH:
		{
			int ret = 0;
			dbg("case LD_PHONET_SWITCH\n");
			ret = ld_set_switch_buf(NULL, NULL, cp, count);
			if (-1 == ret) {
				dbg("MATCH FOR change mode \
				LD_PHONET_SWITCH%c\n", *cp);
				ld_pn->ld_phonet_state = LD_PHONET_NEW_ISI_MSG;
			}
			break;
		}
		case LD_PHONET_NEW_ISI_MSG:
		{
			int first_byte = 0;
			if (count >= 1) {
				if (*cp) {
	                		first_byte = *cp;	
					dbg("case LD_PHONET_NEW_ISI_MSG: \
					%d\n", *cp);
				}
			} else
				dbg("case LD_PHONET_NEW_ISI_MSG\n");
		
			if ((count >= 1) && (first_byte != check_at)) {
				dbg("MATCH FOR change mode %c\n", *cp);
				ld_pn->ld_phonet_state = LD_PHONET_SWITCH;
				continue;
			}

			/* AT-ISI Message Separation Ends */
			ld_phonet_ldisc_initiate_transfer(ld_pn, cp, count);
			break;
		}
		case LD_PHONET_ISI_MSG_LEN:
			/* check if Remaining Data is complete */
			if (ld_pn->n_Remaining_Data > count) {
				/* We dont receive complete data */
				/* Copy the available data */
				memcpy(skb_put(skb, count), cp + \
				ld_pn->n_Data_Processed, count);
				ld_pn->n_Data_Processed += count;
				ld_pn->ld_phonet_state = LD_PHONET_ISI_MSG_LEN;
				ld_pn->n_Remaining_Data -= count;
			} else {
				/* We have complete data available */
				/* Copy remaining data */
				memcpy(skb_put(skb, ld_pn->n_Remaining_Data), \
				cp + ld_pn->n_Data_Processed, \
				ld_pn->n_Remaining_Data);
				/* Send to Phonet */
				ld_pn->dev->stats.rx_packets++;
				ld_pn->dev->stats.rx_bytes += skb->len;
				__skb_pull(skb, sizeof(char));
				dbg("Request buffer start\n");
				for (i = 1; i <= skb->len; i++) {
					dbg("%02x", skb->data[i-1]);
					if (i%8 == 0)
						dbg("\n");
				}
				dbg("Request buffer end\n");
				dbg("calling netif_rx inside ldisc_receive \
				first ld_pn->len=%d\n", ld_pn->len);
				netif_rx(skb);
				ld_pn->n_Data_Sent += ld_pn->len;

				/* TBD : Update pointers */
				ld_pn->n_Data_Sent += ld_pn->n_Remaining_Data;
				ld_pn->n_Data_Processed += \
				ld_pn->n_Remaining_Data;

				/* Initiate a new ISI transfer */
				ld_phonet_ldisc_initiate_transfer\
				(ld_pn, cp, count);
			}
			break;

		case LD_PHONET_ISI_MSG_NO_LEN:
			/*Check if we can extact length */
			if ((ld_pn->len + count) >= ISI_MSG_HEADER_SIZE) {

				/* Copy remaining header to SKBuff to extract 
				length */
				memcpy(skb_put(skb, ISI_MSG_HEADER_SIZE - \
				ld_pn->len), cp + ld_pn->n_Data_Processed, \
				ISI_MSG_HEADER_SIZE - ld_pn->len);
				ph = (struct phonethdr *) \
				(skb->data + sizeof(char));
				msglen = get_unaligned_be16(&ph->pn_length);

				ld_pn->n_Data_Processed += \
				ISI_MSG_HEADER_SIZE - ld_pn->len;

				/* Check if we receive complete data */
				if ((count + ld_pn->len) < \
				(msglen + ISI_MSG_HEADER_SIZE)) {
					/* We have not received complete data */
					/* Copy available data */
					memcpy(skb_put(skb, count - \
					(ISI_MSG_HEADER_SIZE - ld_pn->len)), \
					cp + ld_pn->n_Data_Processed, count - \
					(ISI_MSG_HEADER_SIZE - ld_pn->len));
					ld_pn->ld_phonet_state = \
					LD_PHONET_ISI_MSG_LEN;
					ld_pn->n_Remaining_Data = (msglen + \
					ISI_MSG_HEADER_SIZE) - \
					(count + ld_pn->len);
					ld_pn->n_Data_Processed += count - \
					(ISI_MSG_HEADER_SIZE - ld_pn->len);

					/* Reset pointers */
					ld_pn->len = msglen + \
					ISI_MSG_HEADER_SIZE;

					/* return; */
					break;
				} else {
					/* We receive complete data */
					/* Copy remaining data */
					memcpy(skb_put(skb, (msglen + \
					ISI_MSG_HEADER_SIZE) - (ld_pn->len +  \
					ld_pn->n_Data_Processed)), cp + \
					ld_pn->n_Data_Processed, (msglen + \
					ISI_MSG_HEADER_SIZE) - (ld_pn->len + \
					ld_pn->n_Data_Processed));

					/* Send to Phonet */
					ld_pn->dev->stats.rx_packets++;
					ld_pn->dev->stats.rx_bytes += skb->len;
					__skb_pull(skb, sizeof(char));
					dbg("Request buffer start\n");
					for (i = 1; i <= skb->len; i++) {
						dbg("%02x", skb->data[i-1]);
						if (i%8 == 0)
							dbg("\n");
					}
	
					dbg("Request buffer end\n");
					dbg("calling netif_rx inside \
					ldisc_receive second ld_pn->len= \
					%d\n", ld_pn->len);
					netif_rx(skb);

					ld_pn->n_Data_Sent += (msglen + \
					ISI_MSG_HEADER_SIZE) - (ld_pn->len + \
					ld_pn->n_Data_Processed);

					ld_pn->n_Data_Processed += (msglen + \
					ISI_MSG_HEADER_SIZE) - (ld_pn->len + \
					ld_pn->n_Data_Processed);

					/* Check if we still have data in cp */
					if (count > ld_pn->n_Data_Processed) {
						/* We still have data in cp */
						/* Initiate new ISI transfer */
						ld_phonet_ldisc_initiate_transfer\
						(ld_pn, cp, count);
					} else {
						/* No more data in cp */
						ld_pn->ld_phonet_state = \
						LD_PHONET_NEW_ISI_MSG;

						/* Reset pointers */
						ld_pn->len = 0;
						ld_pn->n_Data_Processed = 0;
						ld_pn->n_Data_Sent = 0;
						ld_pn->n_Remaining_Data = 0;
					}
				}
			} else {
				/* Cannot extract length */
				/* Copy available data */
				memcpy(skb_put(skb, count), cp + \
				ld_pn->n_Data_Processed, count);
				ld_pn->len += count;
				ld_pn->ld_phonet_state = \
				LD_PHONET_ISI_MSG_NO_LEN;
				ld_pn->n_Data_Processed += count;
			}
			break;

		default:
			break;
		}
		break;
	}

	spin_unlock_irqrestore(&ld_pn->lock, flags);
}

static void ld_phonet_ldisc_write_wakeup(struct tty_struct *tty)
{
	clear_bit(TTY_DO_WRITE_WAKEUP, &tty->flags);
	struct ld_phonet *ld_pn;
	ld_pn = tty->disc_data;
	if (ld_pn == NULL)
		return;
	BUG_ON(ld_pn == NULL);
	BUG_ON(ld_pn->tty != tty);
	ld_pn_handle_tx(ld_pn);
}

static struct tty_ldisc_ops ld_phonet_ldisc = {
	.owner =	THIS_MODULE,
	.name =		"phonet",
	.open =		ld_phonet_ldisc_open,
	.close =	ld_phonet_ldisc_close,
	.receive_buf =	ld_phonet_ldisc_receive,
	.write_wakeup =	ld_phonet_ldisc_write_wakeup
};

/*
 * The functions for insering/removing us as a module.
 */

static int __init ld_phonet_init(void)
{
	int retval;
	retval = tty_register_ldisc(N_PHONET, &ld_phonet_ldisc);
	switch_dev_register(&ld_pt_dev);
	return  retval;
}

static void __exit ld_phonet_exit(void)
{
	tty_unregister_ldisc(N_PHONET);
	/* AT-ISI Separation */
	/* sysfs_remove_group(&client->dev.kobj, &ld_group); */
	switch_dev_unregister(&ld_pt_dev);
}




module_init(ld_phonet_init);
module_exit(ld_phonet_exit);
