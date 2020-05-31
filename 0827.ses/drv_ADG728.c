#include "drv_ADG728.h"
#include "twi_manager.h"
#define  NRF_LOG_MODULE_NAME "drv_ADG728   "
#include "nrf_log.h"
#include "macros_common.h"

static struct
{
    drv_ADG728_init_t        init;             ///< TWI configuration.
//    void                      (*cb)(void);      ///< Callback. Invoked when a pin interrupt is caught by GPIOTE.
    bool                      initialized;      ///< Module initialized.
    bool                      int_registered;   ///<
    bool                      enabled;          ///< Driver enabled.
//    uint32_t                  evt_sheduled;     ///< Number of scheduled events.
} m_ADG728 = {.initialized = false, .int_registered = false};


//void twi_event_handler(nrf_drv_twi_evt_t * p_event, void * p_context)
//{
//      NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"NRF_DRV_TWI_EVT_DONE %d", p_event->type);
////    if ((p_event->type == NRF_DRV_TWI_EVT_DONE) && 
////    (p_event->xfer_desc.type == NRF_DRV_TWI_XFER_TXRX))
////    { xfer_completed = true; } 
//       if ((p_event->type == NRF_DRV_TWI_EVT_DONE))
//       {
//          NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"NRF_DRV_TWI_EVT_DONE");
//       }
//
//}


/**@brief Function to init / allocate the TWI module
 */
static __inline uint32_t twi_open(void)
{
    uint32_t err_code;

    err_code = twi_manager_request(m_ADG728.init.p_twi_instance,
                                   m_ADG728.init.p_twi_cfg,
                                   NULL,
                                   NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(m_ADG728.init.p_twi_instance);

    return NRF_SUCCESS;
}


/**@brief Function to deinit the TWI module when this driver does not need to
 *        communicate on the TWI bus, so that other drivers can use the module.
 */
static __inline uint32_t twi_close(void)
{
    nrf_drv_twi_disable(m_ADG728.init.p_twi_instance);

    nrf_drv_twi_uninit(m_ADG728.init.p_twi_instance);

    return NRF_SUCCESS;
}

/**@brief Read and verify accelerometer ID.
 */
//static ret_code_t drv_ADG728_verify_id(void)
//{
//    ret_code_t  err_code;
//    uint8_t     reg_val;
//
//    err_code = drv_ADG728_read(ADG728_1_ADDR, WHO_AM_I, 1, &reg_val);
//    RETURN_IF_ERROR(err_code);
//
//    return (reg_val == I_AM_ADG728);
//}

/**@brief Function to init / allocate the TWI module
 */
uint32_t drv_ADG728_init(drv_ADG728_init_t * p_params)
{
    NULL_PARAM_CHECK(p_params);

    m_ADG728.init.p_twi_cfg      = p_params->p_twi_cfg;
    m_ADG728.init.p_twi_instance = p_params->p_twi_instance;
    m_ADG728.initialized = true;
    m_ADG728.enabled     = false;
    //m_ADG728.evt_sheduled = 0;

    // Check correct ID.
//    if (!drv_ADG728_verify_id())
//    {
//        return DRV_ADG728_STATUS_WRONG_DEVICE;
//    }

    return NRF_SUCCESS;
}


int drv_ADG728_write(unsigned char slave_addr, unsigned char const * p_data)
{
    uint32_t err_code;
    
    uint8_t buffer[1];
    memcpy(&buffer[0], p_data, 1);

    //NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"data = %d\r\n", buffer[0]);

    err_code = twi_open();
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_twi_tx( m_ADG728.init.p_twi_instance,
                               slave_addr,
                               buffer,
                               1,
                               false);
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("drv_ADG728_write Failed!\r\n");
    }

    err_code = twi_close();
    APP_ERROR_CHECK(err_code);

    return 0;
}


int drv_ADG728_read(unsigned char slave_addr, unsigned char * data)
{
    uint32_t err_code;

    err_code = twi_open();
    APP_ERROR_CHECK(err_code);

//    err_code = nrf_drv_twi_tx( m_ADG728.init.p_twi_instance,
//                               slave_addr,
//                               NULL,//&reg_addr,
//                               0,
//                               true);
//    if (err_code != NRF_SUCCESS)
//    {
//        NRF_LOG_ERROR("drv_ADG728_read Failed! = %d\r\n", err_code);
//    }

    err_code = nrf_drv_twi_rx( m_ADG728.init.p_twi_instance,
                               slave_addr,
                               data,
                               1 );
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("drv_ADG728_read Failed!\r\n");
    }

    err_code = twi_close();
    APP_ERROR_CHECK(err_code);

    return 0;
}


