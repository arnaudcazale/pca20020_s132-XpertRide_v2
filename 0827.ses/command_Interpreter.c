#include "command_Interpreter.h"
#include "macros_common.h"
#include "saadc.h"
#include "utils.h"
#include "nrf_delay.h"
#include "shield_manager.h"
#include "math.h"

#define  NRF_LOG_MODULE_NAME "command_Interpreter"


static m_state_machine machine;
static m_command_packet packet;
static bool binary_format = true;
static bool show_calculated_force = true;
static uint16_t sampling_period = 50;

vSoleInfo_TypeDef vSoleInfo;
FSRSensor_TypeDef FSRSensors[NUMBER_OF_SENSORS]; // contains current data for each sensor

static ble_tms_command_fact_lin_packet_t packet_fact_lin;
static ble_tms_command_tare_multi_packet_t packet_tare_multi;
static ble_tms_command_cal_ref_multi_packet_t packet_cal_ref_multi;

static ble_tms_t        * m_tms; //pointer to handle for writing characteristic
extern volatile bool flash_writing;

void state_machine_init()
{
    machine.state = NOTHING;
    machine.state_old = NOTHING;

    fds_test_init ();

    //verify if something is set into memory
    ret_code_t err_code = check_memory();

    if(err_code != NRF_SUCCESS )
    {
      Restore();
    }else{
      load_flash_config();
    }

}

/*******************************************************************************
* Function Name  : init handler
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
ret_code_t fds_test_init (void)
{
    flash_writing = false;

    ret_code_t ret = fds_register(my_fds_evt_handler);
    if (ret != FDS_SUCCESS)
    {
                    return ret;
                    
    }
    ret = fds_init();
    if (ret != FDS_SUCCESS)
    {
                    return ret;
    }
    
    return NRF_SUCCESS;
}

/*******************************************************************************
* Function Name  : event handler
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void my_fds_evt_handler(fds_evt_t const * const p_fds_evt)
{
    switch (p_fds_evt->id)
    {
        case FDS_EVT_INIT:
            if (p_fds_evt->result != FDS_SUCCESS)
            {
                // Initialization failed.
            }
            break;
        case FDS_EVT_WRITE:
            if (p_fds_evt->result == FDS_SUCCESS)
            {
                flash_writing = false;
                NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"WRITE SUCCESS");

//                NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"Record ID:\t0x%04x \r\n",  p_fds_evt->write.record_id);
//                NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"File ID:\t0x%04x \r\n",    p_fds_evt->write.file_id);
//                NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"Record key:\t0x%04x \r\n", p_fds_evt->write.record_key);
            }
            break;
        case FDS_EVT_DEL_RECORD:
            if (p_fds_evt->result == FDS_SUCCESS)
            {
               NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"delete record sucess \n\r");
            }
        break;
        case FDS_EVT_GC:     
            if (p_fds_evt->result == FDS_SUCCESS)
            { 
               NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"Garbage collector succes \n\r");
            }
            break;
        default:
            NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"m_FDS handler - %d - %d\r\n", p_fds_evt->id, p_fds_evt->result);
            break;
    }
}

/*******************************************************************************
* Function Name  : extractCommand
* Description    : eExtractCommand is called from the aAppTick main loop
*                 This function split the command in two parts, one is the command and the other is the arguments. 
*                 The result is stored in command[] array and args[] array respectivlely.
*                  
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

void command_parser(uint16_t length, uint8_t * p_data, ble_tms_t * p_tms) 
{
//    for (int i =0; i < length; i++) {
//    NRF_LOG_INFO("ble_tms_evt_handler: BLE_TMS_EVT_COMMAND_RECEIVED - %X\r\n", p_data[i]);
//    }

    //global handle for GATT 
    m_tms = p_tms;

//    p_data[length] = '\0';
//
//    uint8_t *m_data;
//    strcpy(m_data, p_data);

      uint8_t m_data[length+1];
      strcpy(m_data, p_data);
      m_data[length] = '\0';
    
    if(m_data[0] == 0x2A)
    {    
        //Hexa code for '*' character
        if(m_data[length-1] == 0x0A || m_data[length-1] == 0x0D) //nl and cr
        {   
           m_data[length-1] = '\0';
        }
        
        packet.nbrArgs = 0;
   
        // retrive the command
        packet.command = strtok((char *)&m_data[1], " " );
        
        // retrive the arguments
        packet.args[packet.nbrArgs] = strtok(NULL, " " );
        while(packet.args[packet.nbrArgs]!=NULL)
        {
          packet.nbrArgs++;
          packet.args[packet.nbrArgs] = strtok(NULL, " " );
        }
       
        // making the command to be case insensitive 
        size_t size = strlen( packet.command );
        for(size_t i = 0; i < size; i++)
        {
            if(islower((int)packet.command[i]))
            {
                packet.command[i] = toupper((int)packet.command[i]);
            }
        }
        
        //only for a debug purpose:
        NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"COMMAND:  %s \r\n", packet.command );
        NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN" 'nbrArgs': %d \r\n",packet.nbrArgs);
        for(int i=0;i<packet.nbrArgs;i++)
        NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN" 'arg[%d]': [%s] \r\n",i,packet.args[i]);

        decod();
    }
}

/*******************************************************************************
* Function Name	: Decod
* Description	: decode the incoming data
**				:  decodification of RTX vector
**				:  *start/r:			        :starts sending data
**				:  *stop/r:				:stops sending data
**				:  *ver/r:				:sends firmware version number
**   Realizes the command interpreter (for general purpose commands).
* Input			: None
* Output		: None
* Return		: None
*******************************************************************************/

