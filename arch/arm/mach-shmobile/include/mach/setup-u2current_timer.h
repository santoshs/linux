#ifndef __ASM_ARCH_CURRENT_TIMER_H
#define __ASM_ARCH_CURRENT_TIMER_H

int __init setup_current_timer(void);
int cmt_read_current_timer(unsigned long *timer_val);
#endif /* __ASM_ARCH_CURRENT_TIMER_H */
