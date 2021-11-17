#include <lk/reg.h>
#include <lk/err.h>
#include <arch/arm/cm.h>
#include <kernel/event.h>
#include <platform/ch579.h>
#include <platform/chipflash.h>
#include <platform/eth.h>
#include <dev/gpio.h>
#include <target/gpiomap.h>
#include <CH57x_common.h>

#include "lwip/dhcp.h"
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include <lwip/stats.h>

#include <netif/etharp.h>

/* Global pointer of netif */
struct netif *ch579_netif;

/* DMA buffers for TX and RX */
//Make sure it's 4-bytes aligned
uint8_t __attribute__((aligned(4))) tx_dma_buf[1600], rx_dma_buf[1600];

/* MAC address */
uint8_t macaddr[6];

static event_t rx_arrived_event = EVENT_INITIAL_VALUE(rx_arrived_event, false, 0);

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'

struct ethernetif {
    struct eth_addr *ethaddr;
    /* Add whatever per-interface state that is needed here. */
};

/* Forward declarations. */
static void  ethernetif_input(struct netif *netif);

static bool tx_rx_error_seen;

void ch579_eth_on(void) {
    RWA_UNLOCK;
    R8_SLP_CLK_OFF1 &= ~RB_SLP_CLK_ETH;
    R8_SLP_POWER_CTRL &= ~RB_SLP_ETH_PWR_DN;
    RWA_LOCK;
    R16_PIN_ANALOG_IE |= RB_PIN_ETH_IE;
}

void ch579_eth_off(void) {
    RWA_UNLOCK;
    R8_SLP_CLK_OFF1 |= RB_SLP_CLK_ETH;
    R8_SLP_POWER_CTRL |= RB_SLP_ETH_PWR_DN;
    RWA_LOCK;
    R16_PIN_ANALOG_IE &= ~RB_PIN_ETH_IE;
}

uint16_t ch579_eth_read_phy_reg(uint8_t addr) {
    R8_ETH_MIREGADR = (addr & RB_ETH_MIREGADR_MASK) | RB_ETH_MIREGADR_MIIWR;
    return R16_ETH_MIRD;
}

void ch579_eth_write_phy_reg(uint8_t addr, uint16_t value) {
    R16_ETH_MIWR = value;
    R8_ETH_MIREGADR = addr & RB_ETH_MIREGADR_MASK;
}

bool ch579_eth_is_error_seen(bool clear) {
    bool ret = tx_rx_error_seen;
    if (clear){
        tx_rx_error_seen = false;
    }
    return ret;
}

/* No actual HW initialization but only HW info 
 is filled and passed to the stack in this function. */

static void
low_level_init(struct netif *netif) {
    /* fill MAC hardware address length */
    netif->hwaddr_len = 6;

    /* fill MAC hardware address */
    netif->hwaddr[0] = (R16_ETH_MAADRH >> 8) & 0xFF;
    netif->hwaddr[1] = (R16_ETH_MAADRH) & 0xFF;
    netif->hwaddr[2] = (R32_ETH_MAADRL >> 24) & 0xFF;
    netif->hwaddr[3] = (R32_ETH_MAADRL >> 16) & 0xFF;
    netif->hwaddr[4] = (R32_ETH_MAADRL >> 8) & 0xFF;
    netif->hwaddr[5] = (R32_ETH_MAADRL) & 0xFF;

    /* maximum transfer unit */
    netif->mtu = R16_ETH_MAMXFL;

    /* broadcast capability */
    netif->flags = NETIF_FLAG_BROADCAST;
}

/*
 * low_level_output():
 *
 * Should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 */

static err_t
low_level_output(struct netif *netif, struct pbuf *p) {
    struct pbuf *q;
    int i;
    int j;

#if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE);            /* drop the padding word */
#endif

    /* Wait for previous TX DMA to complete */
    while (R8_ETH_ECON1 & RB_ETH_ECON1_TXRTS) {
        thread_sleep(50);
    }

    i = 0;
    for (q = p; q != NULL; q = q->next) {
        /* Send the data from the pbuf to the interface, one pbuf at a
           time. The size of the data in each pbuf is kept in the ->len
           variable. */
//  debug_dump_memory_bytes(q->payload, q->len);
        for (j = 0; j < q->len; j++)
            *(uint8_t *)(tx_dma_buf + i + j) = ((unsigned char *)q->payload)[j];
        i += q->len;
    }

    /* Set DMA TX length */
    R16_ETH_ETXLN = i;
    /* Start TX DMA */
    R8_ETH_ECON1 |= RB_ETH_ECON1_TXRTS;

#if ETH_PAD_SIZE
    pbuf_header(p, ETH_PAD_SIZE);         /* reclaim the padding word */
#endif

#if LINK_STATS
    lwip_stats.link.xmit++;
#endif /* LINK_STATS */

    return ERR_OK;
}