static void decod(void)
{

    if(strcmp(packet.command,"VER")==0) {
      machine.state_old = machine.state;
      machine.state = VER;
    }
    else if(strcmp(packet.command,"START")==0) {
      machine.state_old = machine.state;
      machine.state = START;
    }
    else if(strcmp(packet.command,"RSTART")==0) {
      machine.state_old = machine.state;
      machine.state = RSTART;
    }
    else if(strcmp(packet.command,"STOP")==0) {
      machine.state_old = machine.state;
      machine.state = STOP;
    }
    else if(strcmp(packet.command,"DEBUG")==0) {
      machine.state_old = machine.state;
      machine.state = DEBUG;
    }
    else if(strcmp(packet.command,"SINGLE")==0) {
      machine.state_old = machine.state;
      machine.state = SINGLE;
    }    
    else if(strcmp(packet.command,"WSN")==0) {
      machine.state_old = machine.state;
      machine.state = WSN;
    }
    else if(strcmp(packet.command,"RSN")==0) {
      machine.state_old = machine.state;
      machine.state = RSN;
    }
    else if(strcmp(packet.command,"TARE")==0) {
      machine.state_old = machine.state;
      machine.state = TARE;
    }
    else if(strcmp(packet.command,"CALW")==0) {
      machine.state_old = machine.state;
      machine.state = CALW;
    }
    else if(strcmp(packet.command,"RTARE")==0) {
      machine.state_old = machine.state;
      machine.state = RTARE;
    }
    else if(strcmp(packet.command,"RCAL")==0) {
      machine.state_old = machine.state;
      machine.state = RCAL;
    }
    else if(strcmp(packet.command,"WTYPE")==0) {
      machine.state_old = machine.state;
      machine.state = WTYPE;
    }
    else if(strcmp(packet.command,"RTYPE")==0) {
      machine.state_old = machine.state;
      machine.state = RTYPE;
    }
    else if(strcmp(packet.command,"WFACTLIN")==0) {
      machine.state_old = machine.state;
      machine.state = WFACTLIN;
    }
    else if(strcmp(packet.command,"RFACTLIN")==0) {
      machine.state_old = machine.state;
      machine.state = RFACTLIN;
    }
    else if(strcmp(packet.command,"WBRIDGE")==0) {
      machine.state_old = machine.state;
      machine.state = WBRIDGE;
    }
    else if(strcmp(packet.command,"RBRIDGE")==0) {
      machine.state_old = machine.state;
      machine.state = RBRIDGE;
    }
    else if(strcmp(packet.command,"WGAIN")==0) {
      machine.state_old = machine.state;
      machine.state = WGAIN;
    }
    else if(strcmp(packet.command,"RGAIN")==0) {
      machine.state_old = machine.state;
      machine.state = RGAIN;
    }
    else if(strcmp(packet.command,"RESTORE")==0) {
      machine.state_old = machine.state;
      machine.state = RESTORE;
    }
    else{
      machine.state_old = machine.state;
      machine.state = NOTHING;
    }

      //reset command array for next receive
      size_t size = strlen( packet.command );
      for(int i = 0; i < size; i++){
        packet.command[i] = '\0';
      }
       
      state_machine_update();
}


