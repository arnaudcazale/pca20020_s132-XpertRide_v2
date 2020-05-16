#ifndef DRV_ADG728_H__
#define DRV_ADG728_H__

#include <stdint.h>
#include <stddef.h>
#include "nrf_drv_twi.h"


/**@brief TWI communication initialization struct.
 */
typedef struct
{
    nrf_drv_twi_t         const * p_twi_instance;
    nrf_drv_twi_config_t  const * p_twi_cfg;
}drv_ADG728_init_t;


/**@brief Function for initializing the MPU-9250 driver.
 *
 * @param[in] p_params      Pointer to the init paramter structure.
 *
 * @retval NRF_SUCCESS.
 */
uint32_t drv_ADG728_init(drv_ADG728_init_t * p_params);

/**@brief Function for enabling or disabling the MPU-9250 driver.
 *
 * @param[in] enable    Enables the device.
 *
 * @retval NRF_SUCCESS.
 */

int drv_drv_ADG728_write(unsigned char slave_addr, unsigned char reg_addr, unsigned char length, unsigned char const * p_data);

/**@brief Function for reading a MPU-9250 register.
 *
 * @param[in]  slave_addr   Slave address on the TWI bus.
 * @param[in]  reg_addr     Register address to read.
 * @param[in]  length       Length of the data to read.
 * @param[out] p_data       Pointer to where the data should be stored.
 *
 * @retval 0 if success. Else -1.
 */
int drv_drv_ADG728_read(unsigned char slave_addr, unsigned char reg_addr, unsigned char length, unsigned char * p_data);

void set_gauge_resistor();
static void select_sensor();

#endif // DRV_ADG728_H__

/** @} */