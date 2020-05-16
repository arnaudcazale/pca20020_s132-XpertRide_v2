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

    return NRF_SUCCESS;
}


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


int drv_ADG728_write(unsigned char slave_addr, unsigned char reg_addr, unsigned char length, unsigned char const * p_data)
{
    uint32_t err_code;
    uint8_t buffer[length+1];
    buffer[0] = reg_addr;
    memcpy(&buffer[1], p_data, length);

    err_code = twi_open();
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_twi_tx( m_ADG728.init.p_twi_instance,
                               slave_addr,
                               buffer,
                               length + 1,
                               false);
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("drv_ADG728_write Failed!\r\n");
    }

    err_code = twi_close();
    APP_ERROR_CHECK(err_code);

    return 0;
}


int drv_ADG728_read(unsigned char slave_addr, unsigned char reg_addr, unsigned char length, unsigned char * data)
{
    uint32_t err_code;

    err_code = twi_open();
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_twi_tx( m_ADG728.init.p_twi_instance,
                               slave_addr,
                               &reg_addr,
                               1,
                               true );
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("drv_ADG728_read Failed!\r\n");
    }

    err_code = nrf_drv_twi_rx( m_ADG728.init.p_twi_instance,
                               slave_addr,
                               data,
                               length );
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("drv_ADG728_read Failed!\r\n");
    }

    err_code = twi_close();
    APP_ERROR_CHECK(err_code);

    return 0;
}


/*******************************************************************************
* Function Name  : select_sensor
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void select_sensor ()
{
		
}

/*******************************************************************************
* Function Name  : select_sensor
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void set_gauge_resistor ()
{
		
    
}