/*******************************************************************************
* Function Name  : StateMachine
* Description    : StateMachiine is called from the aAppTick main loop
*                  It calls functions that implement commands selected
*                  It implements state machine for commands interpreter.
*                  If Set of available commands is extended with device specific commands
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void state_machine_update(void) 
{
  switch(machine.state) {
  case VER:
    //vSoleAppVersion();
    machine.state = machine.state_old; //come back previous state
    break;
  case START:
    if((machine.state_old != START) && (machine.state_old != DEBUG)){
      Start();
      machine.state_old = machine.state; 
    }
    break;
  case RSTART:
    Rstart();
    machine.state_old = machine.state; 
  break;
  case STOP:
    if((machine.state_old != STOP) && (machine.state_old != NOTHING)){
      Stop();
      machine.state_old = machine.state; 
    }
    break;
  case DEBUG:
    if(machine.state_old != DEBUG){
      Debug();
      machine.state_old = machine.state; //come back previous state
      }
    break;
  case SINGLE:
     Single();
     machine.state = machine.state_old; //come back previous state
    break;    
  case WSN:
     Wsn();
     machine.state = machine.state_old; //come back previous state
    break;
  case RSN:
     Rsn();
     machine.state = machine.state_old; //come back previous state
    break;
  case TARE:
     Tare();
     machine.state = machine.state_old; //come back previous state
    break;
  case RTARE:
     Rtare();
     machine.state = machine.state_old; //come back previous state
    break;
  case CALW:
     Calw();
     machine.state = machine.state_old; //come back previous state
    break;
  case RCAL:
     Rcal();
     machine.state = machine.state_old; //come back previous state
    break;
  case WFACTLIN:
     Wfactlin();
     machine.state = machine.state_old; //come back previous state
    break;
  case RFACTLIN:
     Rfactlin();
     machine.state = machine.state_old; //come back previous state
    break;
  case WTYPE:
     Wtype();
     machine.state = machine.state_old; //come back previous state
    break;
  case RTYPE:
     Rtype();
     machine.state = machine.state_old; //come back previous state
    break;
  case WBRIDGE:
     Wbridge();
     machine.state = machine.state_old; //come back previous state
    break;
  case RBRIDGE:
     Rbridge();
     machine.state = machine.state_old; //come back previous state
    break;
  case WGAIN:
     Wgain();
     machine.state = machine.state_old; //come back previous state
    break;
  case RGAIN:
     Rgain();
     machine.state = machine.state_old; //come back previous state
    break;
  case RESTORE:
     Restore();
     machine.state = machine.state_old; //come back previous state
    break;
  case NOTHING:
     machine.state = machine.state_old;
     break; 
  }
}

/*******************************************************************************
* Function Name : Start
* Command       : *start
* Description   : Send the output data to the host (associated to '*start' command).
*                  The system send continuosly through the SendDataToHost function.
* Input format  : *start [value type] [sample rate] [output format]
*               [value type]    -> the type of value to send (force or force calculte)
*                                       F:      Send Force in Newtons.
*                                       FC:     Send force calculated in newtons.
*               [sample rate]   -> Sample rate in sample per second s/s. Must be a value between 1 and 100
*               [output format] -> The output format can be an array of char in ASCII format or an array of floats in binary format.
*                                       ASCII:  ASCII format. ex: [valS0;valS1;valS2;valS3\r\n]
*                                       BIN:    Binary format. ex: [dr0000111122223333\r\n]
*                                               where: dr (data ready), 0000 (force_sensor[0]), 1111 (force_sensor[1]), nnnn (force_sensor[n])
*                                               All numbers have float representation (#4 bytes each);                                
* Output format : [valS0;valS1;valS2;valS3;valSn\r\n] or [dr0000111122223333nnnn\r\n]
* Example       : *start F 100 ASCII
* Notes         : If not specify any arguments "*start" the default command is: *start FC 20 BIN
*******************************************************************************/
static void Start(void)
{
    if(packet.nbrArgs==2)
    {
//        if(strcmp(packet.args[0],"F")==0) 
//          show_calculated_force = false;

        uint32_t sampling_frequency = (uint32_t)atoi(packet.args[1]);   // sampling Frequency in Hz 

        // Be sure not to go higher than 50 Hz
        if(sampling_frequency > 50)
        {
            sampling_frequency = 50;
        }
        sampling_period = 1000/sampling_frequency; // sampling Period in ms

//        if(strcmp(packet.args[2],"ASCII")==0) 
//          binary_format = false;
    }else
    {
        sampling_period = 50; // sampling Period in ms (= 20 Hz)
//        show_calculated_force = true;
//        binary_format = true;
    }
    
    start_ADC(machine.state, m_tms, sampling_period, packet.args[0], NULL);
    uint32_t err_code;
    
    //machine.state = NOTHING;
}

/*******************************************************************************
* Function Name : Rstart
* Command       : *Rstart
* Description   : Send the output data to the host (associated to '*start' command).
*                  The system send continuosly through the SendDataToHost function.
* Input format  : *start [value type] [sample rate] [output format]
*               [value type]    -> the type of value to send (force or force calculte)
*                                       F:      Send Force in Newtons.
*                                       FC:     Send force calculated in newtons.
*               [sample rate]   -> Sample rate in sample per second s/s. Must be a value between 1 and 100
*               [output format] -> The output format can be an array of char in ASCII format or an array of floats in binary format.
*                                       ASCII:  ASCII format. ex: [valS0;valS1;valS2;valS3\r\n]
*                                       BIN:    Binary format. ex: [dr0000111122223333\r\n]
*                                               where: dr (data ready), 0000 (force_sensor[0]), 1111 (force_sensor[1]), nnnn (force_sensor[n])
*                                               All numbers have float representation (#4 bytes each);                                
* Output format : [valS0;valS1;valS2;valS3;valSn\r\n] or [dr0000111122223333nnnn\r\n]
* Example       : *start F 100 ASCII
* Notes         : If not specify any arguments "*start" the default command is: *start FC 20 BIN
*******************************************************************************/
static void Rstart(void)
{
    if(packet.nbrArgs==2)
    {
        uint32_t sampling_frequency = (uint32_t)atoi(packet.args[1]);   // sampling Frequency in Hz 

        // Be sure not to go higher than 50 Hz
        if(sampling_frequency > 50)
        {
            sampling_frequency = 50;
        }
        sampling_period = 1000/sampling_frequency; // sampling Period in ms
    }else
    {
        sampling_period = 50; // sampling Period in ms (= 20 Hz)
    }
    
    start_ADC(machine.state, m_tms, sampling_period, packet.args[0], NULL);
    uint32_t err_code;
}

