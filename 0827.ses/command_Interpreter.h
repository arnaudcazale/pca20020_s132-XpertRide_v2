#ifndef COMMAND_INTERPRETER_H__
#define COMMAND_INTERPRETER_H__

#include "ble.h"
#include "ble_tms.h"
#include "nrf_log.h"
#include "fds.h"
//#include "utils.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define MAX_NUMBER_OF_ARGUMENTS  15

#define DEBUG                   1
#define SINGLE                  2
#define START                   3
#define STOP                    4
#define WSN                     5
#define RSN                     6
#define TARE                    7
#define CALW                    8
#define RTARE                   9
#define RCALW                   10
#define WTYPE                   11
#define RTYPE     		12	              
#define WFACTLIN	        13
#define RFACTLIN                14
#define VER                     15
#define RESTORE                 16
#define NOTHING                 17

#define NUMBER_OF_SENSORS 4
#define SAMPLES_PER_SENSOR              20 //Number of samples per sensor. In order to filtering (smooth the ADC reading)

typedef struct
{
    char * command;
    uint8_t * args[MAX_NUMBER_OF_ARGUMENTS];
    uint8_t nbrArgs; 

}m_command_packet;

typedef struct
{
    uint8_t state;
    uint8_t state_old; 

}m_state_machine;

typedef struct
{
  uint8_t type;
  uint8_t serial_number [4];
  
}vSoleInfo_TypeDef;

typedef struct
{
    //uint32_t readings[SAMPLES_PER_SENSOR];      // the readings from the analog input
    uint32_t sensor_number;                            // the index of the current reading
    //uint32_t total ;                            // the running total
    uint32_t ADCValue ;                         // the ADC average value
    uint32_t voltage;                              // the ADC average value
    float coefficients[5];                      //linearization coefficients
    float offset;                               //Offset. "force" that must be subtracted when the force is calculated in force_calculated 
                                                //Offset is always force absolute
    float cal_ref;                              //Calibration reference. Is a relation between the actual and the expected strength
    float force;                                //Force absolute in Newtons
    float force_calculated;                      //force Calculated in Newtons. To calculate the force, is considered the offset and calibration.
    
} FSRSensor_TypeDef;

#define FILE_ID_T         0x1110
#define RECORD_KEY_T      0x2221

#define FILE_ID_SN        0x1111
#define RECORD_KEY_SN     0x2222

#define FILE_ID_FL        0x1112
#define RECORD_KEY_FL     0x2223

#define FILE_ID_O         0x1116
#define RECORD_KEY_O      0x2227

#define FILE_ID_CR        0x1120
#define RECORD_KEY_CR     0x2231


/**@brief Function .
 *
 * @details 
 *
 */

//void get_handle(ble_tms_t *);

ret_code_t fds_test_init (void);
static void my_fds_evt_handler(fds_evt_t const * );
void write_fds_fact_lin_consecutive(uint8_t cpt);
void write_fds_offset_consecutive(uint8_t cpt);
void write_fds_cal_ref_consecutive(uint8_t cpt);
void write_type_restore(void);
void write_serial_number_restore(void);
void state_machine_init(void);
void command_parser(uint16_t length, uint8_t *, ble_tms_t *);
static void decod(void);
static void state_machine_update(void);
static void Start(void);
static void Stop(void);
static void Wsn(void);
static void Rsn(void);
static void Debug(void);
static void Single(void);
static void Wfactlin(void);
static void Rfactlin(void);
static void Wtype(void);
static void Rtype(void);
static void Restore(void);
static void Tare(void);
static void Rtare(void);
static void Calw(void);
static void Rcalw(void);
static void load_flash_config(void);
static ret_code_t check_memory(void);



//static ret_code_t m_fds_write(uint8_t*);
//static ret_code_t m_fds_read(void);
//static ret_code_t m_fds_find_and_delete (void);
//static ret_code_t m_fds_init(void);


#endif // COMMAND_INTERPRETER_H__

/** @} */