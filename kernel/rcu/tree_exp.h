/*
 * RCU expedited grace periods
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
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * Copyright IBM Corporation, 2016
 *
 * Authors: Paul E. McKenney <paulmck@linux.vnet.ibm.com>
 */

/* Wrapper functions for expedited grace periods.  */	
static void rcu_exp_gp_seq_start(struct rcu_state *rsp)	
{	
	rcu_seq_start(&rsp->expedited_sequence);	
}	
static void rcu_exp_gp_seq_end(struct rcu_state *rsp)	
{	
	rcu_seq_end(&rsp->expedited_sequence);	
	smp_mb(); /* Ensure that consecutive grace periods serialize. */	
}	
static unsigned long rcu_exp_gp_seq_snap(struct rcu_state *rsp)	
{	
	smp_mb(); /* Caller's modifications seen first by other CPUs. */	
	return rcu_seq_snap(&rsp->expedited_sequence);	
}	
static bool rcu_exp_gp_seq_done(struct rcu_state *rsp, unsigned long s)	
{	
	return rcu_seq_done(&rsp->expedited_sequence, s);	
}	
 /*	
 * Reset the ->expmaskinit values in the rcu_node tree to reflect any	
 * recent CPU-online activity.  Note that these masks are not cleared	
 * when CPUs go offline, so they reflect the union of all CPUs that have	
 * ever been online.  This means that this function normally takes its	
 * no-work-to-do fastpath.	
 */	
static void sync_exp_reset_tree_hotplug(struct rcu_state *rsp)	
{	
	bool done;	
	unsigned long flags;	
	unsigned long mask;	
	unsigned long oldmask;	
	int ncpus = READ_ONCE(rsp->ncpus);	
	struct rcu_node *rnp;	
	struct rcu_node *rnp_up;	
 	/* If no new CPUs onlined since last time, nothing to do. */	
	if (likely(ncpus == rsp->ncpus_snap))	
		return;	
	rsp->ncpus_snap = ncpus;	
 	/*	
	 * Each pass through the following loop propagates newly onlined	
	 * CPUs for the current rcu_node structure up the rcu_node tree.	
	 */	
	rcu_for_each_leaf_node(rsp, rnp) {	
		raw_spin_lock_irqsave(&rnp->lock, flags);	
		smp_mb__after_unlock_lock();	
		if (rnp->expmaskinit == rnp->expmaskinitnext) {	
			raw_spin_unlock_irqrestore(&rnp->lock, flags);	
			continue;  /* No new CPUs, nothing to do. */	
		}	
 		/* Update this node's mask, track old value for propagation. */	
		oldmask = rnp->expmaskinit;	
		rnp->expmaskinit = rnp->expmaskinitnext;	
		raw_spin_unlock_irqrestore(&rnp->lock, flags);	
 		/* If was already nonzero, nothing to propagate. */	
		if (oldmask)	
			continue;	
 		/* Propagate the new CPU up the tree. */	
		mask = rnp->grpmask;	
		rnp_up = rnp->parent;	
		done = false;	
		while (rnp_up) {	
			raw_spin_lock_irqsave(&rnp_up->lock, flags);	
			smp_mb__after_unlock_lock();	
			if (rnp_up->expmaskinit)	
				done = true;	
			rnp_up->expmaskinit |= mask;	
			raw_spin_unlock_irqrestore(&rnp_up->lock, flags);	
			if (done)	
				break;	
			mask = rnp_up->grpmask;	
			rnp_up = rnp_up->parent;	
		}	
	}	
}	
 /*	
 * Reset the ->expmask values in the rcu_node tree in preparation for	
 * a new expedited grace period.	
 */	
