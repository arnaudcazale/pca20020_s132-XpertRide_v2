#ifndef FSM_REINS_H__
#define FSM_REINS_H__

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "command_Interpreter.h"
#include "nrf_log.h"

typedef enum {
    IDLE,
    TRIGGER_FROM_HORSE,
    TRIGGER_FROM_RIDER,
} fsm_state_one_side_t;

typedef enum {
    IDLE_LEFT,
    IDLE_RIGHT,
    IDLE_BOTH,
    TRIGGER_FROM_HORSE_LEFT,
    TRIGGER_FROM_HORSE_RIGHT,
    TRIGGER_FROM_HORSE_BOTH,
    TRIGGER_FROM_RIDER_LEFT,
    TRIGGER_FROM_RIDER_RIGHT,
    TRIGGER_FROM_RIDER_BOTH,
} fsm_state_both_side_t;




void FSM_init(void);
fsm_state_both_side_t FSM_update(FSRSensor_TypeDef * FSRSensors, uint8_t * arg);
fsm_state_both_side_t FSM_update_force(FSRSensor_TypeDef * FSRSensors);
fsm_state_both_side_t FSM_update_force_calculated(FSRSensor_TypeDef * FSRSensors);
fsm_state_both_side_t FSM_update_voltage(FSRSensor_TypeDef * FSRSensors);

#endif // FSM_REINS_H__