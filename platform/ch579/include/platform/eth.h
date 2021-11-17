#pragma once

#define CH579_DEFAULT_MAXMFL 1500

#define PHY_REG_STATUS 0x1
#define PHY_REG_STATUS_CONNECTED 0x4

void ch579_eth_init(void);
bool ch579_eth_is_error_seen(bool clear);