/*******************************************************************************
* Function Name : Debug
* Command       : DEBUG
* Description   : the command send all values to the host through "SendDataHost". The output show the data of one sensor per line.
* Input format  : *debug [continuous mode]
*               [continuous mode]       -> optional argument, if set the system send continuosly debug data through the SendDataToHost function with sample rate = 10. 
*                                       CONT: enable continuous mode
*                
* Output format :Sx;ADC;V;F;FC/r/n 
*               where:
*               Sx      -> x is the number of the sensor
*               ADC     -> ADC values for the x sensor
*               V       -> Voltage
*               F       -> Force (Newtons)
*               FC      -> Force calculate
* Example       : *debug CONT                
*******************************************************************************/
static void Debug(void)
{
  uint8_t continuous_mode = false;
  
  if(packet.nbrArgs==1)
  {
    if(strcmp(packet.args[0],"CONT")==0) 
    {
      uint32_t sampling_frequency = 10;
      sampling_period = 1000/sampling_frequency; // sampling Period in ms
      continuous_mode = true;

      start_ADC(machine.state, m_tms, sampling_period, packet.args[0], NULL);

    }
  }else if(packet.nbrArgs==0){

      start_ADC(machine.state, m_tms, sampling_period, NULL, NULL);
      machine.state = NOTHING;
  }
    
}

/*******************************************************************************
* Function Name  : Stop
* Description    : It stops the state machine
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void Stop(void)
{
    stop_ADC(machine.state_old);
}

/*******************************************************************************
* Function Name : Single
* Command       : SINGLE
* Description   : 
                  The default values is "*single fc". in binari format (see start command for more information about binari format)
* Input format  : *single [value type]
*               [value type] is optional and can be: 
*               V:      Send Voltage Value.
*               F:      Send Force in Newtons.
*               FC:     Send force calculated.
* Output format : valS0;valS1;valS2;valS3\r\n (for ASCII output format)
* Example       : *single V         
* Notes         : If not specify any arguments "*single" the default command is: *single FC (BIN)
*******************************************************************************/
static void Single(void)
{
    start_ADC(machine.state, m_tms, NULL, packet.args[0], NULL);
}

/*******************************************************************************
* Function Name  : Wsn
* Description    : Write serial number
* Input          : *WNS[serial number] with [XXXX]
* Output         : None
* Return         : None
*******************************************************************************/
static void Wsn(void)
{
    // int numCharacters=0;
    if(packet.nbrArgs==1)
    {
        uint32_t err_code;
        strcpy(vSoleInfo.serial_number,packet.args[0]);
        
        //First delete previous files for avoid accumulation
        err_code = m_fds_find_and_delete(FILE_ID_SN, RECORD_KEY_SN);
        APP_ERROR_CHECK(err_code);
        //Write to flash
       
        err_code = m_fds_write_serial_number(FILE_ID_SN, RECORD_KEY_SN, packet.args[0]);
        APP_ERROR_CHECK(err_code);
    }
}

/*******************************************************************************
* Function Name  : Rsn
* Description    : Read serial number
* Input          : *RSN[serial number] with [XXXX]
* Output         : None
* Return         : None
*******************************************************************************/
static void Rsn(void)
{
    uint32_t err_code;
    //read from FLASH
    uint8_t * ptr;
    ptr = m_fds_read_serial_number(FILE_ID_SN, RECORD_KEY_SN);
    
    for (uint8_t i=0;i<4;i++)			
    {				
        vSoleInfo.serial_number[i] = ptr[i];
        //NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"%c\r\n",vSoleInfo.serial_number[i]);
    }	
    (void)ble_tms_command_serial_set(m_tms, &vSoleInfo.serial_number);
}


