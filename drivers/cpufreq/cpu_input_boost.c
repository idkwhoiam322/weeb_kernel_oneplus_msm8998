// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2019 Sultan Alsawaf <sultan@kerneltoast.com>.
 */

#define pr_fmt(fmt) "cpu_input_boost: " fmt

#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <linux/kthread.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/power_hal.h>

static unsigned int input_boost_freq_lp __read_mostly =
	CONFIG_INPUT_BOOST_FREQ_LP;
static unsigned int input_boost_freq_hp __read_mostly =
	CONFIG_INPUT_BOOST_FREQ_PERF;
static unsigned int boost_min_freq_lp __read_mostly =
	CONFIG_BASE_BOOST_FREQ_LP;
static unsigned int boost_min_freq_hp __read_mostly =
	CONFIG_BASE_BOOST_FREQ_PERF;
static unsigned int max_boost_freq_lp __read_mostly =
	CONFIG_MAX_BOOST_FREQ_LP;
static unsigned int max_boost_freq_hp __read_mostly =
	CONFIG_MAX_BOOST_FREQ_PERF;

static unsigned short powerhal_boost_duration __read_mostly =
	CONFIG_POWERHAL_BOOST_DURATION_MS;
static unsigned short input_boost_duration __read_mostly =
	CONFIG_INPUT_BOOST_DURATION_MS;
static unsigned short wake_boost_duration __read_mostly =
	CONFIG_WAKE_BOOST_DURATION_MS;

module_param(input_boost_freq_lp, uint, 0644);
module_param(input_boost_freq_hp, uint, 0644);
module_param_named(remove_input_boost_freq_lp,
	boost_min_freq_lp, uint, 0644);
module_param_named(remove_input_boost_freq_perf,
	boost_min_freq_hp, uint, 0644);
module_param(max_boost_freq_lp, uint, 0644);
module_param(max_boost_freq_hp, uint, 0644);

module_param(powerhal_boost_duration, short, 0644);
module_param(input_boost_duration, short, 0644);
module_param(wake_boost_duration, short, 0644);

enum {
	SCREEN_OFF,
	POWERHAL_BOOST,
	POWERHAL_MAX_BOOST,
	VIDEO_STREAMING_INPUT_EVENT,
	INPUT_BOOST,
	MAX_BOOST
};

struct boost_drv {
	struct delayed_work powerhal_unboost;
	struct delayed_work powerhal_max_unboost;
	struct delayed_work video_streaming_unboost;
	struct delayed_work input_unboost;
	struct delayed_work max_unboost;
	struct notifier_block cpu_notif;
	struct notifier_block fb_notif;
	wait_queue_head_t boost_waitq;
	atomic_long_t powerhal_max_boost_expires;
	atomic_long_t video_streaming_expires;
	atomic_long_t max_boost_expires;
	unsigned long state;
};

static void powerhal_unboost_worker(struct work_struct *work);
static void powerhal_max_unboost_worker(struct work_struct *work);
static void video_streaming_unboost_worker(struct work_struct *work);
static void input_unboost_worker(struct work_struct *work);
static void max_unboost_worker(struct work_struct *work);

static struct boost_drv boost_drv_g __read_mostly = {
	.powerhal_unboost = __DELAYED_WORK_INITIALIZER(boost_drv_g.powerhal_unboost,
							powerhal_unboost_worker, 0),
	.powerhal_max_unboost = __DELAYED_WORK_INITIALIZER(boost_drv_g.powerhal_max_unboost,
							powerhal_max_unboost_worker, 0),
	.video_streaming_unboost =
		__DELAYED_WORK_INITIALIZER(boost_drv_g.video_streaming_unboost,
							video_streaming_unboost_worker, 0),
	.input_unboost = __DELAYED_WORK_INITIALIZER(boost_drv_g.input_unboost,
						    input_unboost_worker, 0),
	.max_unboost = __DELAYED_WORK_INITIALIZER(boost_drv_g.max_unboost,
						  max_unboost_worker, 0),
	.boost_waitq = __WAIT_QUEUE_HEAD_INITIALIZER(boost_drv_g.boost_waitq)
};

