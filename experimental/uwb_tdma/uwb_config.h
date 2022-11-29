/*! ----------------------------------------------------------------------------
 *  @file   uwb_config.h
 *  @brief  UWB Config Settings
 *
 */

// Max Length of Input Frame
#define FRAME_LEN_MAX      (127)

// UWB Pins
#define PIN_MISO 12
#define PIN_CS   13
#define PIN_SCK  14
#define PIN_MOSI 15
#define PIN_RSTN 16

// SPI Ports
#define SPI_PORT spi1

// SPI Frequency
#define SPI_FREQ 20 * 1000 * 1000

/* Receive response timeout, expressed in UWB microseconds. */
#define RX_RESP_TO_UUS 4000