/*******************************************************************************
* Function Name  : Wfactlin
* Description    : 
* Input          : 
* Output         : None
* Return         : None
*******************************************************************************/
static void Wfactlin(void){

   uint8_t ind_sensor;
   uint8_t ind_coeff;
   uint16_t record_key;
   uint32_t err_code;
    
    if(packet.nbrArgs==3)
    {
        ind_sensor = atoi(packet.args[0]);
        ind_coeff = atoi(packet.args[1]);

        if(ind_sensor<NUMBER_OF_SENSORS)
        {
            FSRSensors[ind_sensor].coefficients[ind_coeff]=atof(packet.args[2]);    
            record_key = RECORD_KEY_FL + ind_sensor;
            err_code = m_fds_find_and_delete(FILE_ID_FL, record_key);
            APP_ERROR_CHECK(err_code);
            //Write to flash
            err_code = m_fds_write_fact_lin(FILE_ID_FL, record_key, FSRSensors[ind_sensor].coefficients);
            APP_ERROR_CHECK(err_code);
        }
        
    }else if(packet.nbrArgs==6)
    {
        ind_sensor = atoi(packet.args[0]);

        if(ind_sensor<NUMBER_OF_SENSORS)
        {
            for(ind_coeff = 0; ind_coeff<5; ind_coeff++)
            {
              FSRSensors[ind_sensor].coefficients[ind_coeff]=atof(packet.args[ind_coeff+1]);
            }
            
            record_key = RECORD_KEY_FL + ind_sensor;
            //First delete previous files for avoid accumulation
            err_code = m_fds_find_and_delete(FILE_ID_FL, record_key);
            APP_ERROR_CHECK(err_code);
            //Write to flash
            err_code = m_fds_write_fact_lin(FILE_ID_FL, record_key, FSRSensors[ind_sensor].coefficients);
            APP_ERROR_CHECK(err_code);
        }
    }
}

/*******************************************************************************
* Function Name  : Rfactlin
* Description    : 
* Input          : 
* Output         : None
* Return         : None
*******************************************************************************/
static void Rfactlin(void)
{
    uint32_t err_code;
    uint32_t ind_sensor; 
    int ind_coeff;
    //uint16_t file_id; 
    uint16_t record_key;
    float * data;

    if(packet.nbrArgs==1)
    {
        ind_sensor = atoi(packet.args[0]);
        if(ind_sensor<NUMBER_OF_SENSORS)
        {
            record_key = RECORD_KEY_FL + ind_sensor;
            //read from FLASH
            data = m_fds_read_lin_fact(FILE_ID_FL, record_key);

            for(int ind_coeff = 0; ind_coeff<5; ind_coeff++)
            {
                FSRSensors[ind_sensor].coefficients[ind_coeff] = data[ind_coeff];
                packet_fact_lin.coeff[ind_coeff] = FSRSensors[ind_sensor].coefficients[ind_coeff];
            }

            packet_fact_lin.sensor_number = ind_sensor;
            err_code = ble_tms_command_fact_lin_set(m_tms, &packet_fact_lin);
            APP_ERROR_CHECK(err_code);
        }        
    }
}

/*******************************************************************************
* Function Name  : Wtype
* Description    : 
* Input          : 
* Output         : None
* Return         : None
*******************************************************************************/
static void Wtype(void)
{
  uint32_t err_code;
  if(packet.nbrArgs==1){
      vSoleInfo.type = atoi(packet.args[0]);
      //First delete previous files for avoid accumulation
      err_code = m_fds_find_and_delete(FILE_ID_T, RECORD_KEY_T);
      APP_ERROR_CHECK(err_code);
      //Write to flash
      err_code = m_fds_write_type(FILE_ID_T, RECORD_KEY_T, packet.args[0]);
      APP_ERROR_CHECK(err_code);
    }
}

/*******************************************************************************
* Function Name  : Rtype
* Description    : 
* Input          : 
* Output         : None
* Return         : None
*******************************************************************************/
static void Rtype(void)
{
    uint32_t err_code;
    uint8_t * ptr;

    ptr = m_fds_read_type(FILE_ID_T, RECORD_KEY_T);
    vSoleInfo.type = ptr[0];
 
    (void)ble_tms_command_type_set(m_tms, &vSoleInfo.type);
}

/*******************************************************************************
* Function Name  : Tare
* Description    : 
* Input          : 
* Output         : None
* Return         : None
*******************************************************************************/
static void Tare(void)
{
  uint8_t ind_sensor;
 
  if(packet.nbrArgs==1)
  {
    ind_sensor = atoi(packet.args[0]);
    if(ind_sensor<NUMBER_OF_SENSORS)
    {
        start_ADC(machine.state, m_tms, NULL, ind_sensor, NULL);
    }
  }
  else if (packet.nbrArgs==0)
  {
        start_ADC(machine.state, m_tms, NULL, -1, NULL);
  }
  
}

/*******************************************************************************
* Function Name  : Rtare
* Description    : 
* Input          : 
* Output         : None
* Return         : None
*******************************************************************************/
static void Rtare(void)
{
  uint8_t ind_sensor;
  uint16_t record_key;
  float * data;
  
  if(packet.nbrArgs==1)
  {
    ind_sensor = atoi(packet.args[0]);

    if(ind_sensor<NUMBER_OF_SENSORS)
    {
      record_key = RECORD_KEY_O + ind_sensor;
      data = m_fds_read_tare(FILE_ID_O, record_key);
      FSRSensors[ind_sensor].offset = *data;
      
      (void)ble_tms_command_tare_single_set(m_tms, &FSRSensors[ind_sensor].offset);
    }
  }
  else if (packet.nbrArgs==0)
  {
    for(ind_sensor = 0; ind_sensor<NUMBER_OF_SENSORS; ind_sensor++)
    {
      record_key = RECORD_KEY_O + ind_sensor;
      data = m_fds_read_tare(FILE_ID_O, record_key);
      FSRSensors[ind_sensor].offset = *data;
      packet_tare_multi.offset[ind_sensor] = FSRSensors[ind_sensor].offset;
    }

    (void)ble_tms_command_tare_multi_set(m_tms, &packet_tare_multi);
  }
}

