
/*
 *  linux/drivers/cpufreq/cpufreq_userspace.c
 *
 *  Copyright (C)  2001 Russell King
 *            (C)  2002 - 2004 Dominik Brodowski <linux@brodo.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/smp.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/cpufreq.h>
#include <linux/cpu.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/sysfs.h>
#include <linux/mutex.h>
#include <linux/suspend.h>

static DEFINE_PER_CPU(unsigned int, cpu_max_freq);
static DEFINE_PER_CPU(unsigned int, cpu_min_freq);
static DEFINE_PER_CPU(unsigned int, cpu_cur_freq); 
static DEFINE_PER_CPU(unsigned int, cpu_set_freq); 
static DEFINE_PER_CPU(unsigned int, cpu_pre_freq);
static DEFINE_PER_CPU(unsigned int, cpu_is_managed);

static DEFINE_MUTEX(userspace_mutex);
static int cpus_using_userspace_governor;

static int
userspace_cpufreq_notifier(struct notifier_block *nb, unsigned long val,
	void *data)
{
	struct cpufreq_freqs *freq = data;

	if (!per_cpu(cpu_is_managed, freq->cpu))
		return 0;

	if (val == CPUFREQ_POSTCHANGE) {
		pr_debug("saving cpu_cur_freq of cpu %u to be %u kHz\n",
				freq->cpu, freq->new);
		per_cpu(cpu_cur_freq, freq->cpu) = freq->new;
	}

	return 0;
}

static int user_cpufreq_pm_event(struct notifier_block *this,
                                unsigned long event, void *ptr)
{
	int cpu = 0;
	switch (event) {
	case PM_POST_HIBERNATION:
	case PM_POST_SUSPEND:
		for_each_present_cpu(cpu) {
			per_cpu(cpu_set_freq, cpu) = per_cpu(cpu_pre_freq, cpu);
			cpufreq_update_policy(cpu);
		}
	case PM_HIBERNATION_PREPARE:
	case PM_SUSPEND_PREPARE:
		for_each_present_cpu(cpu)
			per_cpu(cpu_pre_freq, cpu) = per_cpu(cpu_cur_freq, cpu);
	default:
		return NOTIFY_DONE;
        }
}

static struct notifier_block userspace_cpufreq_notifier_block = {
	.notifier_call  = userspace_cpufreq_notifier
};

static struct notifier_block __refdata user_cpufreq_pm_notifier = {
        .notifier_call = user_cpufreq_pm_event,
        .priority = -1
};


static int cpufreq_set(struct cpufreq_policy *policy, unsigned int freq)
{
	int ret = -EINVAL;

	pr_debug("cpufreq_set for cpu %u, freq %u kHz\n", policy->cpu, freq);

	mutex_lock(&userspace_mutex);
	if (!per_cpu(cpu_is_managed, policy->cpu))
		goto err;

	per_cpu(cpu_set_freq, policy->cpu) = freq;

	if (freq < per_cpu(cpu_min_freq, policy->cpu))
		freq = per_cpu(cpu_min_freq, policy->cpu);
	if (freq > per_cpu(cpu_max_freq, policy->cpu))
		freq = per_cpu(cpu_max_freq, policy->cpu);

	ret = __cpufreq_driver_target(policy, freq, CPUFREQ_RELATION_L);

 err:
	mutex_unlock(&userspace_mutex);
	return ret;
}


static ssize_t show_speed(struct cpufreq_policy *policy, char *buf)
{
	return sprintf(buf, "%u\n", per_cpu(cpu_cur_freq, policy->cpu));
}

static int cpufreq_governor_userspace(struct cpufreq_policy *policy,
				   unsigned int event)
{
	unsigned int cpu = policy->cpu;
	int rc = 0;

	switch (event) {
	case CPUFREQ_GOV_START:
		if (!cpu_online(cpu))
			return -EINVAL;
		BUG_ON(!policy->cur);
		mutex_lock(&userspace_mutex);

		if (cpus_using_userspace_governor == 0) {
			cpufreq_register_notifier(
					&userspace_cpufreq_notifier_block,
					CPUFREQ_TRANSITION_NOTIFIER);
			register_pm_notifier(&user_cpufreq_pm_notifier);
		}
		cpus_using_userspace_governor++;

		per_cpu(cpu_is_managed, cpu) = 1;
		per_cpu(cpu_min_freq, cpu) = policy->min;
		per_cpu(cpu_max_freq, cpu) = policy->max;
		per_cpu(cpu_cur_freq, cpu) = policy->cur;
		per_cpu(cpu_set_freq, cpu) = policy->cur;
		pr_debug("managing cpu %u started "
			"(%u - %u kHz, currently %u kHz)\n",
				cpu,
				per_cpu(cpu_min_freq, cpu),
				per_cpu(cpu_max_freq, cpu),
				per_cpu(cpu_cur_freq, cpu));

		mutex_unlock(&userspace_mutex);
		break;
	case CPUFREQ_GOV_STOP:
		mutex_lock(&userspace_mutex);
		cpus_using_userspace_governor--;
		if (cpus_using_userspace_governor == 0) {
			cpufreq_unregister_notifier(
					&userspace_cpufreq_notifier_block,
					CPUFREQ_TRANSITION_NOTIFIER);
			unregister_pm_notifier(&user_cpufreq_pm_notifier);
		}

		per_cpu(cpu_is_managed, cpu) = 0;
		per_cpu(cpu_min_freq, cpu) = 0;
		per_cpu(cpu_max_freq, cpu) = 0;
		per_cpu(cpu_set_freq, cpu) = 0;
		pr_debug("managing cpu %u stopped\n", cpu);
		mutex_unlock(&userspace_mutex);
		break;
	case CPUFREQ_GOV_LIMITS:
		mutex_lock(&userspace_mutex);
		pr_debug("limit event for cpu %u: %u - %u kHz, "
			"currently %u kHz, last set to %u kHz\n",
			cpu, policy->min, policy->max,
			per_cpu(cpu_cur_freq, cpu),
			per_cpu(cpu_set_freq, cpu));
		if (policy->max < per_cpu(cpu_set_freq, cpu)) {
			__cpufreq_driver_target(policy, policy->max,
						CPUFREQ_RELATION_H);
		} else if (policy->min > per_cpu(cpu_set_freq, cpu)) {
			__cpufreq_driver_target(policy, policy->min,
						CPUFREQ_RELATION_L);
		} else {
			__cpufreq_driver_target(policy,
						per_cpu(cpu_set_freq, cpu),
						CPUFREQ_RELATION_L);
		}
		per_cpu(cpu_min_freq, cpu) = policy->min;
		per_cpu(cpu_max_freq, cpu) = policy->max;
		per_cpu(cpu_cur_freq, cpu) = policy->cur;
		mutex_unlock(&userspace_mutex);
		break;
	}
	return rc;
}


#ifndef CONFIG_CPU_FREQ_DEFAULT_GOV_USERSPACE
static
#endif
struct cpufreq_governor cpufreq_gov_userspace = {
	.name		= "userspace",
	.governor	= cpufreq_governor_userspace,
	.store_setspeed	= cpufreq_set,
	.show_setspeed	= show_speed,
	.owner		= THIS_MODULE,
};

static int __init cpufreq_gov_userspace_init(void)
{
	return cpufreq_register_governor(&cpufreq_gov_userspace);
}


static void __exit cpufreq_gov_userspace_exit(void)
{
	cpufreq_unregister_governor(&cpufreq_gov_userspace);
}


MODULE_AUTHOR("Dominik Brodowski <linux@brodo.de>, "
		"Russell King <rmk@arm.linux.org.uk>");
MODULE_DESCRIPTION("CPUfreq policy governor 'userspace'");
MODULE_LICENSE("GPL");

#ifdef CONFIG_CPU_FREQ_DEFAULT_GOV_USERSPACE
fs_initcall(cpufreq_gov_userspace_init);
#else
module_init(cpufreq_gov_userspace_init);
#endif
module_exit(cpufreq_gov_userspace_exit);