static void __maybe_unused sync_exp_reset_tree(struct rcu_state *rsp)	
{	
	unsigned long flags;	
	struct rcu_node *rnp;	
 	sync_exp_reset_tree_hotplug(rsp);	
	rcu_for_each_node_breadth_first(rsp, rnp) {	
		raw_spin_lock_irqsave(&rnp->lock, flags);	
		smp_mb__after_unlock_lock();	
		WARN_ON_ONCE(rnp->expmask);	
		rnp->expmask = rnp->expmaskinit;	
		raw_spin_unlock_irqrestore(&rnp->lock, flags);	
	}	
}	
 /*	
 * Return non-zero if there is no RCU expedited grace period in progress	
 * for the specified rcu_node structure, in other words, if all CPUs and	
 * tasks covered by the specified rcu_node structure have done their bit	
 * for the current expedited grace period.  Works only for preemptible	
 * RCU -- other RCU implementation use other means.	
 *	
 * Caller must hold the root rcu_node's exp_funnel_mutex.	
 */	
static int sync_rcu_preempt_exp_done(struct rcu_node *rnp)	
{	
	return rnp->exp_tasks == NULL &&	
	       READ_ONCE(rnp->expmask) == 0;	
}	
 /*	
 * Report the exit from RCU read-side critical section for the last task	
 * that queued itself during or before the current expedited preemptible-RCU	
 * grace period.  This event is reported either to the rcu_node structure on	
 * which the task was queued or to one of that rcu_node structure's ancestors,	
 * recursively up the tree.  (Calm down, calm down, we do the recursion	
 * iteratively!)	
 *	
 * Caller must hold the root rcu_node's exp_funnel_mutex and the	
 * specified rcu_node structure's ->lock.	
 */	
static void __rcu_report_exp_rnp(struct rcu_state *rsp, struct rcu_node *rnp,	
				 bool wake, unsigned long flags)	
	__releases(rnp->lock)	
{	
	unsigned long mask;	
 	for (;;) {	
		if (!sync_rcu_preempt_exp_done(rnp)) {	
			if (!rnp->expmask)	
				rcu_initiate_boost(rnp, flags);	
			else	
				raw_spin_unlock_irqrestore(&rnp->lock, flags);	
			break;	
		}	
		if (rnp->parent == NULL) {	
			raw_spin_unlock_irqrestore(&rnp->lock, flags);	
			if (wake) {	
				smp_mb(); /* EGP done before wake_up(). */	
				wake_up(&rsp->expedited_wq);	
			}	
			break;	
		}	
		mask = rnp->grpmask;	
		raw_spin_unlock(&rnp->lock); /* irqs remain disabled */	
		rnp = rnp->parent;	
		raw_spin_lock(&rnp->lock); /* irqs already disabled */	
		smp_mb__after_unlock_lock();	
		WARN_ON_ONCE(!(rnp->expmask & mask));	
		rnp->expmask &= ~mask;	
	}	
}	
 /*	
 * Report expedited quiescent state for specified node.  This is a	
 * lock-acquisition wrapper function for __rcu_report_exp_rnp().	
 *	
 * Caller must hold the root rcu_node's exp_funnel_mutex.	
 */	
static void __maybe_unused rcu_report_exp_rnp(struct rcu_state *rsp,	
					      struct rcu_node *rnp, bool wake)	
{	
	unsigned long flags;	
 	raw_spin_lock_irqsave(&rnp->lock, flags);	
	smp_mb__after_unlock_lock();	
	__rcu_report_exp_rnp(rsp, rnp, wake, flags);	
}	
 /*	
 * Report expedited quiescent state for multiple CPUs, all covered by the	
 * specified leaf rcu_node structure.  Caller must hold the root	
 * rcu_node's exp_funnel_mutex.	
 */	
static void rcu_report_exp_cpu_mult(struct rcu_state *rsp, struct rcu_node *rnp,	
				    unsigned long mask, bool wake)	
{	
	unsigned long flags;	
 	raw_spin_lock_irqsave(&rnp->lock, flags);	
	smp_mb__after_unlock_lock();	
	if (!(rnp->expmask & mask)) {	
		raw_spin_unlock_irqrestore(&rnp->lock, flags);	
		return;	
	}	
	rnp->expmask &= ~mask;	
	__rcu_report_exp_rnp(rsp, rnp, wake, flags); /* Releases rnp->lock. */	
}	
 /*	
 * Report expedited quiescent state for specified rcu_data (CPU).	
 * Caller must hold the root rcu_node's exp_funnel_mutex.	
 */	
