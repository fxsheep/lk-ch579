#pragma once

#define __CM0_REV              0
#define __NVIC_PRIO_BITS       2
#define __Vendor_SysTickConfig 0
#define __MPU_PRESENT          1
#define __FPU_PRESENT          0

typedef enum {
  Reset_IRQn                = -15,
  NonMaskableInt_IRQn       = -14,
  HardFault_IRQn            = -13,
  SVCall_IRQn               =  -5,
  PendSV_IRQn               =  -2,
  SysTick_IRQn              =  -1,
#define CH579_IRQ(name,num) name##_IRQn = num,
#include <platform/irqinfo.h>
#undef CH579_IRQ
} IRQn_Type;