static unsigned int get_input_boost_freq(struct cpufreq_policy *policy)
{
	unsigned int freq;

	if (cpumask_test_cpu(policy->cpu, cpu_lp_mask))
		freq = input_boost_freq_lp;
	else
		freq = input_boost_freq_hp;

	return min(freq, policy->max);
}

static unsigned int get_max_boost_freq(struct cpufreq_policy *policy)
{
	unsigned int freq;

	if (cpumask_test_cpu(policy->cpu, cpu_lp_mask))
		freq = max_boost_freq_lp;
	else
		freq = max_boost_freq_hp;

	return min(freq, policy->max);
}

static u32 get_min_freq(struct cpufreq_policy *policy)
{
	u32 freq;

	if (cpumask_test_cpu(policy->cpu, cpu_lp_mask))
		freq = boost_min_freq_lp;
	else
		freq = boost_min_freq_hp;

	return max(freq, policy->cpuinfo.min_freq);
}

static void update_online_cpu_policy(void)
{
	unsigned int cpu;

	/* Only one CPU from each cluster needs to be updated */
	get_online_cpus();
	cpu = cpumask_first_and(cpu_lp_mask, cpu_online_mask);
	cpufreq_update_policy(cpu);
	cpu = cpumask_first_and(cpu_perf_mask, cpu_online_mask);
	cpufreq_update_policy(cpu);
	put_online_cpus();
}

static void __powerhal_boost_kick(struct boost_drv *b)
{
	if (test_bit(SCREEN_OFF, &b->state))
		return;

	if (!powerhal_boost_duration)
		return;

	set_bit(POWERHAL_BOOST, &b->state);
	if (!mod_delayed_work(system_unbound_wq, &b->powerhal_unboost,
				msecs_to_jiffies(powerhal_boost_duration)))
		wake_up(&b->boost_waitq);
}

void powerhal_boost_kick(void)
{
	struct boost_drv *b = &boost_drv_g;

	__powerhal_boost_kick(b);
}

static void __powerhal_boost_kick_max(struct boost_drv *b,
				       unsigned int duration_ms)
{
	unsigned long boost_jiffies = msecs_to_jiffies(duration_ms);
	unsigned long curr_expires, new_expires;

	if (test_bit(SCREEN_OFF, &b->state))
		return;

	do {
		curr_expires = atomic_long_read(&b->powerhal_max_boost_expires);
		new_expires = jiffies + boost_jiffies;

		/* Skip this boost if there's a longer boost in effect */
		if (time_after(curr_expires, new_expires))
			return;
	} while (atomic_long_cmpxchg(&b->powerhal_max_boost_expires, curr_expires,
				     new_expires) != curr_expires);

	set_bit(POWERHAL_MAX_BOOST, &b->state);
	if (!mod_delayed_work(system_unbound_wq, &b->powerhal_max_unboost,
			      boost_jiffies))
		wake_up(&b->boost_waitq);
}

void powerhal_boost_kick_max(unsigned int duration_ms)
{
	struct boost_drv *b = &boost_drv_g;

	__powerhal_boost_kick_max(b, duration_ms);
}

static void __video_streaming_disable_schedtune(struct boost_drv *b)
{
	unsigned long boost_jiffies = msecs_to_jiffies(3500);
	unsigned long curr_expires, new_expires;

	if (test_bit(SCREEN_OFF, &b->state))
		return;

	/* Don't do anything if video is not playing */
	if (!video_streaming)
		return;

	do {
		curr_expires = atomic_long_read(&b->video_streaming_expires);
		new_expires = jiffies + boost_jiffies;

		/* Skip this boost if there's a longer boost in effect */
		if (time_after(curr_expires, new_expires))
			return;
	} while (atomic_long_cmpxchg(&b->video_streaming_expires, curr_expires,
				     new_expires) != curr_expires);

	set_bit(VIDEO_STREAMING_INPUT_EVENT, &b->state);
	if (!mod_delayed_work(system_unbound_wq, &b->video_streaming_unboost,
			      boost_jiffies))
		wake_up(&b->boost_waitq);
}

void video_streaming_disable_schedtune(void)
{
	struct boost_drv *b = &boost_drv_g;

	__video_streaming_disable_schedtune(b);
}

