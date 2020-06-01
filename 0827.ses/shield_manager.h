#ifndef SHIELD_MANAGER_H__
#define SHIELD_MANAGER_H__

#include "drv_ADG728.h"
#include "drv_AD5245.h"

#define ADG728_1_ADDR   0x4C  // Sensor selection 
#define ADG728_2_ADDR   0x4D  // Amplifier gain selection
#define ADG728_3_ADDR   0x4E  // Bridge balancing
#define AD5245_ADDR     0x2C  // Bridge balancing


/**@brief Return status codes.
 */
enum
{
    SHIELD_MANAGER_STATUS_CODE_SUCCESS,            ///< Successful.
    SHIELD_MANAGER_STATUS_CODE_INVALID_PARAM,      ///< Invalid parameters.
    //DRV_ACC_STATUS_WRONG_DEVICE,            ///< Wrong device at I2C (TWI) address.
    //DRV_ACC_STATUS_UNINITALIZED,            ///< The driver is unitialized, please initialize.
};

ret_code_t shield_manager_init(void);

ret_code_t setMUXChannel(uint8_t sensor);

ret_code_t amplifier_gain_selection(uint8_t gain);

ret_code_t bridge_balancing(uint16_t resistor_value);

static ret_code_t select_bridge_resistor(uint8_t line);
    
static ret_code_t set_bridge_digiPot(uint8_t digiPot_value_to_set);

#endif // SHIELD_MANAGER_H__

/** @} */