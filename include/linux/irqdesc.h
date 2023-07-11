#ifndef _LINUX_IRQDESC_H
#define _LINUX_IRQDESC_H

/*
 * Core internal functions to deal with irq descriptors
 *
 * This include will move to kernel/irq once we cleaned up the tree.
 * For now it's included from <linux/irq.h>
 */

struct irq_affinity_notify;
struct proc_dir_entry;
struct module;
struct irq_desc;
struct irq_domain;
struct pt_regs;

/**
 * struct irq_desc - interrupt descriptor
 * @irq_common_data:	per irq and chip data passed down to chip functions
 * @kstat_irqs:		irq stats per cpu
 * @handle_irq:		highlevel irq-events handler
 * @preflow_handler:	handler called before the flow handler (currently used by sparc)
 * @action:		the irq action chain
 * @status:		status information
 * @core_internal_state__do_not_mess_with_it: core internal status information
 * @depth:		disable-depth, for nested irq_disable() calls
 * @wake_depth:		enable depth, for multiple irq_set_irq_wake() callers
 * @irq_count:		stats field to detect stalled irqs
 * @last_unhandled:	aging timer for unhandled count
 * @irqs_unhandled:	stats field for spurious unhandled interrupts
 * @threads_handled:	stats field for deferred spurious detection of threaded handlers
 * @threads_handled_last: comparator field for deferred spurious detection of theraded handlers
 * @lock:		locking for SMP
 * @affinity_hint:	hint to user space for preferred irq affinity
 * @affinity_notify:	context for notification of affinity changes
 * @pending_mask:	pending rebalanced interrupts
 * @threads_oneshot:	bitfield to handle shared oneshot threads
 * @threads_active:	number of irqaction threads currently running
 * @wait_for_threads:	wait queue for sync_irq to wait for threaded handlers
 * @nr_actions:		number of installed actions on this descriptor
 * @no_suspend_depth:	number of irqactions on a irq descriptor with
 *			IRQF_NO_SUSPEND set
 * @force_resume_depth:	number of irqactions on a irq descriptor with
 *			IRQF_FORCE_RESUME set
 * @dir:		/proc/irq/ procfs entry
 * @name:		flow handler name for /proc/interrupts output
 */
struct irq_desc {
	struct irq_common_data	irq_common_data; //包含与中断处理相关的通用数据。
	struct irq_data		irq_data;            //包含与中断处理相关的数据，如中断号、中断操作函数等。
	unsigned int __percpu	*kstat_irqs;     //用于统计中断的计数器，每个 CPU 都有一个。
	irq_flow_handler_t	handle_irq;          //中断处理函数，是中断处理的基本流程函数类型，负责中断的分发和处理流程。
#ifdef CONFIG_IRQ_PREFLOW_FASTEOI
	irq_preflow_handler_t	preflow_handler;   //用于提前处理中断的处理函数，仅在启用了 CONFIG_IRQ_PREFLOW_FASTEOI 选项时可用。
#endif
	struct irqaction	*action;   /*
                                    * IRQ action list  指向中断处理程序的链表，保存与该中断相关的所有中断动作。
                                    * 是具体中断处理逻辑的实现函数类型，用于执行中断事件发生时的具体操作。两者在中断处理过程中有不同的角色和功能，但它们通常一起协同工作来完成对中断的处理
                                    * */
	unsigned int		status_use_accessors; //表示是否使用访问器函数来读取和写入中断状态。
	unsigned int		core_internal_state__do_not_mess_with_it;  //内核内部状态变量，用于中断处理的内核使用   istate为他的宏定义。
	unsigned int		depth;		/* nested irq disables   表示当前中断嵌套的深度，用于中断的禁用和启用。*/
	unsigned int		wake_depth;	/* nested wake enables   表示嵌套唤醒中断的深度，用于唤醒中断的禁用和启用。*/
	unsigned int		irq_count;	/* For detecting broken IRQs  用于检测是否存在中断问题的计数器。*/
	unsigned long		last_unhandled;	/* Aging timer for unhandled count  未处理中断的计时器，用于检测未处理中断的时长。*/
	unsigned int		irqs_unhandled;       //未处理中断的计数。
	atomic_t		threads_handled;          //已处理中断的线程计数。
	int			threads_handled_last;         //上一次已处理中断的线程计数。
	raw_spinlock_t		lock;                 //结构体的访问的自旋锁。
	struct cpumask		*percpu_enabled;      //指向处理中断的 CPU 集合。
#ifdef CONFIG_SMP
	const struct cpumask	*affinity_hint;   //用于指示中断亲和性的提示。
	struct irq_affinity_notify *affinity_notify; //指向中断亲和性通知的结构体。
#ifdef CONFIG_GENERIC_PENDING_IRQ
	cpumask_var_t		pending_mask;         //用于存储待处理中断的 CPU 集合。
#endif
#endif
	unsigned long		threads_oneshot;      //用于跟踪一次性中断处理线程的数量。
	atomic_t		threads_active;           //活动中断处理线程的计数。
	wait_queue_head_t       wait_for_threads; //等待中断处理线程完成的等待队列头。
#ifdef CONFIG_PM_SLEEP
	unsigned int		nr_actions;           //中断动作的数量。
	unsigned int		no_suspend_depth;     //禁用挂起的深度。
	unsigned int		cond_suspend_depth;   //条件挂起的深度。
	unsigned int		force_resume_depth;   //强制恢复的深度。
#endif
#ifdef CONFIG_PROC_FS
	struct proc_dir_entry	*dir;             //指向中断在 /proc 文件系统中的目录项的指针。
#endif
	int			parent_irq;                   //父中断号。
	struct module		*owner;               //拥有该中断的内核模块。
	const char		*name;                    //中断的名称。
} ____cacheline_internodealigned_in_smp;

