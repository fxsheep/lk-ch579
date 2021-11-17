#include <lk/reg.h>
#include <arch/arm/cm.h>
#include <platform/ch579.h>
#include <platform/eth.h>
#include <dev/gpio.h>
#include <target/gpiomap.h>

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
    R8_ETH_MIREGADR = addr & RB_ETH_MIREGADR_MASK | RB_ETH_MIREGADR_MIIWR;
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

void ch579_eth_init(uint16_t maxmfl, uint8_t macaddr[]) {
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

    //TODO: DMA buffer handling

    NVIC_EnableIRQ(ETH_IRQn);
}

void ETH_IRQHandler(void) {
    bool resched = false;
    uint8_t eir;
    arm_cm_irq_entry();
    eir = R8_ETH_EIR;
    if (eir & RB_ETH_EIR_RXIF) {
        //Receive complete
        //TODO
        R8_ETH_EIR = RB_ETH_EIR_RXIF;
    }
    if (eir & RB_ETH_EIR_TXIF) {
        //Send complete
        //TODO
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