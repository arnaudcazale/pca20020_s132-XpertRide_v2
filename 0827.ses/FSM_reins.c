# include "FSM_reins.h"

static fsm_state_t fsm_state;
static uint16_t trigger_threshold_voltage = 100;
static float trigger_threshold_force = 0.1;
static float trigger_threshold_force_calculated = 0.1;

extern FSRSensor_TypeDef FSRSensors[NUMBER_OF_SENSORS];

void FSM_init()
{
    fsm_state = IDLE;
}

fsm_state_t FSM_update(FSRSensor_TypeDef * FSRSensors, uint8_t * arg)
{
  if(arg == 0)
  {
    return FSM_update_force_calculated(FSRSensors);
  }else {
    if(strcmp(arg,"V")==0) 
    {
        return FSM_update_voltage(FSRSensors);

    }else if(strcmp(arg,"F")==0) 
    {
       return FSM_update_force(FSRSensors);

    }else if(strcmp(arg,"FC")==0) 
    {
       return FSM_update_force_calculated(FSRSensors);

    }else
    {
       return FSM_update_force_calculated(FSRSensors);
    }
  }
    
}

fsm_state_t FSM_update_force(FSRSensor_TypeDef * FSRSensors)
{
  switch(fsm_state)
    {
      case IDLE:
          
        break;

      case TRIGGER_FROM_HORSE:
          
        break;

      case TRIGGER_FROM_RIDER:
          
        break;
    }
}

fsm_state_t FSM_update_voltage(FSRSensor_TypeDef * FSRSensors)
{
  //NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN" FSRSensors.voltage %d %d %d %d ", FSRSensors[0].voltage, FSRSensors[1].voltage, FSRSensors[2].voltage, FSRSensors[3].voltage);
  //NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN" FSRSensors.voltage %d %d %d %d \r\n", FSRSensors[4].voltage, FSRSensors[5].voltage, FSRSensors[6].voltage, FSRSensors[7].voltage);
  switch(fsm_state)
    {
      case IDLE:
          // Si Rider Force
          if( (FSRSensors[0].voltage > trigger_threshold_voltage) && (FSRSensors[2].voltage < trigger_threshold_voltage) )
          {
            fsm_state = TRIGGER_FROM_RIDER;
          }else if ( (FSRSensors[0].voltage < trigger_threshold_voltage) && (FSRSensors[2].voltage > trigger_threshold_voltage) ) // si Cheval Force
          {
            fsm_state = TRIGGER_FROM_HORSE;
          }
        break;

      case TRIGGER_FROM_HORSE:
          // Si Rider Force
          if( (FSRSensors[0].voltage > trigger_threshold_voltage) && (FSRSensors[2].voltage < trigger_threshold_voltage) )
          {
            fsm_state = TRIGGER_FROM_RIDER;
          }else if ( (FSRSensors[0].voltage < trigger_threshold_voltage) && (FSRSensors[2].voltage < trigger_threshold_voltage) ) // si 0 Force
          {
            fsm_state = IDLE;
          }
        break;

      case TRIGGER_FROM_RIDER:
          // Si Cheval Force
          if( (FSRSensors[0].voltage < trigger_threshold_voltage) && (FSRSensors[2].voltage > trigger_threshold_voltage) )
          {
            fsm_state = TRIGGER_FROM_HORSE;
          }else if ( (FSRSensors[0].voltage < trigger_threshold_voltage) && (FSRSensors[2].voltage < trigger_threshold_voltage) ) // si 0 Force
          {
            fsm_state = IDLE;
          }
        break;
    }

    return fsm_state;

}

fsm_state_t FSM_update_force_calculated(FSRSensor_TypeDef * FSRSensors)
{
switch(fsm_state)
    {
      case IDLE:
          
        break;

      case TRIGGER_FROM_HORSE:
          
        break;

      case TRIGGER_FROM_RIDER:
          
        break;
    }
}