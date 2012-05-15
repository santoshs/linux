/*
 * pmdbg_suspend.c
 *
 * Copyright (C) 2011-2012 Renesas Mobile Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include "pmdbg_api.h"
#include <linux/suspend.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <mach/pm.h>
#include <linux/wakelock.h>

#ifdef CONFIG_MACH_U2EVM
extern int has_wake_lock_no_expire(int type);
#endif /*CONFIG_MACH_U2EVM*/

static int suspend_cmd(char*, int);
static int wakelock_cmd(char*, int);
static int suspend_init(void);
static void suspend_exit(void);

#ifdef CONFIG_SUSPEND
extern int suspend_devices_and_enter(suspend_state_t state);
#else /*!CONFIG_SUSPEND*/
static inline int suspend_devices_and_enter(suspend_state_t state)
{
	return 0;
}
#endif /*CONFIG_SUSPEND*/

LOCAL_DECLARE_MOD(suspend, suspend_init, suspend_exit);

DECLARE_CMD(suspend, suspend_cmd);
DECLARE_CMD(wakelock, wakelock_cmd);


/* suspend [type]
 * type:
 * - force: force suspend (skip early suspend) 
 * */
static int suspend_cmd(char *para, int size)
{
	int ret = 0;
	char item[PAR_SIZE];
	int para_sz = 0;
	int pos = 0;
	struct timeval beforeTime;
	struct timeval afterTime;
	int suspend_time;
	FUNC_MSG_IN;
	para = strim(para);
	ret = get_word(para, size, 0, item, &para_sz);
	pos = ret;
	if (para_sz > 0){
		ret = strncmp(item, "force", sizeof("force"));
		if (0 == ret){
			do_gettimeofday(&beforeTime);
			ret = suspend_devices_and_enter(PM_SUSPEND_MEM);
		}
		else{
			ret = -ENOTSUPP;
			MSG_INFO("No supported");
			goto fail;
		}
	}else{
		do_gettimeofday(&beforeTime);
		ret = pm_suspend(PM_SUSPEND_MEM);
	}
	do_gettimeofday(&afterTime);
	suspend_time = (afterTime.tv_sec - beforeTime.tv_sec) * 1000000
					+ (afterTime.tv_usec - beforeTime.tv_usec);
	
	MSG_INFO("Suspended in %12uus", suspend_time);
fail:
	FUNC_MSG_RET(ret);
}


/* wakelock
 * - no parameter: Display list of active wakelock to console
 * */
static int wakelock_cmd(char *para, int size)
{
	int ret = 0;
	FUNC_MSG_IN;

#ifdef CONFIG_MACH_U2EVM
	ret = has_wake_lock_no_expire(WAKE_LOCK_SUSPEND);
	if (ret == 0){
		MSG_INFO("No active suspend wakelock");
	}
#else /*!CONFIG_MACH_U2EVM*/
	ret = has_wake_lock(WAKE_LOCK_SUSPEND);
	if (ret == 0){
		MSG_INFO("No active suspend wakelock");
	}
#endif /*CONFIG_MACH_U2EVM*/
	
	FUNC_MSG_RET(0);
}


static int suspend_init(void)
{
	FUNC_MSG_IN;
	ADD_CMD(suspend, suspend);
	ADD_CMD(suspend, wakelock);

	FUNC_MSG_RET(0);
}

static void suspend_exit(void)
{
	FUNC_MSG_IN;
	DEL_CMD(suspend, suspend);
	DEL_CMD(suspend, wakelock);

	FUNC_MSG_OUT;
}



