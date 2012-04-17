#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include "sndptest_if.h"
#include "sndptest_core.h"
#define TEST
#define PROCNAME "sndptest"
#define MAX_TABLE 10
#define MAX_PATH 64
#define MAX_SIZE (1 << MAX_TABLE)
#define INPUT_MAX_LEN 2

#define LOOP_NONE		0
#define LOOP_AUDIOIC	1
#define LOOP_SPUV		2

static struct file *record_filp;
static struct file *play_filp;
static struct proc_dir_entry *sndp_parent;
static int loop_kind;
static int play_length;
static char rec_path[MAX_PATH];
static char play_path[MAX_PATH];
static char *rec_table;
static char *play_table;
static struct proc_dir_entry *record_entry;
static struct proc_dir_entry *play_entry;
static struct proc_dir_entry *loopback_audioic_entry;
static struct proc_dir_entry *loopback_spuv_entry;
static struct proc_dir_entry *recdata_entry;
static struct proc_dir_entry *playdata_entry;
#ifdef TEST
static struct proc_dir_entry *test_entry;
static unsigned int make_dummy_data(char *number);
static void read_playdata(char *number);
#endif  /* TEST */
static int write_file(unsigned int size);
static int read_file(void);

int sndptest_init(void);
void free_resouce(void);

void record_sound_callback(unsigned int size)
{
#ifdef TEST
	printk(KERN_WARNING "record_sound_callback(%d)\n", size);
#endif  /* TEST */
	write_file(size);
}

static int read_file(void)
{
	mm_segment_t fs;
	int nr_read = 0;
	int nr_read_sum = 0;
	struct inode *inode = NULL;
	int length;

	fs = get_fs();
	set_fs(get_ds());

	inode = play_filp->f_dentry->d_inode;
	length = i_size_read(inode->i_mapping->host);
	if ((PAGE_SIZE * MAX_SIZE) < length)
		length = (PAGE_SIZE * MAX_SIZE);
	play_length = length;
	do {
		nr_read = play_filp->f_op->read(play_filp, play_table,
									length, &play_filp->f_pos);
		nr_read_sum += nr_read;
	} while (nr_read_sum != length);
#ifdef TEST
	printk(KERN_WARNING "nr_read:%ld\n", (long)nr_read);
#endif  /* TEST */
/*
	play_table[length] = '\0';
*/
	set_fs(fs);
	filp_close(play_filp, NULL);
	play_filp = NULL;
	return 0;
}

static int write_file(unsigned int size)
{
	mm_segment_t fs;
	int nr_write = 0;
	int nr_write_sum = 0;

	fs = get_fs();
	set_fs(get_ds());
	if ((PAGE_SIZE * MAX_SIZE) < size)
		size = (PAGE_SIZE * MAX_SIZE);
	do {
		nr_write = record_filp->f_op->write(record_filp, rec_table,
											size, &record_filp->f_pos);
		nr_write_sum += nr_write;
	} while (nr_write_sum != size);
#ifdef TEST
	printk(KERN_WARNING "nr_write:%d\n", nr_write);
#endif  /* TEST */
	set_fs(fs);
	filp_close(record_filp, NULL);
	record_filp = NULL;
	return 0;
}

static int sndp_proc_record_write(struct file *filp, const char *buf,
									unsigned long len, void *data)
{
	char number[INPUT_MAX_LEN];
	unsigned long ret;

	if (len != INPUT_MAX_LEN) {
		printk(KERN_WARNING "proc_write len = %lu\n", len);
		return -ENOSPC;
	}
	ret = copy_from_user(number, buf, len);
	number[len-1] = '\0';

	record_filp = filp_open(rec_path, O_RDWR | O_CREAT | O_LARGEFILE | O_TRUNC,
								S_IRWXU | S_IRWXG | S_IRWXO);
	if (IS_ERR(record_filp)) {
		printk(KERN_ERR "can't create file:err %ld\n",
					PTR_ERR(record_filp));
		return PTR_ERR(record_filp);
	}
	if (!S_ISREG(record_filp->f_dentry->d_inode->i_mode))
		return -EACCES;

	switch (number[0]) {
	case '0':
		sound_rec_path_release();
		break;
	case '1':
		sound_rec_path_set(rec_table, PAGE_SIZE * MAX_SIZE);
		break;
	default:
		break;
	}

	return len;
}