/*******************************************************************************
* Function Name  : Calw
* Description    : 
* Input          : 
* Output         : None
* Return         : None
*******************************************************************************/
static void Calw(void)
{
  float current_force;
  float expected_force;
  uint32_t ind_sensor;
  
  if(packet.nbrArgs==2){
    ind_sensor = atoi(packet.args[0]);
    expected_force = atof(packet.args[1]);
    start_ADC(machine.state, m_tms, NULL, ind_sensor, expected_force);
  }
}

/*******************************************************************************
* Function Name  : RCalw
* Description    : 
* Input          : 
* Output         : None
* Return         : None
*******************************************************************************/
static void Rcal(void)
{
  uint8_t ind_sensor; 
  uint16_t record_key;
  float * data;
  
  if(packet.nbrArgs==1)
  {
    ind_sensor = atoi(packet.args[0]);
    if(ind_sensor<NUMBER_OF_SENSORS)
    {
      record_key = RECORD_KEY_CR + ind_sensor;
      data = m_fds_read_cal_ref(FILE_ID_CR, record_key);
      FSRSensors[ind_sensor].cal_ref = *data;
      
      (void)ble_tms_command_cal_ref_single_set(m_tms, &FSRSensors[ind_sensor].cal_ref);
    }
  }
  else if (packet.nbrArgs==0)
  {
    for(ind_sensor = 0; ind_sensor<NUMBER_OF_SENSORS; ind_sensor++)
    {
      record_key = RECORD_KEY_CR + ind_sensor;
      data = m_fds_read_cal_ref(FILE_ID_CR, record_key);
      FSRSensors[ind_sensor].cal_ref = *data;
      
      packet_cal_ref_multi.cal_ref[ind_sensor] = FSRSensors[ind_sensor].cal_ref;
    }
    
    (void)ble_tms_command_cal_ref_multi_set(m_tms, &packet_cal_ref_multi);
  }
}

/*******************************************************************************
* Function Name  : Wbridge
* Description    : Modify resistor value of balanced wheatstone bridge
* Input          : 
* Output         : None
* Return         : None
*******************************************************************************/
static void Wbridge(void)
{
  
  uint32_t err_code;
  uint16_t resistor;

  if(packet.nbrArgs==1)
  {
    resistor = atoi(packet.args[0]);

    if(resistor < 800)
    {
      err_code = bridge_balancing(resistor);
      APP_ERROR_CHECK(err_code);

      if (err_code != NRF_SUCCESS)
      {
        return err_code;
      }
      
      //First delete previous files for avoid accumulation
      err_code = m_fds_find_and_delete(FILE_ID_B, RECORD_KEY_B);
      APP_ERROR_CHECK(err_code);
      //Write to flash
      err_code = m_fds_write_bridge(FILE_ID_B, RECORD_KEY_B, &resistor);
      APP_ERROR_CHECK(err_code);
   
    } 
  }
}

/*******************************************************************************
* Function Name  : Rbridge
* Description    : 
* Input          : 
* Output         : None
* Return         : None
*******************************************************************************/
static void Rbridge(void)
{

  uint8_t data_mux;
  uint8_t data_pot;
  uint32_t err_code;

  //read from FLASH
  uint16_t * ptr;
  ptr = m_fds_read_bridge(FILE_ID_B, RECORD_KEY_B);
  
  (void)ble_tms_command_bridge_set(m_tms, &ptr[0]);
 
}

/*******************************************************************************
* Function Name  : Wgain
* Description    : 
* Input          : 
* Output         : None
* Return         : None
*******************************************************************************/
static void Wgain(void)
{
  uint32_t err_code;
  uint16_t gain;

  if(packet.nbrArgs==1)
  {
    gain = atoi(packet.args[0]);
    err_code = amplifier_gain_selection(gain);
    APP_ERROR_CHECK(err_code);

    if (err_code != NRF_SUCCESS)
    {
      return err_code;
    }

    //First delete previous files for avoid accumulation
    err_code = m_fds_find_and_delete(FILE_ID_G, RECORD_KEY_G);
    APP_ERROR_CHECK(err_code);
    //Write to flash
    err_code = m_fds_write_gain(FILE_ID_G, RECORD_KEY_G, &gain);
    APP_ERROR_CHECK(err_code);
  }
}

