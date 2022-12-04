// Pico Libraries
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"

// Decawave Libraries
#include "deca_device_api.h"
#include "deca_regs.h"

// Config Files
#include "uwb_config.h"


/*! ------------------------------------------------------------------------------------------------------------------
 * @brief This is used to intialize the GPIO pins use by the UWB
 *
 * NOTE: This is called by the uwb_init function
 * 
 */
void gpio_uwb_init(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @brief This is used to initialize the UWB SPI at the frequency defined in
 *        uwb_config.h
 *
 * NOTE: This is called by the uwb_init function
 * 
 */
void spi_uwb_init(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @brief Fully intiializes and configures UWB based on settings in uwb_config.h
 * 
 * NOTE: This will spin forever if any of the initiation steps fail
 * 
 */
void uwb_init(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @brief This is used to reset the UWB using the reset pin
 *
 * NOTE: This should be called everytime there is a hardware error with the UWB
 * 
 */
void uwb_reset(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @brief  This function is used to wait for an incoming message from UWB and
           write it into the buffer
 *
 * input parameters:
 * @param rx_buf  - Pointer to uint8_t buffer to write incoming message to
 *
 * output parameters:
 *
 * @return len - length of incoming message 
 */
int receive_msg(uint8_t *rx_buf, void (*idle_task)());

/*! ------------------------------------------------------------------------------------------------------------------
 * @brief  This function is used to send a message using UWB
 *
 * input parameters:
 * @param msg_len    - Length of message to send
 * @param tx_buf     - Pointer to uint8_t buffer where message is stored
 * @param msg_offset - Offset from buffer to start
 *
 * output parameters:
 *
 * @return len - length of incoming message, returns -1 if no message received
 */
void send_msg(uint16_t msg_len, uint8_t *tx_buf, uint16_t msg_offset, void (*idle_task)());