static void __cpu_input_boost_kick(struct boost_drv *b)
{
	if (test_bit(SCREEN_OFF, &b->state))
		return;

	if (!input_boost_duration)
		return;

	set_bit(INPUT_BOOST, &b->state);
	if (!mod_delayed_work(system_unbound_wq, &b->input_unboost,
			      msecs_to_jiffies(input_boost_duration)))
		wake_up(&b->boost_waitq);
}

void cpu_input_boost_kick(void)
{
	struct boost_drv *b = &boost_drv_g;

	__cpu_input_boost_kick(b);
}

static void __cpu_input_boost_kick_max(struct boost_drv *b,
				       unsigned int duration_ms)
{
	unsigned long boost_jiffies = msecs_to_jiffies(duration_ms);
	unsigned long curr_expires, new_expires;

	if (test_bit(SCREEN_OFF, &b->state))
		return;

	do {
		curr_expires = atomic_long_read(&b->max_boost_expires);
		new_expires = jiffies + boost_jiffies;

		/* Skip this boost if there's a longer boost in effect */
		if (time_after(curr_expires, new_expires))
			return;
	} while (atomic_long_cmpxchg(&b->max_boost_expires, curr_expires,
				     new_expires) != curr_expires);

	set_bit(MAX_BOOST, &b->state);
	if (!mod_delayed_work(system_unbound_wq, &b->max_unboost,
			      boost_jiffies))
		wake_up(&b->boost_waitq);
}

void cpu_input_boost_kick_max(unsigned int duration_ms)
{
	struct boost_drv *b = &boost_drv_g;

	__cpu_input_boost_kick_max(b, duration_ms);
}

static void powerhal_unboost_worker(struct work_struct *work)
{
	struct boost_drv *b = container_of(to_delayed_work(work),
					   typeof(*b), powerhal_unboost);

	clear_bit(POWERHAL_BOOST, &b->state);
	wake_up(&b->boost_waitq);
}

static void powerhal_max_unboost_worker(struct work_struct *work)
{
	struct boost_drv *b = container_of(to_delayed_work(work),
					   typeof(*b), powerhal_max_unboost);

	clear_bit(POWERHAL_MAX_BOOST, &b->state);
	wake_up(&b->boost_waitq);
}

static void video_streaming_unboost_worker(struct work_struct *work)
{
	struct boost_drv *b = container_of(to_delayed_work(work),
					   typeof(*b), video_streaming_unboost);

	clear_bit(VIDEO_STREAMING_INPUT_EVENT, &b->state);
	wake_up(&b->boost_waitq);
}

static void input_unboost_worker(struct work_struct *work)
{
	struct boost_drv *b = container_of(to_delayed_work(work),
					   typeof(*b), input_unboost);

	clear_bit(INPUT_BOOST, &b->state);
	wake_up(&b->boost_waitq);
}

static void max_unboost_worker(struct work_struct *work)
{
	struct boost_drv *b = container_of(to_delayed_work(work),
					   typeof(*b), max_unboost);

	clear_bit(MAX_BOOST, &b->state);
	wake_up(&b->boost_waitq);
}

static int cpu_boost_thread(void *data)
{
	static const struct sched_param sched_max_rt_prio = {
		.sched_priority = MAX_RT_PRIO - 1
	};
	struct boost_drv *b = data;
	unsigned long old_state = 0;

	sched_setscheduler_nocheck(current, SCHED_FIFO, &sched_max_rt_prio);

	while (1) {
		bool should_stop = false;
		unsigned long curr_state;

		wait_event(b->boost_waitq,
			(curr_state = READ_ONCE(b->state)) != old_state ||
			(should_stop = kthread_should_stop()));

		if (should_stop)
			break;

		old_state = curr_state;
		update_online_cpu_policy();
	}

	return 0;
}

