#ifndef _EXT_IRQ_H_
#define _EXT_IRQ_H_

#define IRQ_LIST_SIZE 16

typedef void (*EXTIRQ_PROC)(int exti_line);

struct EXTIRQ_PROC_DATA {
        void *owner;
        EXTIRQ_PROC proc;
};

extern struct EXTIRQ_PROC_DATA irq_proc_list[];

#define	EXT_IRQ_OWNER_SYSTEM	(void*)0x08000000

#ifndef SVC_CLIENT

void ext_irq_init();
int ext_irq_set(int exti_line, EXTIRQ_PROC proc, void* owner);
int ext_irq_clear(int exti_line, void *owner);
void ext_irq_clear_all(void* clear);
void ext_irq_call_proc(int exti_line);
EXTIRQ_PROC ext_irq_get(int exti_line);

#else

int svc_ext_irq_set(int exti_line, EXTIRQ_PROC proc);
int svc_ext_irq_clear(int exti_line);
EXTIRQ_PROC svc_ext_irq_get(int exti_line);

#endif //SVC_CLIENT

#endif //_EXT_IRQ_H_

#ifdef SVC_CLIENT_IMPL

#ifndef _EXT_IRQ_H_IMPL_
#define _EXT_IRQ_H_IMPL_

#include "svc.h"

__attribute__ ((noinline)) int svc_ext_irq_set(int exti_line, EXTIRQ_PROC proc) {
        svc(SVC_EXT_IRQ_SET);
}

__attribute__ ((noinline)) int svc_ext_irq_clear(int exti_line) {
        svc(SVC_EXT_IRQ_CLEAR);
}

__attribute__ ((noinline)) EXTIRQ_PROC svc_ext_irq_get(int exti_line) {
        svc(SVC_EXT_IRQ_GET);
}

#endif //_EXT_IRQ_H_IMPL_
#endif //SVC_CLIENT_IMPL
