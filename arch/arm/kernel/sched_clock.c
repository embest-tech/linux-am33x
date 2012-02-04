/*
 * sched_clock.c: support for extending counters to full 64-bit ns counter
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/clocksource.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/syscore_ops.h>

#include <asm/sched_clock.h>

static void sched_clock_poll(unsigned long wrap_ticks);
static DEFINE_TIMER(sched_clock_timer, sched_clock_poll, 0, 0);
static void (*sched_clock_update_fn)(void);
static struct clock_data *cdata;

static u32 notrace jiffy_sched_clock_read(void)
{
	return (u32)(jiffies - INITIAL_JIFFIES);
}

static void sched_clock_poll(unsigned long wrap_ticks)
{
	mod_timer(&sched_clock_timer, round_jiffies(jiffies + wrap_ticks));
	sched_clock_update_fn();
}

static u32 __read_mostly (*read_sched_clock)(void) = jiffy_sched_clock_read;

void __init setup_sched_clock_read(u32 (*read)(void))
{
	WARN_ON(read_sched_clock != jiffy_sched_clock_read);
	read_sched_clock = read;
}

void __init init_sched_clock(struct clock_data *cd, void (*update)(void),
	unsigned int clock_bits, unsigned long rate)
{
	unsigned long r, w;
	u64 res, wrap;
	char r_unit;

	sched_clock_update_fn = update;
	WARN_ON(cdata != NULL);
	cdata = cd;

	/* calculate the mult/shift to convert counter ticks to ns. */
	clocks_calc_mult_shift(&cd->mult, &cd->shift, rate, NSEC_PER_SEC, 0);

	r = rate;
	if (r >= 4000000) {
		r /= 1000000;
		r_unit = 'M';
	} else {
		r /= 1000;
		r_unit = 'k';
	}

	/* calculate how many ns until we wrap */
	wrap = cyc_to_ns((1ULL << clock_bits) - 1, cd->mult, cd->shift);
	do_div(wrap, NSEC_PER_MSEC);
	w = wrap;

	/* calculate the ns resolution of this counter */
	res = cyc_to_ns(1ULL, cd->mult, cd->shift);
	pr_info("sched_clock: %u bits at %lu%cHz, resolution %lluns, wraps every %lums\n",
		clock_bits, r, r_unit, res, w);

	/*
	 * Start the timer to keep sched_clock() properly updated and
	 * sets the initial epoch.
	 */
	sched_clock_timer.data = msecs_to_jiffies(w - (w / 10));
	update();

	/*
	 * Ensure that sched_clock() starts off at 0ns
	 */
	cd->epoch_ns = 0;
}

void __init sched_clock_postinit(void)
{
	sched_clock_poll(sched_clock_timer.data);
}

static int sched_clock_suspend(void)
{
	sched_clock_poll(sched_clock_timer.data);
	cdata->suspended = true;
	return 0;
}

static void sched_clock_resume(void)
{
	cdata->epoch_cyc = read_sched_clock();
	cdata->epoch_cyc_copy = cdata->epoch_cyc;
	cdata->suspended = false;
}

static struct syscore_ops sched_clock_ops = {
	.suspend = sched_clock_suspend,
	.resume = sched_clock_resume,
};

static int __init sched_clock_syscore_init(void)
{
	register_syscore_ops(&sched_clock_ops);
	return 0;
}
device_initcall(sched_clock_syscore_init);