static int cpu_notifier_cb(struct notifier_block *nb, unsigned long action,
			   void *data)
{
	struct boost_drv *b = container_of(nb, typeof(*b), cpu_notif);
	struct cpufreq_policy *policy = data;

	if (action != CPUFREQ_ADJUST)
		return NOTIFY_OK;

	/* Unboost when the screen is off */
	if (test_bit(SCREEN_OFF, &b->state)) {
		policy->min = get_min_freq(policy);
		disable_schedtune_boost(1);
		/* Enable EAS behaviour */
		energy_aware_enable = true;
		/* UFS unboost */
		set_ufshcd_clkgate_enable_status(1);
		/* CPUBW unboost */
		set_hyst_trigger_count_val(3);
		set_hist_memory_val(20);
		set_hyst_length_val(10);
		return NOTIFY_OK;
	}

	/* Boost CPU to max frequency for max boost */
	if (test_bit(MAX_BOOST, &b->state))
		policy->min = get_max_boost_freq(policy);

	/* Do powerhal boost for powerhal_max_boost */
	if (test_bit(POWERHAL_MAX_BOOST, &b->state)) {
		/* Disable EAS behaviour */
		energy_aware_enable = false;

		/* UFS boost */
		set_ufshcd_clkgate_enable_status(0);

		/* CPUBW boost */
		set_hyst_trigger_count_val(0);
		set_hist_memory_val(0);
		set_hyst_length_val(0);

		/* GPU boost */
		/* Enable KGSL_PWRFLAGS_POWER_ON */
		__force_on_store_ph(1, 0);
		/* Enable KGSL_PWRFLAGS_CLK_ON */
		__force_on_store_ph(1, 1);
		__timer_store_ph(10000, KGSL_PWR_IDLE_TIMER);
	} else {
		/* Enable EAS behaviour */
		energy_aware_enable = true;

		/* GPU unboost */
		/* Disable KGSL_PWRFLAGS_POWER_ON */
		__force_on_store_ph(0, 0);
		/* Disable KGSL_PWRFLAGS_CLK_ON */
		__force_on_store_ph(0, 1);
		__timer_store_ph(64, KGSL_PWR_IDLE_TIMER);
	}

	/* Put VIDEO_STREAMING_INPUT_EVENT check here to cover max_boost cases */
	if (test_bit(VIDEO_STREAMING_INPUT_EVENT, &b->state))
		disable_schedtune_boost(0);
	else if (video_streaming)
		disable_schedtune_boost(1);

	/* return early if being max bosted */
	if (test_bit(MAX_BOOST, &b->state) ||
		test_bit(POWERHAL_MAX_BOOST, &b->state))
		return NOTIFY_OK;

	/*
	 * Boost to policy->max if the boost frequency is higher. When
	 * unboosting, set policy->min to the absolute min freq for the CPU.
	 */
	if (test_bit(INPUT_BOOST, &b->state))
		policy->min = get_input_boost_freq(policy);
	else
		policy->min = get_min_freq(policy);

	if (test_bit(POWERHAL_BOOST, &b->state)) {
		/* UFS boost */
		set_ufshcd_clkgate_enable_status(0);

		/* CPUBW boost */
		set_hyst_trigger_count_val(0);
		set_hist_memory_val(0);
		set_hyst_length_val(0);
	} else {
		/* UFS boost */
		set_ufshcd_clkgate_enable_status(1);

		/* CPUBW unboost */
		set_hyst_trigger_count_val(3);
		set_hist_memory_val(20);
		set_hyst_length_val(10);
	}

	return NOTIFY_OK;
}

static int fb_notifier_cb(struct notifier_block *nb, unsigned long action,
			  void *data)
{
	struct boost_drv *b = container_of(nb, typeof(*b), fb_notif);
	int *blank = ((struct fb_event *)data)->data;

	/* Parse framebuffer blank events as soon as they occur */
	if (action != FB_EARLY_EVENT_BLANK)
		return NOTIFY_OK;

	/* Boost when the screen turns on and unboost when it turns off */
	if (*blank == FB_BLANK_UNBLANK) {
		clear_bit(SCREEN_OFF, &b->state);
		__cpu_input_boost_kick_max(b, wake_boost_duration);
		disable_schedtune_boost(0);
	} else {
		set_bit(SCREEN_OFF, &b->state);
		wake_up(&b->boost_waitq);
	}

	return NOTIFY_OK;
}

