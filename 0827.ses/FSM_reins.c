# include "FSM_reins.h"

static fsm_state_one_side_t fsm_state_left;
static fsm_state_one_side_t fsm_state_right;
static fsm_state_both_side_t fsm_state_both;
static uint16_t trigger_threshold_voltage = 100;
static float trigger_threshold_force = 0.1;
static float trigger_threshold_force_calculated = 0.1;

extern FSRSensor_TypeDef FSRSensors[NUMBER_OF_SENSORS];

void FSM_init()
{
    fsm_state_both = IDLE_BOTH;
}

fsm_state_both_side_t FSM_update(FSRSensor_TypeDef * FSRSensors, uint8_t * arg)
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

fsm_state_both_side_t FSM_update_force(FSRSensor_TypeDef * FSRSensors)
{

}

fsm_state_both_side_t FSM_update_voltage(FSRSensor_TypeDef * FSRSensors)
{
  //NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN" FSRSensors.voltage %d %d %d %d ", FSRSensors[0].voltage, FSRSensors[1].voltage, FSRSensors[2].voltage, FSRSensors[3].voltage);
  //NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN" FSRSensors.voltage %d %d %d %d \r\n", FSRSensors[4].voltage, FSRSensors[5].voltage, FSRSensors[6].voltage, FSRSensors[7].voltage);

          //Si Rider Force from left
          if( (FSRSensors[0].voltage > trigger_threshold_voltage) && (FSRSensors[2].voltage < trigger_threshold_voltage) )
          {
            fsm_state_left = TRIGGER_FROM_RIDER;
          }
          // si 0 Force
          if ( (FSRSensors[0].voltage < trigger_threshold_voltage) && (FSRSensors[2].voltage < trigger_threshold_voltage) ) 
          {
            fsm_state_left = IDLE;
          }
          // Si Rider Force from right
          if( (FSRSensors[3].voltage > trigger_threshold_voltage) && (FSRSensors[6].voltage < trigger_threshold_voltage) )
          {
            fsm_state_right = TRIGGER_FROM_RIDER;
          }
          // si 0 Force
          if ( (FSRSensors[3].voltage < trigger_threshold_voltage) && (FSRSensors[6].voltage < trigger_threshold_voltage) ) 
          {
            fsm_state_right = IDLE;
          }
          // Si Cheval Force from left
          if( (FSRSensors[0].voltage < trigger_threshold_voltage) && (FSRSensors[2].voltage > trigger_threshold_voltage) )
          {
            fsm_state_left = TRIGGER_FROM_HORSE;
          }
          // si 0 Force
          if ( (FSRSensors[0].voltage < trigger_threshold_voltage) && (FSRSensors[2].voltage < trigger_threshold_voltage) ) 
          {
            fsm_state_left = IDLE;
          }
          // Si Cheval Force from right
          if( (FSRSensors[3].voltage < trigger_threshold_voltage) && (FSRSensors[6].voltage > trigger_threshold_voltage) )
          {
            fsm_state_right = TRIGGER_FROM_HORSE;
          }
          // si 0 Force
          if ( (FSRSensors[3].voltage < trigger_threshold_voltage) && (FSRSensors[6].voltage < trigger_threshold_voltage) ) 
          {
            fsm_state_right = IDLE;
          }

          //NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"fsm_state_left = %d , fsm_state_right  = %d \n", fsm_state_left, fsm_state_right);
          //Check status
          if( (fsm_state_left == TRIGGER_FROM_RIDER) && (fsm_state_right == TRIGGER_FROM_RIDER) )
          {
            fsm_state_both = TRIGGER_FROM_RIDER_BOTH;
          }
          if( (fsm_state_left == TRIGGER_FROM_RIDER) && (fsm_state_right == IDLE) )
          {
            fsm_state_both = TRIGGER_FROM_RIDER_LEFT;
          }
          if( (fsm_state_right == TRIGGER_FROM_RIDER) && (fsm_state_left == IDLE) )
          {
            fsm_state_both = TRIGGER_FROM_RIDER_RIGHT;
          }
          if( (fsm_state_right == IDLE) && (fsm_state_left == IDLE) )
          {
            fsm_state_both = IDLE_BOTH;
          }
          if( (fsm_state_left == TRIGGER_FROM_HORSE) && (fsm_state_right == TRIGGER_FROM_HORSE) )
          {
            fsm_state_both = TRIGGER_FROM_HORSE_BOTH;
          }
          if( (fsm_state_left == TRIGGER_FROM_HORSE) && (fsm_state_right == IDLE) )
          {
            fsm_state_both = TRIGGER_FROM_HORSE_LEFT;
          }
          if( (fsm_state_right == TRIGGER_FROM_HORSE) && (fsm_state_left == IDLE) )
          {
            fsm_state_both = TRIGGER_FROM_HORSE_RIGHT;
          }
          if( (fsm_state_left == IDLE) && (fsm_state_right == IDLE) )
          {
            fsm_state_both = IDLE_BOTH;
          }

    return fsm_state_both;

}

fsm_state_both_side_t FSM_update_force_calculated(FSRSensor_TypeDef * FSRSensors)
{

}