static int sndp_proc_play_write(struct file *filp, const char *buf,
									unsigned long len, void *data)
{
	char number[INPUT_MAX_LEN];
	unsigned long ret;

	if (len != INPUT_MAX_LEN) {
		printk(KERN_WARNING "proc_write len = %lu\n", len);
		return -ENOSPC;
	}

	ret = copy_from_user(number, buf, len);
	number[len-1] = '\0';

	play_filp = filp_open(play_path, O_RDONLY | O_LARGEFILE, 0);
	if (IS_ERR(play_filp)) {
		printk(KERN_ERR "can't open file:err [%ld]\n", PTR_ERR(play_filp));
		return PTR_ERR(play_filp);
	}

	if (!S_ISREG(play_filp->f_dentry->d_inode->i_mode))
		return -EACCES;

	ret = copy_from_user(number, buf, len);
	number[len-1] = '\0';

	switch (number[0]) {
	case '0':
		sound_play_path_release();
		break;
	case '1':
		read_file();
		sound_play_path_set(play_table, play_length);
		break;
	default:
		break;
	}

	return len;
}

static int sndp_proc_loopback_audioic_write(struct file *filp, const char *buf,
												unsigned long len, void *data)
{
	char number[INPUT_MAX_LEN];
	unsigned long ret;

	if (len != INPUT_MAX_LEN) {
		printk(KERN_WARNING "proc_write len = %lu\n", len);
		return -ENOSPC;
	}

	ret = copy_from_user(number, buf, len);
	number[len-1] = '\0';

	switch (number[0]) {
	case '0':
		if (loop_kind == LOOP_AUDIOIC) {
			sound_loopback_audioic_stop();
			loop_kind = LOOP_NONE;
		}
		break;
	case '1':
		if (loop_kind == LOOP_NONE)
			sound_loopback_audioic_start();
		else if (loop_kind == LOOP_SPUV) {
			sound_loopback_spuv_stop();
			sound_loopback_audioic_start();
		}
		loop_kind = LOOP_AUDIOIC;
		break;
	default:
		break;
	}

	return len;
}

static int sndp_proc_loopback_spuv_write(struct file *filp, const char *buf,
											unsigned long len, void *data)
{
	char number[INPUT_MAX_LEN];
	unsigned long ret;

	if (len != INPUT_MAX_LEN) {
		printk(KERN_WARNING "proc_write len = %lu\n", len);
		return -ENOSPC;
	}

	ret = copy_from_user(number, buf, len);
	number[len-1] = '\0';

	switch (number[0]) {
	case '0':
		if (loop_kind == LOOP_SPUV) {
			sound_loopback_spuv_stop();
			loop_kind = LOOP_NONE;
		}
		break;
	case '1':
		if (loop_kind == LOOP_NONE) {
			sound_loopback_spuv_start();
		} else if (loop_kind == LOOP_AUDIOIC) {
			sound_loopback_audioic_stop();
			sound_loopback_spuv_start();
		}
		loop_kind  = LOOP_SPUV;
		break;
	default:
		break;
	}

	return len;
}

static int sndp_proc_recdata_write(struct file *filp, const char *buf,
									unsigned long len, void *data)
{
	unsigned long ret;

	if (len >= MAX_PATH) {
		printk(KERN_WARNING "proc_write len = %lu\n", len);
		return -ENOSPC;
	}
	ret = copy_from_user(rec_path, buf, len);
	rec_path[len-1] = '\0';

	return len;
}

static int sndp_proc_playdata_write(struct file *filp, const char *buf,
										unsigned long len, void *data)
{
	unsigned long ret;

	if (len >= MAX_PATH) {
		printk(KERN_WARNING "proc_write len = %lu\n", len);
		return -ENOSPC;
	}
	ret = copy_from_user(play_path, buf, len);
	play_path[len-1] = '\0';
	/* ret = read_file(buf, len); */

	return len;
}

#ifdef TEST
static int sndp_proc_test_write(struct file *filp, const char *buf,
									unsigned long len, void *data)
{
	unsigned long ret;

	int dummy_size;
	char number[INPUT_MAX_LEN];
	if (len != INPUT_MAX_LEN) {
		printk(KERN_WARNING "proc_write len = %lu\n", len);
		return -ENOSPC;
	}

	ret = copy_from_user(number, buf, len);
	number[len-1] = '\0';
	switch (number[0]) {
	case '0':
	case '1':
	case '2':
		dummy_size = make_dummy_data(number);
		record_sound_callback(dummy_size);
		break;
	case '3':
	case '4':
	case '5':
		read_playdata(number);
		break;
	default:
		break;
	}
	return len;
}

static unsigned int make_dummy_data(char *number)
{
	int i;
	int dummy_size = 4194304;

	switch (number[0]) {
	case '0':
		for (i = 0; i < dummy_size; i++)
			rec_table[i] = 'a';
		break;
	case '1':
		for (i = 0; i < dummy_size - 100; i++)
			rec_table[i] = 'a';
			dummy_size -= 100;
		break;
	case '2':
			dummy_size = 0;

		break;
	default:
		break;
	}

	return dummy_size;
}