/*******************************************************************************
* Function Name  : Rgain
* Description    : 
* Input          : 
* Output         : None
* Return         : None
*******************************************************************************/
static void Rgain(void)
{
  uint8_t data;
  uint32_t err_code;

  uint8_t * ptr;
  ptr = m_fds_read_gain(FILE_ID_G, RECORD_KEY_G);
  
  (void)ble_tms_command_gain_set(m_tms, &ptr[0]);
  
}

/*******************************************************************************
* Function Name  : Restore
* Description    : 
* Input          : 
* Output         : None
* Return         : None
*******************************************************************************/
static void Restore(void)
{
    uint32_t err_code;
    uint8_t ind_sensor; 

    //SERIAL NUMBER Write into FLASH
    strcpy(vSoleInfo.serial_number,"AAAA");
    flash_writing = true;
    write_serial_number();
    while(!flash_writing);

    //TYPE Write into FLASH
    vSoleInfo.type = 2;
    flash_writing = true;
    write_type();
    while(!flash_writing);

    //Bridge Write into FLASH and set chip to resistor value
    flash_writing = true;
    write_bridge(200);
    while(!flash_writing);

    //Gain Write into FLASH and set chip to resistor value
    flash_writing = true;
    write_gain(2);
    while(!flash_writing);

    for(ind_sensor=0 ; ind_sensor<NUMBER_OF_SENSORS ; ind_sensor++)
    {
      FSRSensors[ind_sensor].coefficients[0]=0;
      FSRSensors[ind_sensor].coefficients[1]=1;
      FSRSensors[ind_sensor].coefficients[2]=0;
      FSRSensors[ind_sensor].coefficients[3]=0;
      FSRSensors[ind_sensor].coefficients[4]=0;
      FSRSensors[ind_sensor].offset=0;
      FSRSensors[ind_sensor].cal_ref=1;

      //LINEARIZATION FACTOR Write into FLASH
      flash_writing = true;
      write_fact_lin(ind_sensor);
      while(!flash_writing);

      //OFFSET Write into FLASH
      flash_writing = true;
      write_offset(ind_sensor);
      while(!flash_writing);

      //CAL_REF Write into FLASH
      flash_writing = true;
      write_cal_ref(ind_sensor);
      while(!flash_writing);
    }

  // call the garbage collector to empty them, don't need to do this all the time, this is just for demonstration
  err_code = fds_gc();
  APP_ERROR_CHECK(err_code);
}

void write_type(void)
{
   uint32_t err_code;
   //First delete previous files for avoid accumulation
   err_code = m_fds_find_and_delete(FILE_ID_T, RECORD_KEY_T);
   APP_ERROR_CHECK(err_code);
   //Write to flash
   err_code = m_fds_write_type(FILE_ID_T, RECORD_KEY_T, "2");
   APP_ERROR_CHECK(err_code);

}

void write_serial_number(void)
{
   uint32_t err_code;
   //First delete previous files for avoid accumulation
   err_code = m_fds_find_and_delete(FILE_ID_SN, RECORD_KEY_SN);
   APP_ERROR_CHECK(err_code);
   //Write to flash
   err_code = m_fds_write_serial_number(FILE_ID_SN, RECORD_KEY_SN, vSoleInfo.serial_number);
   APP_ERROR_CHECK(err_code);
}

void write_fact_lin(uint8_t cpt)
{
  uint32_t err_code; 
  uint16_t record_key;

  //Write coeffs
  record_key = RECORD_KEY_FL + cpt;

  //First delete previous files for avoid accumulation
  err_code = m_fds_find_and_delete(FILE_ID_FL, record_key);
  APP_ERROR_CHECK(err_code);

  //Write to flash
  err_code = m_fds_write_fact_lin(FILE_ID_FL, record_key, FSRSensors[cpt].coefficients);
  APP_ERROR_CHECK(err_code);
}

void write_offset(uint8_t cpt)
{
   uint32_t err_code;
   uint16_t record_key;

   record_key = RECORD_KEY_O + cpt;
   //First delete previous files for avoid accumulation
   err_code = m_fds_find_and_delete(FILE_ID_O, record_key);
   APP_ERROR_CHECK(err_code);
   //Write to flash
   err_code = m_fds_write_offset(FILE_ID_O, record_key, &FSRSensors[cpt].offset);
   APP_ERROR_CHECK(err_code);
}

void write_cal_ref(uint8_t cpt)
{
   uint32_t err_code;
   uint16_t record_key;

   record_key = RECORD_KEY_CR + cpt;
   //First delete previous files for avoid accumulation
   err_code = m_fds_find_and_delete(FILE_ID_CR, record_key);
   APP_ERROR_CHECK(err_code);
   //Write to flash
   err_code = m_fds_write_cal_ref(FILE_ID_CR, record_key, &FSRSensors[cpt].cal_ref);
   APP_ERROR_CHECK(err_code);
}