static void rcu_report_exp_rdp(struct rcu_state *rsp, struct rcu_data *rdp,	
			       bool wake)	
{	
	rcu_report_exp_cpu_mult(rsp, rdp->mynode, rdp->grpmask, wake);	
}	
 /* Common code for synchronize_{rcu,sched}_expedited() work-done checking. */	
static bool sync_exp_work_done(struct rcu_state *rsp, struct rcu_node *rnp,	
			       struct rcu_data *rdp,	
			       atomic_long_t *stat, unsigned long s)	
{	
	if (rcu_exp_gp_seq_done(rsp, s)) {	
		if (rnp)	
			mutex_unlock(&rnp->exp_funnel_mutex);	
		else if (rdp)	
			mutex_unlock(&rdp->exp_funnel_mutex);	
		/* Ensure test happens before caller kfree(). */	
		smp_mb__before_atomic(); /* ^^^ */	
		atomic_long_inc(stat);	
		return true;	
	}	
	return false;	
}	
 /*	
 * Funnel-lock acquisition for expedited grace periods.  Returns a	
 * pointer to the root rcu_node structure, or NULL if some other	
 * task did the expedited grace period for us.	
 */	
static struct rcu_node *exp_funnel_lock(struct rcu_state *rsp, unsigned long s)	
{	
	struct rcu_data *rdp = per_cpu_ptr(rsp->rda, raw_smp_processor_id());	
	struct rcu_node *rnp0;	
	struct rcu_node *rnp1 = NULL;	
 	/*	
	 * First try directly acquiring the root lock in order to reduce	
	 * latency in the common case where expedited grace periods are	
	 * rare.  We check mutex_is_locked() to avoid pathological levels of	
	 * memory contention on ->exp_funnel_mutex in the heavy-load case.	
	 */	
	rnp0 = rcu_get_root(rsp);	
	if (!mutex_is_locked(&rnp0->exp_funnel_mutex)) {	
		if (mutex_trylock(&rnp0->exp_funnel_mutex)) {	
			if (sync_exp_work_done(rsp, rnp0, NULL,	
					       &rdp->expedited_workdone0, s))	
				return NULL;	
			return rnp0;	
		}	
	}	
 	/*	
	 * Each pass through the following loop works its way	
	 * up the rcu_node tree, returning if others have done the	
	 * work or otherwise falls through holding the root rnp's	
	 * ->exp_funnel_mutex.  The mapping from CPU to rcu_node structure	
	 * can be inexact, as it is just promoting locality and is not	
	 * strictly needed for correctness.	
	 */	
	if (sync_exp_work_done(rsp, NULL, NULL, &rdp->expedited_workdone1, s))	
		return NULL;	
	mutex_lock(&rdp->exp_funnel_mutex);	
	rnp0 = rdp->mynode;	
	for (; rnp0 != NULL; rnp0 = rnp0->parent) {	
		if (sync_exp_work_done(rsp, rnp1, rdp,	
				       &rdp->expedited_workdone2, s))	
			return NULL;	
		mutex_lock(&rnp0->exp_funnel_mutex);	
		if (rnp1)	
			mutex_unlock(&rnp1->exp_funnel_mutex);	
		else	
			mutex_unlock(&rdp->exp_funnel_mutex);	
		rnp1 = rnp0;	
	}	
	if (sync_exp_work_done(rsp, rnp1, rdp,	
			       &rdp->expedited_workdone3, s))	
		return NULL;	
	return rnp1;	
}	
 /* Invoked on each online non-idle CPU for expedited quiescent state. */	