static void read_playdata(char *number)
{
	int i;
	char data[305];

	if (number[0] == '3') {
		for (i = 0; i < 304; i++)
			data[i] = play_table[4194000 + i];
	} else if (number[0] == '4') {
		for (i = 0; i < 304; i++)
			data[i] = play_table[2000000 + i];
	} else if (number[0] == '5') {
		printk(KERN_WARNING "loff_t :[%d]\n", sizeof(loff_t));
		printk(KERN_WARNING "ssize_t :[%d]\n", sizeof(ssize_t));
		return;
	}

	printk(KERN_WARNING "read_playdata:[%s]\n", data);
}
#endif  /* TEST */

int sndptest_init(void)
{
	sndp_parent = proc_mkdir(PROCNAME, NULL);
	if (sndp_parent) {
		record_entry = create_proc_entry("record", (S_IFREG | S_IRUGO
													| S_IWUGO), sndp_parent);
		if (record_entry) {
			record_entry->write_proc = sndp_proc_record_write;
		} else {
			printk(KERN_ERR "create failed for rec path entry\n");
			goto proc_err;
		}

		play_entry  = create_proc_entry("play", (S_IFREG | S_IRUGO
											| S_IWUGO), sndp_parent);
		if (play_entry) {
			play_entry->write_proc = sndp_proc_play_write;
		} else {
			printk(KERN_ERR "create failed for play path entry\n");
			goto proc_err;
		}

		loopback_audioic_entry = create_proc_entry("loopback_audioic", (S_IFREG
														| S_IRUGO | S_IWUGO)
														, sndp_parent);
		if (loopback_audioic_entry) {
			loopback_audioic_entry->write_proc
				= sndp_proc_loopback_audioic_write;
		} else {
			printk(KERN_ERR "create failed for loopback max entry\n");
			goto proc_err;
		}

		loopback_spuv_entry = create_proc_entry("loopback_spuv",
													(S_IFREG | S_IRUGO
													| S_IWUGO), sndp_parent);
		if (loopback_spuv_entry) {
			loopback_spuv_entry->write_proc
				= sndp_proc_loopback_spuv_write;
		} else {
			printk(KERN_ERR "create failed for loopback spuv entry\n");
			goto proc_err;
		}

		recdata_entry = create_proc_entry("rec_data", (S_IFREG | S_IRUGO
											| S_IWUGO), sndp_parent);
		if (recdata_entry) {
			recdata_entry->write_proc = sndp_proc_recdata_write;
		} else {
			printk(KERN_ERR "create failed for recdata entry\n");
			goto proc_err;
		}

		playdata_entry = create_proc_entry("play_data", (S_IFREG | S_IRUGO
												| S_IWUGO), sndp_parent);
		if (playdata_entry) {
			playdata_entry->write_proc = sndp_proc_playdata_write;
		} else {
			printk(KERN_ERR "create failed for playdata entry\n");
			goto proc_err;
		}
#ifdef TEST
		test_entry = create_proc_entry("test", (S_IFREG | S_IRUGO
											| S_IWUGO), sndp_parent);
		if (test_entry) {
			test_entry->write_proc = sndp_proc_test_write;
		} else {
			printk(KERN_ERR "create failed for test entry\n");
			goto proc_err;
		}
#endif  /* TEST */
	} else {
		printk(KERN_ERR "create failed for proc parrent\n");
		goto proc_err;
	}
	rec_table = (char *)__get_free_pages(GFP_KERNEL, MAX_TABLE);
	if (!rec_table)
		goto alloc_err;
	play_table = (char *)__get_free_pages(GFP_KERNEL, MAX_TABLE);
	if (!play_table)
		goto alloc_err;
	return 0;

alloc_err:
proc_err:
	free_resouce();
	return -ENOMEM;
}

void free_resouce(void)
{
	if (rec_table)
		free_pages((unsigned int)rec_table, MAX_TABLE);
	if (play_table)
		free_pages((unsigned int)play_table, MAX_TABLE);
	if (record_entry)
		remove_proc_entry("record", sndp_parent);
	if (play_entry)
		remove_proc_entry("play", sndp_parent);
	if (loopback_audioic_entry)
		remove_proc_entry("loopback_audioic", sndp_parent);
	if (loopback_spuv_entry)
		remove_proc_entry("loopback_spuv", sndp_parent);
	if (recdata_entry)
		remove_proc_entry("rec_data", sndp_parent);
	if (playdata_entry)
		remove_proc_entry("play_data", sndp_parent);
#ifdef TEST
	if (test_entry)
		remove_proc_entry("test", sndp_parent);
#endif  /* TEST */
	if (sndp_parent)
		remove_proc_entry(PROCNAME, NULL);
}
