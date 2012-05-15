/*
 * kernel/power/pm_test.c
 *
 * Copyright (C) 2012 Renesas Mobile Corporation
 *
 */

#include "power.h"
#include <linux/module.h>

/* 
 * Offer files to turn on/off below functions
 *   Serial console 
 *     0: no_console_suspend, 1:console_suspend
 *   Wakelock checking 
 *     0: don't ignore wakelocks, 1: ignore wakelocks
 *   Call late resume immediately after resume
 *     0: don't call late resume
 *     1: call late resume
 *   Set wait time before resuming (ms)
 */
extern int console_suspend_enabled;
int ignore_wakelock;
EXPORT_SYMBOL(ignore_wakelock);
int for_kernel_test;
EXPORT_SYMBOL(for_kernel_test);
int wait_time;
EXPORT_SYMBOL(wait_time);

/* Initialize */
ignore_wakelock = 0;
for_kernel_test = 0;
wait_time = 5;		/* default is 5ms */

module_param_named(console_suspend, console_suspend_enabled, int, \
					S_IRUGO | S_IWUSR | S_IWGRP);
module_param_named(ignore_wakelocks, ignore_wakelock, int, \
					S_IRUGO | S_IWUSR | S_IWGRP);
module_param_named(for_kernel_test, for_kernel_test, int, \
					S_IRUGO | S_IWUSR | S_IWGRP);
module_param_named(wait_time, wait_time, int, \
					S_IRUGO | S_IWUSR | S_IWGRP);