static void sync_sched_exp_handler(void *data)	
{	
	struct rcu_data *rdp;	
	struct rcu_node *rnp;	
	struct rcu_state *rsp = data;	
 	rdp = this_cpu_ptr(rsp->rda);	
	rnp = rdp->mynode;	
	if (!(READ_ONCE(rnp->expmask) & rdp->grpmask) ||	
	    __this_cpu_read(rcu_sched_data.cpu_no_qs.b.exp))	
		return;	
	if (rcu_is_cpu_rrupt_from_idle()) {	
		rcu_report_exp_rdp(&rcu_sched_state,	
				   this_cpu_ptr(&rcu_sched_data), true);	
		return;	
	}	
	__this_cpu_write(rcu_sched_data.cpu_no_qs.b.exp, true);	
	resched_cpu(smp_processor_id());	
}	
 /* Send IPI for expedited cleanup if needed at end of CPU-hotplug operation. */	
static void sync_sched_exp_online_cleanup(int cpu)	
{	
	struct rcu_data *rdp;	
	int ret;	
	struct rcu_node *rnp;	
	struct rcu_state *rsp = &rcu_sched_state;	
 	rdp = per_cpu_ptr(rsp->rda, cpu);	
	rnp = rdp->mynode;	
	if (!(READ_ONCE(rnp->expmask) & rdp->grpmask))	
		return;	
	ret = smp_call_function_single(cpu, sync_sched_exp_handler, rsp, 0);	
	WARN_ON_ONCE(ret);	
}	
 /*	
 * Select the nodes that the upcoming expedited grace period needs	
 * to wait for.	
 */	
