#include <lk/debug.h>
#include <lk/compiler.h>
#include <arch/arm/cm.h>

/* un-overridden irq handler */
void ch579_dummy_irq(void) {
    arm_cm_irq_entry();
    panic("unhandled irq\n");
}

/* a list of default handlers that are simply aliases to the dummy handler */
#define CH579_IRQ(name,num) \
void name##_IRQHandler(void) __WEAK_ALIAS("ch579_dummy_irq");
#include <platform/irqinfo.h>
#undef CH579_IRQ

const void* const __SECTION(".text.boot.vectab2") vectab2[26] = {
#define CH579_IRQ(name,num) [name##_IRQn] = name##_IRQHandler,
#include <platform/irqinfo.h>
#undef CH579_IRQ
};