void write_bridge(uint16_t resistor)
{
   uint32_t err_code;
  
   if(resistor < 800)
   {

      //Set chip to resistor value
      //NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"Wbridge, arg = %d", resistor);
      err_code = bridge_balancing(resistor);
      APP_ERROR_CHECK(err_code);
      
      //First delete previous files for avoid accumulation
      err_code = m_fds_find_and_delete(FILE_ID_B, RECORD_KEY_B);
      APP_ERROR_CHECK(err_code);
      //Write to flash
      err_code = m_fds_write_bridge(FILE_ID_B, RECORD_KEY_B, &resistor);
      APP_ERROR_CHECK(err_code);
   
   } 
}

void write_gain(uint8_t gain)
{
  uint32_t err_code;
  err_code = amplifier_gain_selection(gain);
  APP_ERROR_CHECK(err_code);

  //First delete previous files for avoid accumulation
  err_code = m_fds_find_and_delete(FILE_ID_G, RECORD_KEY_G);
  APP_ERROR_CHECK(err_code);
  //Write to flash
  err_code = m_fds_write_gain(FILE_ID_G, RECORD_KEY_G, &gain);
  APP_ERROR_CHECK(err_code);
}

void load_flash_config()
{
    uint32_t err_code;
    uint8_t * ptr;
    float * data;
    uint16_t *resistor;
    uint8_t *gain;
    uint16_t record_key;
    uint32_t ind_sensor;

    //load serial_number
    ptr = m_fds_read_serial_number(FILE_ID_SN, RECORD_KEY_SN);
    for (uint8_t i=0;i<4;i++)			
    {				
        vSoleInfo.serial_number[i] = ptr[i];
    }	

    //load type
    ptr = m_fds_read_type(FILE_ID_T, RECORD_KEY_T);
    vSoleInfo.type = ptr[0];

    //load and set bridge resistor
    resistor = m_fds_read_bridge(FILE_ID_B, RECORD_KEY_B);
    if(resistor[0] < 800)
    {
      err_code = bridge_balancing(resistor[0]);
      APP_ERROR_CHECK(err_code);
    }

    //load and set gain
    gain = m_fds_read_gain(FILE_ID_G, RECORD_KEY_G);
    err_code = amplifier_gain_selection(gain[0]);
    APP_ERROR_CHECK(err_code);

    //load coefficients
    for(ind_sensor = 0; ind_sensor < NUMBER_OF_SENSORS; ind_sensor ++)
    {
        record_key = RECORD_KEY_FL + ind_sensor;
        data = m_fds_read_lin_fact(FILE_ID_FL, record_key);
        for(int ind_coeff = 0; ind_coeff<5; ind_coeff++)
        {
            FSRSensors[ind_sensor].coefficients[ind_coeff] = data[ind_coeff];
        }
    }

    //load offsets
    for(ind_sensor = 0; ind_sensor < NUMBER_OF_SENSORS; ind_sensor++)
    {
      record_key = RECORD_KEY_O + ind_sensor;
      data = m_fds_read_tare(FILE_ID_O, record_key);
      FSRSensors[ind_sensor].offset = *data;
    }

    //load cal_ref
    for(ind_sensor = 0; ind_sensor<NUMBER_OF_SENSORS; ind_sensor++)
    {
      record_key = RECORD_KEY_CR + ind_sensor;
      data = m_fds_read_cal_ref(FILE_ID_CR, record_key);
      FSRSensors[ind_sensor].cal_ref = *data;
    }
        
    //debug all
    printf("\r\n");
    printf("***********************FLASH_CONFIG***************************\r\n");
    printf("serial number:  %s \r\n", vSoleInfo.serial_number);
    printf("type: %c \n", vSoleInfo.type);
    printf("bridge resistor: %d kOhms \n", *resistor);
    printf("amplifier gain: %d \n", *gain);

    for(ind_sensor = 0; ind_sensor < NUMBER_OF_SENSORS; ind_sensor++)
    {

       printf("sensor [%d] coeff: %f - %f - %f - %f - %f \n", ind_sensor,
           FSRSensors[ind_sensor].coefficients[0],
           FSRSensors[ind_sensor].coefficients[1],
           FSRSensors[ind_sensor].coefficients[2],
           FSRSensors[ind_sensor].coefficients[3],
           FSRSensors[ind_sensor].coefficients[4]);

       printf("           offset %f \n", FSRSensors[ind_sensor].offset);
       printf("           cal_ref %f \n", FSRSensors[ind_sensor].cal_ref);
    }
    printf("**************************************************************\r\n");
    printf("\r\n");
}

ret_code_t check_memory()
{
  //Check if the first record exists
  fds_flash_record_t  flash_record;
  fds_record_desc_t   record_desc;
  fds_find_token_t    ftok;
  /* It is required to zero the token before first use. */
  memset(&ftok, 0x00, sizeof(fds_find_token_t));
  
  ret_code_t err_code = fds_record_find(FILE_ID_SN, RECORD_KEY_SN, &record_desc, &ftok);
  return err_code;
}