static void sync_rcu_exp_select_cpus(struct rcu_state *rsp,	
				     smp_call_func_t func)	
{	
	int cpu;	
	unsigned long flags;	
	unsigned long mask_ofl_test;	
	unsigned long mask_ofl_ipi;	
	int ret;	
	struct rcu_node *rnp;	
 	sync_exp_reset_tree(rsp);	
	rcu_for_each_leaf_node(rsp, rnp) {	
		raw_spin_lock_irqsave(&rnp->lock, flags);	
		smp_mb__after_unlock_lock();	
 		/* Each pass checks a CPU for identity, offline, and idle. */	
		mask_ofl_test = 0;	
		for_each_leaf_node_possible_cpu(rnp, cpu) {
			struct rcu_data *rdp = per_cpu_ptr(rsp->rda, cpu);	
			struct rcu_dynticks *rdtp = &per_cpu(rcu_dynticks, cpu);	
 			if (raw_smp_processor_id() == cpu ||	
			    !(atomic_add_return(0, &rdtp->dynticks) & 0x1))	
				mask_ofl_test |= rdp->grpmask;	
		}	
		mask_ofl_ipi = rnp->expmask & ~mask_ofl_test;	
 		/*	
		 * Need to wait for any blocked tasks as well.  Note that	
		 * additional blocking tasks will also block the expedited	
		 * GP until such time as the ->expmask bits are cleared.	
		 */	
		if (rcu_preempt_has_tasks(rnp))	
			rnp->exp_tasks = rnp->blkd_tasks.next;	
		raw_spin_unlock_irqrestore(&rnp->lock, flags);	
 		/* IPI the remaining CPUs for expedited quiescent state. */	
		for_each_leaf_node_possible_cpu(rnp, cpu) {
			unsigned long mask = leaf_node_cpu_bit(rnp, cpu);
			if (!(mask_ofl_ipi & mask))	
				continue;	
retry_ipi:	
			ret = smp_call_function_single(cpu, func, rsp, 0);	
			if (!ret) {	
				mask_ofl_ipi &= ~mask;	
			} else {	
				/* Failed, raced with offline. */	
				raw_spin_lock_irqsave(&rnp->lock, flags);	
				if (cpu_online(cpu) &&	
				    (rnp->expmask & mask)) {	
					raw_spin_unlock_irqrestore(&rnp->lock,	
								   flags);	
					schedule_timeout_uninterruptible(1);	
					if (cpu_online(cpu) &&	
					    (rnp->expmask & mask))	
						goto retry_ipi;	
					raw_spin_lock_irqsave(&rnp->lock,	
							      flags);	
				}	
				if (!(rnp->expmask & mask))	
					mask_ofl_ipi &= ~mask;	
				raw_spin_unlock_irqrestore(&rnp->lock, flags);	
			}	
		}	
		/* Report quiescent states for those that went offline. */	
		mask_ofl_test |= mask_ofl_ipi;	
		if (mask_ofl_test)	
			rcu_report_exp_cpu_mult(rsp, rnp, mask_ofl_test, false);	
	}	
}	
 static void synchronize_sched_expedited_wait(struct rcu_state *rsp)	
{	
	int cpu;	
	unsigned long jiffies_stall;	
	unsigned long jiffies_start;	
	unsigned long mask;	
	struct rcu_node *rnp;	
	struct rcu_node *rnp_root = rcu_get_root(rsp);	
	int ret;	
 	jiffies_stall = rcu_jiffies_till_stall_check();	
	jiffies_start = jiffies;	
 	for (;;) {	
		ret = wait_event_interruptible_timeout(	
				rsp->expedited_wq,	
				sync_rcu_preempt_exp_done(rnp_root),	
				jiffies_stall);	
		if (ret > 0)	
			return;	
		if (ret < 0) {	
			/* Hit a signal, disable CPU stall warnings. */	
			wait_event(rsp->expedited_wq,	
				   sync_rcu_preempt_exp_done(rnp_root));	
			return;	
		}	
		pr_err("INFO: %s detected expedited stalls on CPUs/tasks: {",	
		       rsp->name);	
		rcu_for_each_leaf_node(rsp, rnp) {	
			(void)rcu_print_task_exp_stall(rnp);	
			for_each_leaf_node_possible_cpu(rnp, cpu) {
				struct rcu_data *rdp;	
				mask = leaf_node_cpu_bit(rnp, cpu);
 				if (!(rnp->expmask & mask))	
					continue;	
				rdp = per_cpu_ptr(rsp->rda, cpu);	
				pr_cont(" %d-%c%c%c", cpu,	
					"O."[!!cpu_online(cpu)],	
					"o."[!!(rdp->grpmask & rnp->expmaskinit)],	
					"N."[!!(rdp->grpmask & rnp->expmaskinitnext)]);	
			}	
		}	
		pr_cont(" } %lu jiffies s: %lu\n",	
			jiffies - jiffies_start, rsp->expedited_sequence);	
		rcu_for_each_leaf_node(rsp, rnp) {	
			for_each_leaf_node_possible_cpu(rnp, cpu) {
				mask = leaf_node_cpu_bit(rnp, cpu);
				if (!(rnp->expmask & mask))	
					continue;	
				dump_cpu_task(cpu);	
			}	
		}	
		jiffies_stall = 3 * rcu_jiffies_till_stall_check() + 3;	
	}	
}	
 /**	
 * synchronize_sched_expedited - Brute-force RCU-sched grace period	
 *	
 * Wait for an RCU-sched grace period to elapse, but use a "big hammer"	
 * approach to force the grace period to end quickly.  This consumes	
 * significant time on all CPUs and is unfriendly to real-time workloads,	
 * so is thus not recommended for any sort of common-case code.  In fact,	
 * if you are using synchronize_sched_expedited() in a loop, please	
 * restructure your code to batch your updates, and then use a single	
 * synchronize_sched() instead.	
 *	
 * This implementation can be thought of as an application of sequence	
 * locking to expedited grace periods, but using the sequence counter to	
 * determine when someone else has already done the work instead of for	
 * retrying readers.	
 */	
void synchronize_sched_expedited(void)	
{	
	unsigned long s;	
	struct rcu_node *rnp;	
	struct rcu_state *rsp = &rcu_sched_state;	
 	/* If only one CPU, this is automatically a grace period. */	
	if (rcu_blocking_is_gp())	
		return;	
 	/* Take a snapshot of the sequence number.  */	
	s = rcu_exp_gp_seq_snap(rsp);	
 	rnp = exp_funnel_lock(rsp, s);	
	if (rnp == NULL)	
		return;  /* Someone else did our work for us. */	
 	rcu_exp_gp_seq_start(rsp);	
	sync_rcu_exp_select_cpus(rsp, sync_sched_exp_handler);	
	synchronize_sched_expedited_wait(rsp);	
 	rcu_exp_gp_seq_end(rsp);	
	mutex_unlock(&rnp->exp_funnel_mutex);	
}	
EXPORT_SYMBOL_GPL(synchronize_sched_expedited);

