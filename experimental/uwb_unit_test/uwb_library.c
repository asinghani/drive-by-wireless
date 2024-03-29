#include "uwb_library.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"

/*
 * TX Power Configuration Settings
 */
/* Values for the PG_DELAY and TX_POWER registers reflect the bandwidth and power of the spectrum at the current
 * temperature. These values can be calibrated prior to taking reference measurements. */
dwt_txconfig_t txconfig_options =
{
    0x34,           /* PG delay. */
    0xfdfdfdfd,     /* TX power. */
    0x0             /*PG count*/
};

/* Default communication configuration. We use default non-STS DW mode. */
static dwt_config_t config = {
    .chan            = 9,               /* Channel number. */
    .txPreambLength  = DWT_PLEN_128,    /* Preamble length. Used in TX only. */
    .rxPAC           = DWT_PAC8,        /* Preamble acquisition chunk size. Used in RX only. */
    .txCode          = 9,               /* TX preamble code. Used in TX only. */
    .rxCode          = 9,               /* RX preamble code. Used in RX only. */
    .sfdType         = DWT_SFD_DW_8,    /* 0 to use standard 8 symbol SFD */
    .dataRate        = DWT_BR_6M8,      /* Data rate. */
    .phrMode         = DWT_PHRMODE_EXT, /* PHY header mode. */
    .phrRate         = DWT_PHRRATE_STD, /* PHY header rate. */
    .sfdTO           = (129 + 8 - 8),   /* SFD timeout */
    .stsMode         = DWT_STS_MODE_OFF,
    .stsLength       = DWT_STS_LEN_64,  /* STS length, see allowed values in Enum dwt_sts_lengths_e */
    .pdoaMode        = DWT_PDOA_M0      /* PDOA mode off */
};

// Initialize GPIO Pins
void gpio_uwb_init() {
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_init(PIN_RSTN);

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
}

// Hardware reset for UWB
void uwb_reset() {
    // ResetN is active-low, we'll drive it low for a while, then let it float
    gpio_set_dir(PIN_RSTN, GPIO_OUT);
    gpio_put(PIN_RSTN, 0);
    sleep_ms(1);
    gpio_set_dir(PIN_RSTN, false);
    gpio_disable_pulls(PIN_RSTN);
    sleep_ms(10);
}

// Function to receive UWB message into buffer
int receive_msg(uint8_t *rx_buf, absolute_time_t *recv_timestamp) {
    /* Activate reception immediately. See NOTE 2 below. */
    dwt_rxenable(DWT_START_RX_IMMEDIATE);

    /* Hold copy of status register state here for reference so that it can
     * be examined at a debug breakpoint. */
    uint32_t status_reg;

    /* Hold copy of frame length of frame received (if good) so that it can
     * be examined at a debug breakpoint. */
    uint16_t frame_len;
    
    /* Poll until a frame is properly received or an error/timeout occurs.
     * STATUS register is 5 bytes long but, as the event we are looking at
     * is in the first byte of the register, we can use this simplest API
     * function to access it. */
    while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_ERR )))
    {
    };

    if (status_reg & SYS_STATUS_ALL_RX_ERR) {
        if (status_reg & SYS_STATUS_RXPHE_BIT_MASK)  printf("receive error: RXPHE\n");  // Phy. Header Error
        if (status_reg & SYS_STATUS_RXFCE_BIT_MASK)  printf("receive error: RXFCE\n");  // Rcvd Frame & CRC Error
        if (status_reg & SYS_STATUS_RXFSL_BIT_MASK)  printf("receive error: RXFSL\n");  // Frame Sync Loss
        if (status_reg & SYS_STATUS_RXSTO_BIT_MASK)  printf("receive error: RXSTO\n");  // Rcv Timeout
        if (status_reg & SYS_STATUS_ARFE_BIT_MASK)   printf("receive error: ARFE\n");   // Rcv Frame Error
        if (status_reg & SYS_STATUS_CIAERR_BIT_MASK) printf("receive error: CIAERR\n"); 
        // if (status_reg & SYS_STATUS_RXFTO_BIT_MASK) printf("receive error: RXFT0\n");  // Timeout!
    }

    if (status_reg & SYS_STATUS_RXFCG_BIT_MASK) {

        if (status_reg & SYS_STATUS_ALL_RX_ERR) printf("Am I supposed to be here?\n");

        *recv_timestamp = get_absolute_time();
        
        /* A frame has been received, copy it to our local buffer. */
        frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_BIT_MASK;
        if (frame_len <= FRAME_LEN_MAX) {
            dwt_readrxdata(rx_buf, frame_len-FCS_LEN, 0); /* No need to read the FCS/CRC. */
        }

        /* Clear good RX frame event in the DW IC status register. */
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);
    }
    else {
        /* Clear RX error events in the DW IC status register. */
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
        return -1;
    }
    
    if (status_reg & SYS_STATUS_ALL_RX_ERR) {
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
        return -1;
    }

    return frame_len;
}

// Function to send message via UWB
void send_msg(uint16_t msg_len, uint8_t *tx_buf, uint16_t msg_offset) {
    /* Write frame data to DW IC and prepare transmission */
    dwt_writetxdata(msg_len, tx_buf, msg_offset);

    /* In this example since the length of the transmitted frame does not change,
     * nor the other parameters of the dwt_writetxfctrl function, the
     * dwt_writetxfctrl call could be outside the main while(1) loop.
     */
    dwt_writetxfctrl(msg_len + FCS_LEN, 0, 0); /* Zero offset in TX buffer, no ranging. */

    /* Start transmission. */
    dwt_starttx(DWT_START_TX_IMMEDIATE);

    /* Poll DW IC until TX frame sent event set. See NOTE 4 below.
     * STATUS register is 4 bytes long but, as the event we are looking at
     * is in the first byte of the register, we can use this simplest API
     * function to access it.*/
    while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS_BIT_MASK))
    {};

    /* Clear TX frame sent event. */
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
}

// SPI Initiation
void spi_uwb_init() {
    // This example will use SPI1 at 20MHz.
    spi_init(SPI_PORT, SPI_FREQ);
    // spi_set_format(SPI_PORT, 8, SPI_POL, SPI_PHA, 1);
}

// Full UWB Initiation with Configuration
void uwb_init() {
    
    gpio_uwb_init();
    spi_uwb_init();
    uwb_reset();

    sleep_ms(10);

    if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR) {
        printf("INIT FAILED");
        while (1) { printf("INIT FAILED\n"); };
    }

    /* Configure DW IC. See NOTE 5 below. */
    /* If the dwt_configure returns DWT_ERROR either the PLL or RX calibration
     * has failed the host should reset the device */
    if (dwt_configure(&config))  {
        printf("CONFIG FAILED");
        while (1) { printf("CONFIG FAILED\n"); };
    }

    /* Configure the TX spectrum parameters (power PG delay and PG Count) */
    dwt_configuretxrf(&txconfig_options);

    /* Set response frame timeout. */
    dwt_setrxtimeout(RX_RESP_TO_UUS);

    sleep_ms(10);
}

