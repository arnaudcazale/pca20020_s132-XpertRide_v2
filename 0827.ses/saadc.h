#ifndef SAADC_H__
#define SAADC_H__

#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#include "nrf_log.h"
#include "nrf_drv_saadc.h"
#include "ble_tms.h"
#include "command_interpreter.h"

enum
{
    M_SAADC_STATUS_CODE_SUCCESS,                     ///< Successfull
    M_SAADC_STATUS_CODE_INVALID_PARAM,               ///< Invalid parameters
};

extern bool saadc_busy;

void get_handle(ble_tms_t * p_tms);
void start_ADC(uint8_t machine_state, ble_tms_t *, uint16_t sampling_period, char * arg, float expected_force);
void stop_ADC(uint8_t machine_state);
static void app_timer_periodic_handler(void * p_context);
static uint32_t adc_configure(void);
void saadc_event_handler(nrf_drv_saadc_evt_t const * p_event);
static uint32_t adc_to_batt_voltage(uint32_t adc_val, uint16_t * voltage);
static uint32_t adc_gain_enum_to_real_gain(nrf_saadc_gain_t gain_reg, float * real_val);
static void dispatch_ADC_results(void);
static uint32_t adc_calibrate(void);


#endif // SAAD_H__