#ifdef CONFIG_PREEMPT_RCU

/*	
 * Remote handler for smp_call_function_single().  If there is an	
 * RCU read-side critical section in effect, request that the	
 * next rcu_read_unlock() record the quiescent state up the	
 * ->expmask fields in the rcu_node tree.  Otherwise, immediately	
 * report the quiescent state.	
 */	
static void sync_rcu_exp_handler(void *info)	
{	
	struct rcu_data *rdp;	
	struct rcu_state *rsp = info;	
	struct task_struct *t = current;	
 	/*	
	 * Within an RCU read-side critical section, request that the next	
	 * rcu_read_unlock() report.  Unless this RCU read-side critical	
	 * section has already blocked, in which case it is already set	
	 * up for the expedited grace period to wait on it.	
	 */	
	if (t->rcu_read_lock_nesting > 0 &&	
	    !t->rcu_read_unlock_special.b.blocked) {	
		t->rcu_read_unlock_special.b.exp_need_qs = true;	
		return;	
	}	
 	/*	
	 * We are either exiting an RCU read-side critical section (negative	
	 * values of t->rcu_read_lock_nesting) or are not in one at all	
	 * (zero value of t->rcu_read_lock_nesting).  Or we are in an RCU	
	 * read-side critical section that blocked before this expedited	
	 * grace period started.  Either way, we can immediately report	
	 * the quiescent state.	
	 */	
	rdp = this_cpu_ptr(rsp->rda);	
	rcu_report_exp_rdp(rsp, rdp, true);	
}	
 /**	
 * synchronize_rcu_expedited - Brute-force RCU grace period	
 *	
 * Wait for an RCU-preempt grace period, but expedite it.  The basic	
 * idea is to invoke synchronize_sched_expedited() to push all the tasks to	
 * the ->blkd_tasks lists and wait for this list to drain.  This consumes	
 * significant time on all CPUs and is unfriendly to real-time workloads,	
 * so is thus not recommended for any sort of common-case code.	
 * In fact, if you are using synchronize_rcu_expedited() in a loop,	
 * please restructure your code to batch your updates, and then Use a	
 * single synchronize_rcu() instead.	
 */	
void synchronize_rcu_expedited(void)	
{	
	struct rcu_node *rnp;	
	struct rcu_node *rnp_unlock;	
	struct rcu_state *rsp = rcu_state_p;	
	unsigned long s;	
 	s = rcu_exp_gp_seq_snap(rsp);	
 	rnp_unlock = exp_funnel_lock(rsp, s);	
	if (rnp_unlock == NULL)	
		return;  /* Someone else did our work for us. */	
 	rcu_exp_gp_seq_start(rsp);	
 	/* Initialize the rcu_node tree in preparation for the wait. */	
	sync_rcu_exp_select_cpus(rsp, sync_rcu_exp_handler);	
 	/* Wait for snapshotted ->blkd_tasks lists to drain. */	
	rnp = rcu_get_root(rsp);	
	synchronize_sched_expedited_wait(rsp);	
 	/* Clean up and exit. */	
	rcu_exp_gp_seq_end(rsp);	
	mutex_unlock(&rnp_unlock->exp_funnel_mutex);	
}	
EXPORT_SYMBOL_GPL(synchronize_rcu_expedited);

#else /* #ifdef CONFIG_PREEMPT_RCU */

/*
 * Wait for an rcu-preempt grace period, but make it happen quickly.
 * But because preemptible RCU does not exist, map to rcu-sched.
 */
void synchronize_rcu_expedited(void)
{
	synchronize_sched_expedited();
}
EXPORT_SYMBOL_GPL(synchronize_rcu_expedited);
 #endif /* #else #ifdef CONFIG_PREEMPT_RCU */
