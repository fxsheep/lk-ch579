#pragma once

#include <CH57x_common.h>

#define RWA_UNLOCK do {R8_SAFE_ACCESS_SIG = SAFE_ACCESS_SIG1; \
    R8_SAFE_ACCESS_SIG = SAFE_ACCESS_SIG2;} while(0)
#define RWA_LOCK do {R8_SAFE_ACCESS_SIG = 0;} while(0)