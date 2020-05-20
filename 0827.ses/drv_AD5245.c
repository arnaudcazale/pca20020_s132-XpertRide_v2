#include "drv_AD5245.h"
#include "twi_manager.h"
#define  NRF_LOG_MODULE_NAME "drv_AD5245   "
#include "nrf_log.h"
#include "macros_common.h"

static struct
{
    drv_AD5245_init_t         init;             ///< TWI configuration.
//    void                      (*cb)(void);      ///< Callback. Invoked when a pin interrupt is caught by GPIOTE.
    bool                      initialized;      ///< Module initialized.
    bool                      int_registered;   ///<
    bool                      enabled;          ///< Driver enabled.
//    uint32_t                  evt_sheduled;     ///< Number of scheduled events.
} m_AD5245 = {.initialized = false, .int_registered = false};


/**@brief Function to init / allocate the TWI module
 */
static __inline uint32_t twi_open(void)
{
    uint32_t err_code;

    err_code = twi_manager_request(m_AD5245.init.p_twi_instance,
                                   m_AD5245.init.p_twi_cfg,
                                   NULL,
                                   NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(m_AD5245.init.p_twi_instance);

    return NRF_SUCCESS;
}


/**@brief Function to deinit the TWI module when this driver does not need to
 *        communicate on the TWI bus, so that other drivers can use the module.
 */
static __inline uint32_t twi_close(void)
{
    nrf_drv_twi_disable(m_AD5245.init.p_twi_instance);

    nrf_drv_twi_uninit(m_AD5245.init.p_twi_instance);

    return NRF_SUCCESS;
}

/**@brief Read and verify accelerometer ID.
 */
//static ret_code_t drv_AD5245_verify_id(void)
//{
//    ret_code_t  err_code;
//    uint8_t     reg_val;
//
//    err_code = drv_AD5245_read(AD5245_1_ADDR, WHO_AM_I, 1, &reg_val);
//    RETURN_IF_ERROR(err_code);
//
//    return (reg_val == I_AM_AD5245);
//}

/**@brief Function to init / allocate the TWI module
 */
uint32_t drv_AD5245_init(drv_AD5245_init_t * p_params)
{
    NULL_PARAM_CHECK(p_params);

    m_AD5245.init.p_twi_cfg      = p_params->p_twi_cfg;
    m_AD5245.init.p_twi_instance = p_params->p_twi_instance;
    m_AD5245.initialized = true;
    m_AD5245.enabled     = false;
    //m_AD5245.evt_sheduled = 0;

    // Check correct ID.
//    if (!drv_AD5245_verify_id())
//    {
//        return DRV_AD5245_STATUS_WRONG_DEVICE;
//    }

    return NRF_SUCCESS;
}


int drv_AD5245_write(unsigned char slave_addr, unsigned char instruction_byte, unsigned char const * p_data)
{
    uint32_t err_code;
    uint8_t buffer[2];
    buffer[0] = instruction_byte;
    memcpy(&buffer[1], p_data, 1);

    err_code = twi_open();
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_twi_tx( m_AD5245.init.p_twi_instance,
                               slave_addr,
                               buffer,
                               2,
                               false);
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("drv_AD5245_write Failed!\r\n");
    }

    err_code = twi_close();
    APP_ERROR_CHECK(err_code);

    return 0;
}


int drv_AD5245_read(unsigned char slave_addr, unsigned char * data)
{
    uint32_t err_code;

    err_code = twi_open();
    APP_ERROR_CHECK(err_code);

//    err_code = nrf_drv_twi_tx( m_AD5245.init.p_twi_instance,
//                               slave_addr,
//                               &reg_addr,
//                               1,
//                               true );
//    if (err_code != NRF_SUCCESS)
//    {
//        NRF_LOG_ERROR("drv_AD5245_read Failed!\r\n");
//    }

    err_code = nrf_drv_twi_rx( m_AD5245.init.p_twi_instance,
                               slave_addr,
                               data,
                               1 );
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("drv_AD5245_read Failed!\r\n");
    }

    err_code = twi_close();
    APP_ERROR_CHECK(err_code);

    return 0;
}