static void cpu_input_boost_input_event(struct input_handle *handle,
					unsigned int type, unsigned int code,
					int value)
{
	struct boost_drv *b = handle->handler->private;

	__video_streaming_disable_schedtune(b);

	__powerhal_boost_kick(b);

	__cpu_input_boost_kick(b);
}

static int cpu_input_boost_input_connect(struct input_handler *handler,
					 struct input_dev *dev,
					 const struct input_device_id *id)
{
	struct input_handle *handle;
	int ret;

	handle = kzalloc(sizeof(*handle), GFP_KERNEL);
	if (!handle)
		return -ENOMEM;

	handle->dev = dev;
	handle->handler = handler;
	handle->name = "cpu_input_boost_handle";

	ret = input_register_handle(handle);
	if (ret)
		goto free_handle;

	ret = input_open_device(handle);
	if (ret)
		goto unregister_handle;

	return 0;

unregister_handle:
	input_unregister_handle(handle);
free_handle:
	kfree(handle);
	return ret;
}

static void cpu_input_boost_input_disconnect(struct input_handle *handle)
{
	input_close_device(handle);
	input_unregister_handle(handle);
	kfree(handle);
}

static const struct input_device_id cpu_input_boost_ids[] = {
	/* Multi-touch touchscreen */
	{
		.flags = INPUT_DEVICE_ID_MATCH_EVBIT |
			INPUT_DEVICE_ID_MATCH_ABSBIT,
		.evbit = { BIT_MASK(EV_ABS) },
		.absbit = { [BIT_WORD(ABS_MT_POSITION_X)] =
			BIT_MASK(ABS_MT_POSITION_X) |
			BIT_MASK(ABS_MT_POSITION_Y) }
	},
	/* Touchpad */
	{
		.flags = INPUT_DEVICE_ID_MATCH_KEYBIT |
			INPUT_DEVICE_ID_MATCH_ABSBIT,
		.keybit = { [BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH) },
		.absbit = { [BIT_WORD(ABS_X)] =
			BIT_MASK(ABS_X) | BIT_MASK(ABS_Y) }
	},
	/* Keypad */
	{
		.flags = INPUT_DEVICE_ID_MATCH_EVBIT,
		.evbit = { BIT_MASK(EV_KEY) }
	},
	{ }
};

static struct input_handler cpu_input_boost_input_handler = {
	.event		= cpu_input_boost_input_event,
	.connect	= cpu_input_boost_input_connect,
	.disconnect	= cpu_input_boost_input_disconnect,
	.name		= "cpu_input_boost_handler",
	.id_table	= cpu_input_boost_ids
};

static int __init cpu_input_boost_init(void)
{
	struct boost_drv *b = &boost_drv_g;
	struct task_struct *thread;
	int ret;

	b->cpu_notif.notifier_call = cpu_notifier_cb;
	ret = cpufreq_register_notifier(&b->cpu_notif, CPUFREQ_POLICY_NOTIFIER);
	if (ret) {
		pr_err("Failed to register cpufreq notifier, err: %d\n", ret);
		return ret;
	}

	cpu_input_boost_input_handler.private = b;
	ret = input_register_handler(&cpu_input_boost_input_handler);
	if (ret) {
		pr_err("Failed to register input handler, err: %d\n", ret);
		goto unregister_cpu_notif;
	}

	b->fb_notif.notifier_call = fb_notifier_cb;
	b->fb_notif.priority = INT_MAX;
	ret = fb_register_client(&b->fb_notif);
	if (ret) {
		pr_err("Failed to register fb notifier, err: %d\n", ret);
		goto unregister_handler;
	}

	thread = kthread_run(cpu_boost_thread, b, "cpu_boostd");
	if (IS_ERR(thread)) {
		ret = PTR_ERR(thread);
		pr_err("Failed to start CPU boost thread, err: %d\n", ret);
		goto unregister_fb_notif;
	}

	return 0;

unregister_fb_notif:
	fb_unregister_client(&b->fb_notif);
unregister_handler:
	input_unregister_handler(&cpu_input_boost_input_handler);
unregister_cpu_notif:
	cpufreq_unregister_notifier(&b->cpu_notif, CPUFREQ_POLICY_NOTIFIER);
	return ret;
}
late_initcall(cpu_input_boost_init);
