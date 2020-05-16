#include "saadc.h"
#include "app_timer.h"
#include "app_scheduler.h"
#include "nrf_log.h"
#include "nrf_drv_saadc.h"
#include "math.h"
#include "pca20020.h"

#include "macros_common.h"

//#include "command_interpreter.h"




static uint32_t                m_adc_evt_counter;

//APP_TIMER_DEF(FSR_timer_id_debug);
APP_TIMER_DEF(FSR_timer_id_start);
APP_TIMER_DEF(FSR_timer_id_single);
#define ADC_GAIN                    NRF_SAADC_GAIN1_6     // ADC gain.
#define ADC_REFERENCE_VOLTAGE       (0.6f)              // The standard internal ADC reference voltage.
#define ADC_RESOLUTION_BITS         (8 + (SAADC_CONFIG_RESOLUTION * 2)) //ADC resolution [bits].
#define ADC_BUF_SIZE                (1)                 // Size of each ADC buffer.
static nrf_saadc_value_t            m_buffer[ADC_BUF_SIZE];
static nrf_saadc_value_t            buffer_adc[4];
static uint16_t                     buffer_voltage[4];
static volatile bool                m_adc_cal_in_progress;          //
//static batt_meas_param_t            m_batt_meas_param;
uint8_t channel = 0;
uint16_t cpt = 0;

static ble_tms_t        * m_tms; //pointer to handle for writing characteristic

static uint8_t m_machine_state;
static uint8_t * m_arg;
static float m_expected_force;
bool flag_write_tare_consecutive = false;

extern FSRSensor_TypeDef FSRSensors[NUMBER_OF_SENSORS];
ble_tms_FSR_data_t FSR_data;
//static ble_tms_FSR_data_voltage_packet_t packet_FSR_data_voltage;
static ble_tms_FSR_data_force_packet_t packet_FSR_data_force;
static ble_tms_FSR_data_force_calculated_packet_t packet_FSR_data_force_calculated;
static ble_tms_command_debug_packet_t packet_debug;
static ble_tms_command_single_voltage_packet_t packet_single_voltage;
static ble_tms_command_single_force_packet_t packet_single_force;
static ble_tms_command_single_force_calculated_packet_t packet_single_force_calculated;
static ble_tms_command_tare_multi_packet_t packet_tare_multi;

/**@brief Function for start ADC.
 *
 */
void start_ADC(uint8_t machine_state, ble_tms_t * tms, uint16_t sampling_period, uint8_t * arg, float expected_force)
{
    m_tms = tms;

    m_machine_state = machine_state;
    m_arg = arg;
    m_expected_force = expected_force;


    uint32_t err_code;

    err_code = app_timer_create(&FSR_timer_id_start, APP_TIMER_MODE_REPEATED, app_timer_periodic_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&FSR_timer_id_single, APP_TIMER_MODE_SINGLE_SHOT, app_timer_periodic_handler);
    APP_ERROR_CHECK(err_code);

    adc_calibrate();

    //Init Pin as output
    nrf_gpio_cfg_output(ANA_DIG1);
    nrf_gpio_cfg_output(ANA_DIG2);
    
    switch(m_machine_state){
      case START:
        NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"START ADC FOR 'START', sampling period: %d \r\n", sampling_period);
         /**@brief Init application timers */

        err_code = app_timer_start(FSR_timer_id_start,
                                   APP_TIMER_TICKS(sampling_period),
                                   NULL);
        APP_ERROR_CHECK(err_code);
      break;
      case DEBUG:
        if(strcmp(m_arg,"CONT")==0){
          NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"START ADC FOR 'DEBUG CONTINOUS'");
          err_code = app_timer_start(FSR_timer_id_start,
                                     APP_TIMER_TICKS(100),
                                     NULL);
          APP_ERROR_CHECK(err_code);
        }else{
          NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"START ADC FOR 'DEBUG SINGLE'");
          err_code = app_timer_start(FSR_timer_id_single,
                                     APP_TIMER_TICKS(20),
                                     NULL);
          APP_ERROR_CHECK(err_code);
        }
      break;
      case SINGLE:
          NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"START ADC FOR 'DEBUG SINGLE'");
          err_code = app_timer_start(FSR_timer_id_single,
                                     APP_TIMER_TICKS(20),
                                     NULL);
          APP_ERROR_CHECK(err_code);
      break;
      case TARE:
          NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"START ADC FOR 'TARE'");
          err_code = app_timer_start(FSR_timer_id_single,
                                     APP_TIMER_TICKS(20),
                                     NULL);
          APP_ERROR_CHECK(err_code);
      break;
      case CALW:
          NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"START ADC FOR 'CALW");
          err_code = app_timer_start(FSR_timer_id_single,
                                     APP_TIMER_TICKS(20),
                                     NULL);
          APP_ERROR_CHECK(err_code);
      break;
    }  
}

