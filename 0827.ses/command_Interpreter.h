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
#define RCAL                   10
#define WTYPE                  11
#define RTYPE     	         12	              
#define WFACTLIN	         13
#define RFACTLIN               14
#define WBRIDGE                15
#define RBRIDGE                16
#define WGAIN                  17
#define RGAIN                  18
#define RSTART                 19
#define VER                    20
#define RESTORE                21
#define NOTHING                22

#define NUMBER_OF_SENSORS       8
#define NUMBER_OF_SENSORS_HEAD  2
#define SAMPLES_PER_SENSOR      20 //Number of samples per sensor. In order to filtering (smooth the ADC reading)

enum
{
    CONFIG_SENSOR_CLASSIC,                     ///< XPertNode
    CONFIG_SENSOR_HEAD,                     ///< XpertRennes
};

typedef struct
{
    char * command;
    char * args[MAX_NUMBER_OF_ARGUMENTS];
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

//Flash info for TYPE
#define FILE_ID_T         0x1110
#define RECORD_KEY_T      0x2222
//Flash info for SERIAL NUMBER
#define FILE_ID_SN        0x1111
#define RECORD_KEY_SN     0x2222
//Flash info for LINEARIZATION FACTORS
#define FILE_ID_FL        0x1112
#define RECORD_KEY_FL     0x2222
//Flash info for OFFSET
#define FILE_ID_O         0x1113
#define RECORD_KEY_O      0x2222
//Flash info for CAL_REF
#define FILE_ID_CR        0x1114
#define RECORD_KEY_CR     0x2222
//Flash info for BRIDGE
#define FILE_ID_B         0x1115
#define RECORD_KEY_B      0x2222
//Flash info for GAIN
#define FILE_ID_G         0x1116
#define RECORD_KEY_G      0x2222


/**@brief Function .
 *
 * @details 
 *
 */

//void get_handle(ble_tms_t *);

ret_code_t fds_test_init (void);
static void my_fds_evt_handler(fds_evt_t const * );
void write_type(void);
void write_serial_number(void);
void write_fact_lin(uint8_t cpt);
void write_offset(uint8_t cpt);
void write_cal_ref(uint8_t cpt);

void state_machine_init(void);
void command_parser(uint16_t length, uint8_t *, ble_tms_t *);
static void decod(void);
static void state_machine_update(void);
static void Start(void);
static void Rstart(void);
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
static void Rcal(void);
static void Wbridge(void);
static void Rbridge(void);
static void Wgain(void);
static void Rgain(void);
static void load_flash_config(void);
static ret_code_t check_memory(void);



//static ret_code_t m_fds_write(uint8_t*);
//static ret_code_t m_fds_read(void);
//static ret_code_t m_fds_find_and_delete (void);
//static ret_code_t m_fds_init(void);


#endif // COMMAND_INTERPRETER_H__

/** @} */