/*
 * low_level_input():
 *
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 */

static struct pbuf *
low_level_input(struct netif *netif) {
    struct pbuf *p, *q;
    u16_t len;
    int i;

    /* Obtain the size of the packet and put it into the "len"
       variable. */
    len = R16_ETH_ERXLN;

#if ETH_PAD_SIZE
    len += ETH_PAD_SIZE;                      /* allow room for Ethernet padding */
#endif

    /* We allocate a pbuf chain of pbufs from the pool. */
    p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
    if (p != NULL) {

#if ETH_PAD_SIZE
        pbuf_header(p, -ETH_PAD_SIZE);          /* drop the padding word */
#endif

        /* We iterate over the pbuf chain until we have read the entire
         * packet into the pbuf. */
        int pos = 0;
        for (q = p; q != NULL; q = q->next) {
            /* Read enough bytes to fill this pbuf in the chain. The
             * available data in the pbuf is given by the q->len
             * variable. */
            for (i=0; i < q->len; i++) {
                ((unsigned char *)q->payload)[i] = *(uint8_t *)(rx_dma_buf + pos);
                pos++;
            }
        }

#if ETH_PAD_SIZE
        pbuf_header(p, ETH_PAD_SIZE);           /* reclaim the padding word */
#endif

#if LINK_STATS
        lwip_stats.link.recv++;
#endif /* LINK_STATS */
    } else {
#if LINK_STATS
        lwip_stats.link.memerr++;
        lwip_stats.link.drop++;
#endif /* LINK_STATS */
    }

    return p;
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
static void
ethernetif_input(struct netif *netif)
{
  struct ethernetif *ethernetif;
  struct eth_hdr *ethhdr;
  struct pbuf *p;

  ethernetif = netif->state;

  /* move received packet into a new pbuf */
  p = low_level_input(netif);
  /* no packet could be read, silently ignore this */
  if (p == NULL) return;
  /* points to packet payload, which starts with an Ethernet header */
  ethhdr = p->payload;

  switch (htons(ethhdr->type)) {
  /* IP or ARP packet? */
  case ETHTYPE_IP:
  case ETHTYPE_ARP:
#if PPPOE_SUPPORT
  /* PPPoE packet? */
  case ETHTYPE_PPPOEDISC:
  case ETHTYPE_PPPOE:
#endif /* PPPOE_SUPPORT */
    /* full packet send to tcpip_thread to process */
    if (netif->input(p, netif)!=ERR_OK)
     { LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
       pbuf_free(p);
       p = NULL;
     }
    break;

  default:
    pbuf_free(p);
    p = NULL;
    break;
  }
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t
ethernetif_init(struct netif *netif)
{
  struct ethernetif *ethernetif;

  LWIP_ASSERT("netif != NULL", (netif != NULL));
    
  ethernetif = mem_malloc(sizeof(struct ethernetif));
  if (ethernetif == NULL) {
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
    return ERR_MEM;
  }

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

  netif->state = ethernetif;
  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;
  netif->linkoutput = low_level_output;
  
  ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);
  
  /* initialize the hardware */
  low_level_init(netif);

  return ERR_OK;
}

status_t ethernet_init(void) {
    ch579_netif = calloc(sizeof(struct netif), 1);
    struct ip_addr *ipaddr = calloc(sizeof(struct ip_addr), 1);
    struct ip_addr *netmask = calloc(sizeof(struct ip_addr), 1);
    struct ip_addr *gw = calloc(sizeof(struct ip_addr), 1);

    struct netif *netifret = netif_add(ch579_netif, ipaddr, netmask, gw, NULL, &ethernetif_init, &ethernet_input);
    if (netifret == NULL) {
        free(ch579_netif);
        free(ipaddr);
        free(netmask);
        free(gw);
        return ERR_NOT_FOUND;
    }

    netif_set_default(ch579_netif);

    NVIC_EnableIRQ(ETH_IRQn);

    return NO_ERROR;
}

/*
static int ch579_eth_rx_thread(void *arg) {
    while (1) {
        event_wait(&rx_arrived_event);
        ethernetif_input(ch579_netif);
    }
    return 0;
}
*/

static void ch579_eth_hwinit(uint16_t maxmfl, uint8_t macaddr[]) {
    /* Init Ethernet LEDs*/
    gpio_config(ETH_CONN_LED, GPIO_OUTPUT);
    gpio_config(ETH_DATA_LED, GPIO_OUTPUT);
    gpio_set(ETH_CONN_LED, 1);
    gpio_set(ETH_DATA_LED, 1);

    ch579_eth_on();

    R8_ETH_EIE = RB_ETH_EIE_INTIE | RB_ETH_EIE_RXIE | RB_ETH_EIE_LINKIE \
        | RB_ETH_EIE_TXIE | RB_ETH_EIE_R_EN50 | RB_ETH_EIE_TXERIE | RB_ETH_EIE_RXERIE;
    
    R8_ETH_EIR = 0xFF;
    R8_ETH_ESTAT = RB_ETH_ESTAT_INT | RB_ETH_ESTAT_BUFER;

    R8_ETH_ECON1 |= RB_ETH_ECON1_TXRST | RB_ETH_ECON1_RXRST;
    R8_ETH_ECON1 &= ~(RB_ETH_ECON1_TXRST | RB_ETH_ECON1_RXRST);

    R32_ETH_MACON = 0;
    R8_ETH_MACON1 |= RB_ETH_MACON1_MARXEN;
    R8_ETH_MACON2 &= 0x1F;
    R8_ETH_MACON2 |= 0x20;
    R8_ETH_MACON2 |= RB_ETH_MACON2_TXCRCEN;
    R8_ETH_MACON2 &= ~RB_ETH_MACON2_HFRMEN;
    R8_ETH_MACON2 |= RB_ETH_MACON2_FULDPX;

    /* Configure MTU */
    R16_ETH_MAMXFL = maxmfl;

    /* Configure MAC address */
    R16_ETH_MAADRH = macaddr[0] << 8 | macaddr[1];
    R32_ETH_MAADRL = macaddr[2] << 24 | macaddr[3] << 16 | macaddr[4] << 8 | macaddr[5];

    /* Configure DMA buffers */
    R16_ETH_ETXST = tx_dma_buf;
    R16_ETH_ERXST = rx_dma_buf;
}

void ch579_eth_init(void) {
    thread_t *rx_thread;

    ch579_infoflash_read_macaddr(macaddr);
    ch579_eth_hwinit(CH579_DEFAULT_MAXMFL, macaddr);

    ethernet_init();

    dhcp_start(ch579_netif);
//    rx_thread = thread_create("ch579_eth_rx", ch579_eth_rx_thread, NULL, HIGH_PRIORITY, DEFAULT_STACK_SIZE);
//    thread_resume(rx_thread);
}

void ETH_IRQHandler(void) {
    bool resched = false;
    uint8_t eir;
    arm_cm_irq_entry();
    eir = R8_ETH_EIR;
    if (eir & RB_ETH_EIR_RXIF) {
        //Receive complete
        ethernetif_input(ch579_netif);
//        event_signal(&rx_arrived_event, true);
//        resched = true;
        R8_ETH_EIR = RB_ETH_EIR_RXIF;
    }
    if (eir & RB_ETH_EIR_TXIF) {
        //Send complete
        R8_ETH_EIR = RB_ETH_EIR_TXIF;
    }
    if (eir & RB_ETH_EIR_LINKIF) {
        //Link has changed
        if (ch579_eth_read_phy_reg(PHY_REG_STATUS) & PHY_REG_STATUS_CONNECTED) {
            R8_ETH_ECON1 |= RB_ETH_ECON1_RXEN;
            gpio_set(ETH_CONN_LED, 0);
            gpio_set(ETH_DATA_LED, 0);
        } else {
            R8_ETH_ECON1 &= ~RB_ETH_ECON1_RXEN;
            gpio_set(ETH_CONN_LED, 1);
            gpio_set(ETH_DATA_LED, 1);
        }
        R8_ETH_EIR = RB_ETH_EIR_LINKIF;
    }
    if (eir << 30) {
        //Send/receive error seen
        tx_rx_error_seen = true;
        R8_ETH_EIR = RB_ETH_EIR_TXERIF | RB_ETH_EIR_RXERIF;
    }
    arm_cm_irq_exit(resched);
}