#ifndef CONFIG_SPARSE_IRQ
extern struct irq_desc irq_desc[NR_IRQS];
#endif

static inline struct irq_data *irq_desc_get_irq_data(struct irq_desc *desc)
{
	return &desc->irq_data;
}

static inline struct irq_chip *irq_desc_get_chip(struct irq_desc *desc)
{
	return desc->irq_data.chip;
}

static inline void *irq_desc_get_chip_data(struct irq_desc *desc)
{
	return desc->irq_data.chip_data;
}

static inline void *irq_desc_get_handler_data(struct irq_desc *desc)
{
	return desc->irq_data.handler_data;
}

static inline struct msi_desc *irq_desc_get_msi_desc(struct irq_desc *desc)
{
	return desc->irq_data.msi_desc;
}

/*
 * Architectures call this to let the generic IRQ layer
 * handle an interrupt. If the descriptor is attached to an
 * irqchip-style controller then we call the ->handle_irq() handler,
 * and it calls __do_IRQ() if it's attached to an irqtype-style controller.
 */
static inline void generic_handle_irq_desc(unsigned int irq, struct irq_desc *desc)
{
	desc->handle_irq(irq, desc);
}

int generic_handle_irq(unsigned int irq);

#ifdef CONFIG_HANDLE_DOMAIN_IRQ
/*
 * Convert a HW interrupt number to a logical one using a IRQ domain,
 * and handle the result interrupt number. Return -EINVAL if
 * conversion failed. Providing a NULL domain indicates that the
 * conversion has already been done.
 */
int __handle_domain_irq(struct irq_domain *domain, unsigned int hwirq,
			bool lookup, struct pt_regs *regs);

static inline int handle_domain_irq(struct irq_domain *domain,
				    unsigned int hwirq, struct pt_regs *regs)
{
	return __handle_domain_irq(domain, hwirq, true, regs);
}
#endif

/* Test to see if a driver has successfully requested an irq */
static inline int irq_has_action(unsigned int irq)
{
	struct irq_desc *desc = irq_to_desc(irq);
	return desc->action != NULL;
}

/* caller has locked the irq_desc and both params are valid */
static inline void __irq_set_handler_locked(unsigned int irq,
					    irq_flow_handler_t handler)
{
	struct irq_desc *desc;

	desc = irq_to_desc(irq);
	desc->handle_irq = handler;
}

/* caller has locked the irq_desc and both params are valid */
static inline void
__irq_set_chip_handler_name_locked(unsigned int irq, struct irq_chip *chip,
				   irq_flow_handler_t handler, const char *name)
{
	struct irq_desc *desc;

	desc = irq_to_desc(irq);
	irq_desc_get_irq_data(desc)->chip = chip;
	desc->handle_irq = handler;
	desc->name = name;
}

static inline int irq_balancing_disabled(unsigned int irq)
{
	struct irq_desc *desc;

	desc = irq_to_desc(irq);
	return desc->status_use_accessors & IRQ_NO_BALANCING_MASK;
}

static inline int irq_is_percpu(unsigned int irq)
{
	struct irq_desc *desc;

	desc = irq_to_desc(irq);
	return desc->status_use_accessors & IRQ_PER_CPU;
}

static inline void
irq_set_lockdep_class(unsigned int irq, struct lock_class_key *class)
{
	struct irq_desc *desc = irq_to_desc(irq);

	if (desc)
		lockdep_set_class(&desc->lock, class);
}

#ifdef CONFIG_IRQ_PREFLOW_FASTEOI
static inline void
__irq_set_preflow_handler(unsigned int irq, irq_preflow_handler_t handler)
{
	struct irq_desc *desc;

	desc = irq_to_desc(irq);
	desc->preflow_handler = handler;
}
#endif

#endif
