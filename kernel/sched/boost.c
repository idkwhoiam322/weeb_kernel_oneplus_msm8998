/* Copyright (c) 2012-2016, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Copyright (C) idkwhoiam322 <snkm1999@gmail.com>.
 *
 * The purpose of this has been modified to simply serve as stub 
 * functions to satisfy userspace's demands. Without this,
 * userspace spams a lot in logcat about not being able to write
 * anything.
 *
 */

#include "sched.h"

unsigned int sysctl_sched_boost;
static DEFINE_MUTEX(boost_mutex);

static bool verify_boost_params(int old_val, int new_val)
{
	/*
	 * Boost can only be turned on or off. There is no possiblity of
	 * switching from one boost type to another or to set the same
	 * kind of boost several times.
	 */
	return !(!!old_val == !!new_val);
}

static void _sched_set_boost(int old_val, int type)
{
	/* Do nothing */
	
	return;
}

int sched_set_boost(int type)
{
	int ret = 0;

	if (verify_boost_params(sysctl_sched_boost, type))
		_sched_set_boost(sysctl_sched_boost, type);
	else
		ret = -EINVAL;

	return ret;
}

int sched_boost_handler(struct ctl_table *table, int write,
		void __user *buffer, size_t *lenp,
		loff_t *ppos)
{
	int ret;
	unsigned int *data = (unsigned int *)table->data;
	unsigned int old_val;

	mutex_lock(&boost_mutex);

	old_val = *data;
	ret = proc_dointvec_minmax(table, write, buffer, lenp, ppos);

	if (ret || !write)
		goto done;

	if (verify_boost_params(old_val, *data)) {
		_sched_set_boost(old_val, *data);
	} else {
		*data = old_val;
		ret = -EINVAL;
	}

done:
	mutex_unlock(&boost_mutex);
	return ret;
}

int sched_boost(void)
{
	return sysctl_sched_boost;
}
