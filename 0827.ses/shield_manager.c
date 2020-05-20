#include "shield_manager.h"
#include "nrf_log.h"
#include "nrf_delay.h"

static ret_code_t select_bridge_resistor(uint8_t line)
{
    if( (line > 8) || (line == 0) ){
      return SHIELD_MANAGER_STATUS_CODE_INVALID_PARAM;
    }
  
    uint8_t data;
    uint8_t data_read;

    data = 1 << (line-1);
    //NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"data_write = %d\r\n", data);
    drv_ADG728_write(ADG728_3_ADDR, &data);
    //drv_ADG728_read(ADG728_3_ADDR, &data_read);
    //NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"data_read = %d\r\n", data_read);

    return SHIELD_MANAGER_STATUS_CODE_SUCCESS;

}
    
static ret_code_t set_bridge_digiPot(uint8_t digiPot_value_to_set)
{
    uint8_t data;
    uint8_t data_read;
    uint8_t instruction_byte = 0;

    data = digiPot_value_to_set;
    //NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"data_write = %d\r\n", data);
    drv_AD5245_write(AD5245_ADDR, instruction_byte, &data);
//    drv_AD5245_read(AD5245_ADDR, &data_read);
//    NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"data_read = %d\r\n", data_read);

}

ret_code_t setMUXChannel(uint8_t sensor)
{
    if( (sensor > 8) || (sensor == 0) ){
      return SHIELD_MANAGER_STATUS_CODE_INVALID_PARAM;
    }
  
    uint8_t data;
    uint8_t data_read;

    data = 1 << (sensor-1);
   // NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"data_write = %d\r\n", data);
    drv_ADG728_write(ADG728_1_ADDR, &data);
    nrf_delay_ms(50);
    drv_ADG728_read(ADG728_1_ADDR, &data_read);
    //NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"data_read = %d\r\n", data_read);

    return SHIELD_MANAGER_STATUS_CODE_SUCCESS;

}

ret_code_t bridge_balancing(uint16_t resistor_value)
{
    if(resistor_value > 800){
      return SHIELD_MANAGER_STATUS_CODE_INVALID_PARAM;
    }
    uint8_t centaines = resistor_value / 100 % 10;
    //NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"centaines = %d\r\n", centaines);

    uint8_t line = centaines + 1;
    //NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"line = %d\r\n", line);

    float digiPot_value_to_set = (resistor_value - (centaines*100)) * 2.56;
    //NRF_LOG_INFO("Float " NRF_LOG_FLOAT_MARKER "\r\n", NRF_LOG_FLOAT(digiPot_value_to_set));
    uint8_t digiPot_value_to_set_uint8_t = (uint8_t)digiPot_value_to_set;
    //NRF_LOG_INFO("line = %d\r\n", digiPot_value_to_set_uint8_t);

    select_bridge_resistor(line);

    set_bridge_digiPot(digiPot_value_to_set_uint8_t);

    return SHIELD_MANAGER_STATUS_CODE_SUCCESS;
}

ret_code_t amplifier_gain_selection(uint8_t gain)
{
    if( (gain > 10) || (gain < 2)){
      return SHIELD_MANAGER_STATUS_CODE_INVALID_PARAM;
    }

    uint8_t data;

    switch (gain)
    {
      case 2:
        data = 00000001;
      break;
      case 3:
        data = 00000010;
      break;
       case 4:
        data = 00000100;
      break;
       case 5:
        data = 00001000;
      break;
       case 10:
        data = 10000000;
      break;
      default:
        data = 00000001;
      break;

    }
     
    drv_ADG728_write(ADG728_2_ADDR, &data);

   
    return SHIELD_MANAGER_STATUS_CODE_SUCCESS;
}

ret_code_t shield_manager_init(void)
{

    uint32_t err_code;
    err_code = setMUXChannel(1);
    APP_ERROR_CHECK(err_code);
   
    err_code = bridge_balancing(200);
    APP_ERROR_CHECK(err_code);

    if (err_code != NRF_SUCCESS)
    {
        return SHIELD_MANAGER_STATUS_CODE_INVALID_PARAM;
    }

    err_code = amplifier_gain_selection(2);
    APP_ERROR_CHECK(err_code);

    if (err_code != NRF_SUCCESS)
    {
        return SHIELD_MANAGER_STATUS_CODE_INVALID_PARAM;
    }
   
    return SHIELD_MANAGER_STATUS_CODE_SUCCESS;
}