/**@brief Function for stop ADC.
 *
 */
void stop_ADC(uint8_t machine_state)
{
  NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"STOP ADC %d", machine_state);
  uint32_t err_code;
  err_code = app_timer_stop(FSR_timer_id_start);
}

/**@brief Function 
 *
 */
static void adc_configure()
{
  uint32_t err_code;

  nrf_drv_saadc_config_t saadc_config = NRF_DRV_SAADC_DEFAULT_CONFIG;
  err_code = nrf_drv_saadc_init(&saadc_config, saadc_event_handler);
  APP_ERROR_CHECK(err_code);
  nrf_saadc_channel_config_t channel_config = 
  NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN0);
  /* Burst enabled to oversample the SAADC. */
  channel_config.burst    = NRF_SAADC_BURST_ENABLED;
  channel_config.acq_time = NRF_SAADC_ACQTIME_40US;
  err_code = nrf_drv_saadc_channel_init(0, &channel_config);
  APP_ERROR_CHECK(err_code);
  err_code = nrf_drv_saadc_buffer_convert(m_buffer, ADC_BUF_SIZE);
  APP_ERROR_CHECK(err_code);
}

/**@brief Function 
 *
 */
static void app_timer_periodic_handler(void * p_context)
{
    //NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"EVERY TIME TIMER HANDLER \r\n");
    UNUSED_PARAMETER(p_context);
    
    if(channel == 0)
    {
        //NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"First TIMER HANDLER \r\n");
        setMUXChannel(0);
        adc_configure();
        ret_code_t err_code;
        err_code = nrf_drv_saadc_sample();
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function 
 *
 */
void saadc_event_handler(nrf_drv_saadc_evt_t const * p_event)
{
    if (p_event->type == NRF_DRV_SAADC_EVT_CALIBRATEDONE)
    {
        m_adc_cal_in_progress = false;
        nrf_drv_saadc_uninit();
    }

    if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
    {
    //NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"SAAC HANDLER \r\n");
    ret_code_t err_code;

      switch(channel)
      {
        case 0:
          buffer_adc[channel] = *m_buffer;
          setMUXChannel(1);
          nrf_drv_saadc_uninit();
          adc_configure();
          channel = 1;
          err_code = nrf_drv_saadc_sample();
          APP_ERROR_CHECK(err_code);
        break;
        case 1:
          buffer_adc[channel] = *m_buffer;
          setMUXChannel(2);
          nrf_drv_saadc_uninit();
          adc_configure();
          channel = 2;
          err_code = nrf_drv_saadc_sample();
          APP_ERROR_CHECK(err_code);
        break;
         case 2:
          buffer_adc[channel] = *m_buffer;
          setMUXChannel(3);
          nrf_drv_saadc_uninit();
          adc_configure();
          channel = 3;
          err_code = nrf_drv_saadc_sample();
          APP_ERROR_CHECK(err_code);
        break;
        case 3:
          buffer_adc[channel] = *m_buffer;
          setMUXChannel(0);
          nrf_drv_saadc_uninit();
          channel = 0;
          dispatch_ADC_results();
          //NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"SAAC BEFORE %d %d %d %d\r\n", buffer_adc[0], buffer_adc[1], buffer_adc[2], buffer_adc[3] );
        break;
      }
    }
}

/**@brief Function 
 *
 */
static uint32_t adc_to_batt_voltage(uint32_t adc_val, uint16_t * voltage)
{
    uint32_t err_code;
    float    adc_gain;
    uint16_t tmp_voltage;

    err_code = adc_gain_enum_to_real_gain(ADC_GAIN, &adc_gain);
    RETURN_IF_ERROR(err_code);

    //NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"adc_val: %d \r\n", adc_val);
    float tmp = adc_val * (( ADC_REFERENCE_VOLTAGE / adc_gain ) / pow(2, ADC_RESOLUTION_BITS));
    //NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"converted float" NRF_LOG_FLOAT_MARKER "\r\n", NRF_LOG_FLOAT(tmp));
    *voltage =  (uint16_t) (tmp * 1000);
//    *voltage = ( (tmp_voltage + 5) / 10) * 10;  // Round the value.

    return M_BATT_STATUS_CODE_SUCCESS;
}


/**@brief Function 
 *
 */
static uint32_t adc_gain_enum_to_real_gain(nrf_saadc_gain_t gain_reg, float * real_val)
{
    switch(gain_reg)
    {
        case NRF_SAADC_GAIN1_6: *real_val = 1 / (float)6;
        break;
        case NRF_SAADC_GAIN1_5: *real_val = 1 / (float)5;
        break;
        case NRF_SAADC_GAIN1_4: *real_val = 1 / (float)4;
        break;
        case NRF_SAADC_GAIN1_3: *real_val = 1 / (float)3;
        break;
        case NRF_SAADC_GAIN1_2: *real_val = 1 / (float)2;
        break;
        case NRF_SAADC_GAIN1:   *real_val = 1;
        break;
        case NRF_SAADC_GAIN2:   *real_val = 2;
        break;
        case NRF_SAADC_GAIN4:   *real_val = 3;
        break;
        default: return M_BATT_STATUS_CODE_INVALID_PARAM;
    };

    return M_BATT_STATUS_CODE_SUCCESS;
}


static void dispatch_ADC_results()
{
  //NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"SAAC BEFORE %d %d %d \r\n", buffer_adc[0], buffer_adc[1], buffer_adc[2] );
  ret_code_t err_code;

  for(int i=0; i<sizeof(buffer_adc)/sizeof(buffer_adc[0]); i++)
  {
    // Avoid negative values due to noise (-1 lead to 65535 when cast to uint16_t)
    if(buffer_adc[i] < 0){
      buffer_adc[i] = 0;
    }

    uint16_t voltage;
    err_code = adc_to_batt_voltage(buffer_adc[i], &voltage);
    APP_ERROR_CHECK(err_code);

    buffer_voltage[i] = voltage;
  }

  for(int sensor_index=0; sensor_index<NUMBER_OF_SENSORS; sensor_index++){
    FSRSensors[sensor_index].sensor_number = sensor_index + 1;
    FSRSensors[sensor_index].ADCValue = buffer_adc[sensor_index];
    FSRSensors[sensor_index].voltage = buffer_voltage[sensor_index];
  }

//  NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"SAAC AFTER %d %d %d %d \r\n", buffer_adc[0], buffer_adc[1], buffer_adc[2], buffer_adc[3] );
  NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"SAAC BUFFER VOLTAGE %d %d %d %d \r\n", buffer_voltage[0], buffer_voltage[1], buffer_voltage[2], buffer_voltage[3] );
  

  uint8_t ind_sensor;

  switch(m_machine_state){
    case START:
        for(int sensor_index=0; sensor_index<NUMBER_OF_SENSORS; sensor_index++)
        {
            processForceData(&FSRSensors[sensor_index]);
            //packet_FSR_data_voltage.voltage[sensor_index] = FSRSensors[sensor_index].voltage; 
            packet_FSR_data_force.force[sensor_index] = FSRSensors[sensor_index].force;
            packet_FSR_data_force_calculated.force_calculated[sensor_index] = FSRSensors[sensor_index].force_calculated;
        }
        if(m_arg == 0){
            (void)ble_tms_FSR_data_force_calculated_set(m_tms, &packet_FSR_data_force_calculated);
        }else{
            if(strcmp(m_arg,"V")==0) {
                FSR_data.FSR1 = FSRSensors[0].voltage; //buffer_voltage[0];
                FSR_data.FSR2 = FSRSensors[1].voltage; //buffer_voltage[1];
                FSR_data.FSR3 = FSRSensors[2].voltage; //buffer_voltage[2];
                FSR_data.FSR4 = FSRSensors[3].voltage; //buffer_voltage[2];
                (void)ble_tms_FSR_data_set(m_tms, &FSR_data);
            }else if(strcmp(m_arg,"F")==0) {
                (void)ble_tms_FSR_data_force_set(m_tms, &packet_FSR_data_force);
            }else if(strcmp(m_arg,"FC")==0) {
               (void)ble_tms_FSR_data_force_calculated_set(m_tms, &packet_FSR_data_force_calculated);
            }else{
                (void)ble_tms_FSR_data_force_calculated_set(m_tms, &packet_FSR_data_force_calculated);
            }
        }
        
    break;
    case DEBUG:
        for(int sensor_index=0; sensor_index<NUMBER_OF_SENSORS; sensor_index++)
        {
          processForceData(&FSRSensors[sensor_index]);
          packet_debug.packet[sensor_index].sensor_number = FSRSensors[sensor_index].sensor_number;
          packet_debug.packet[sensor_index].ADCValue = FSRSensors[sensor_index].ADCValue;
          packet_debug.packet[sensor_index].voltage = FSRSensors[sensor_index].voltage;
          packet_debug.packet[sensor_index].force = FSRSensors[sensor_index].force;
          packet_debug.packet[sensor_index].force_calculated = FSRSensors[sensor_index].force_calculated;

//          printf("sensor %d \n", sensor_index);
//          printf("ADCValue %d \n", FSRSensors[sensor_index].ADCValue);
//          printf("voltage %d \n", FSRSensors[sensor_index].voltage);
//          printf("force %f \n", FSRSensors[sensor_index].force);
//          printf("force_calculated %f \n", FSRSensors[sensor_index].force_calculated);
        }
        (void)ble_tms_command_debug_set(m_tms, &packet_debug);
    break;
    case SINGLE:
        for(int sensor_index=0; sensor_index<NUMBER_OF_SENSORS; sensor_index++)
        {
            processForceData(&FSRSensors[sensor_index]);
            packet_single_voltage.voltage[sensor_index] = FSRSensors[sensor_index].voltage;
            packet_single_force.force[sensor_index] = FSRSensors[sensor_index].force;
            packet_single_force_calculated.force_calculated[sensor_index] = FSRSensors[sensor_index].force_calculated;

//            printf("sensor %d \n", sensor_index);
//            printf("voltage %d \n", FSRSensors[sensor_index].voltage);
//            printf("force %f \n", FSRSensors[sensor_index].force);
//            printf("force_calculated %f \n", FSRSensors[sensor_index].force_calculated);
        }
        
        if(m_arg == 0)
        {
            (void)ble_tms_command_single_force_calculated_set(m_tms, &packet_single_force_calculated);
        }else{
            if(strcmp(m_arg,"V")==0) {
                (void)ble_tms_command_single_voltage_set(m_tms, &packet_single_voltage);
            }else if(strcmp(m_arg,"F")==0) {
                (void)ble_tms_command_single_force_set(m_tms, &packet_single_force);
            }else if(strcmp(m_arg,"FC")==0) {
                (void)ble_tms_command_single_force_calculated_set(m_tms, &packet_single_force_calculated);
            }else{
                (void)ble_tms_command_single_force_calculated_set(m_tms, &packet_single_force_calculated);
            }
        }
    break;
    case TARE:   
      if(m_arg==-1)
      {
          ind_sensor = m_arg;
//        NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"TARE ALL SENSORS \r\n");
          for(ind_sensor = 0; ind_sensor<NUMBER_OF_SENSORS; ind_sensor++){
              processForceData(&FSRSensors[ind_sensor]);
              FSRSensors[ind_sensor].offset= FSRSensors[ind_sensor].force;
              packet_tare_multi.offset[ind_sensor] = FSRSensors[ind_sensor].offset;
          }
          //write to flash consecutive
          flag_write_tare_consecutive = true;
          write_fds_offset_consecutive(0);
          //gatt_set
          (void)ble_tms_command_tare_multi_set(m_tms, &packet_tare_multi);
      }else {
          ind_sensor = m_arg;
//        NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"TARE SENSOR %d \r\n", m_arg);
          processForceData(&FSRSensors[ind_sensor]);
//        printf("sensor %d \n", ind_sensor);
//        printf("ADCValue %d \n", FSRSensors[ind_sensor].ADCValue);
//        printf("voltage %d \n", FSRSensors[ind_sensor].voltage);
//        printf("coeff: %f - %f - %f - %f - %f \n", FSRSensors[ind_sensor].coefficients[0],
//                                                   FSRSensors[ind_sensor].coefficients[1],
//                                                   FSRSensors[ind_sensor].coefficients[2],
//                                                   FSRSensors[ind_sensor].coefficients[3],
//                                                   FSRSensors[ind_sensor].coefficients[4]);
//        printf("offset %f \n", FSRSensors[ind_sensor].offset);
//        printf("cal_ref %f \n", FSRSensors[ind_sensor].cal_ref);
//        printf("force %f \n", FSRSensors[ind_sensor].force);
//        printf("force_calculated %f \n", FSRSensors[ind_sensor].force_calculated);
          FSRSensors[ind_sensor].offset= FSRSensors[ind_sensor].force;
          //write to Flash
          write_fds_offset_consecutive(ind_sensor);
          //gatt_set
          (void)ble_tms_command_tare_single_set(m_tms, &FSRSensors[ind_sensor].offset);
      }
    break;
  case CALW:   
      ind_sensor = m_arg;
      printf("ind_sensor %d \n", m_arg);
      printf("expected_force %f \n", m_expected_force);
      processForceData(&FSRSensors[ind_sensor]); 
      float current_force = FSRSensors[ind_sensor].force;
      printf("current_force %f \n", current_force);
      if(current_force > 0){
        FSRSensors[ind_sensor].cal_ref= m_expected_force/current_force;
      }
      printf("cal_ref %f \n", FSRSensors[ind_sensor].cal_ref);
      write_fds_cal_ref_consecutive(ind_sensor);
    break;
   }  
}

void adc_calibrate(void){
    uint32_t err_code;

    nrf_drv_saadc_config_t saadc_config = NRF_DRV_SAADC_DEFAULT_CONFIG;

    err_code = nrf_drv_saadc_init(&saadc_config, saadc_event_handler);
    RETURN_IF_ERROR(err_code);

    m_adc_cal_in_progress = true;
    err_code = nrf_drv_saadc_calibrate_offset();
    RETURN_IF_ERROR(err_code);

    while(m_adc_cal_in_progress)
    {
        /* Wait for SAADC calibration to finish. */
    }
}
 
void setMUXChannel(uint8_t channel)
{
  uint32_t err_code;

  switch(channel){
    case 0:
      nrf_gpio_pin_clear(ANA_DIG1);
      nrf_gpio_pin_clear(ANA_DIG2);
      break;
    case 1:
      nrf_gpio_pin_set(ANA_DIG1);
      nrf_gpio_pin_clear(ANA_DIG2);
      break;
    case 2:
      nrf_gpio_pin_clear(ANA_DIG1);
      nrf_gpio_pin_set(ANA_DIG2);
      break;
    case 3:
      nrf_gpio_pin_set(ANA_DIG1);
      nrf_gpio_pin_set(ANA_DIG2);
      break;
  }
  
}

 