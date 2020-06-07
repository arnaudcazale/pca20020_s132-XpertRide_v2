#include "utils.h"
#include "fstorage.h"


static fds_record_desc_t    my_record_desc;

extern FSRSensor_TypeDef FSRSensors[NUMBER_OF_SENSORS];


/*******************************************************************************
* Function Name  : delete specific file from FLASH
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
ret_code_t m_fds_find_and_delete (uint16_t FILE_ID, uint16_t RECORD_KEY)
{
    ret_code_t err_code;
    fds_find_token_t    ftok;

    ftok.page=0;
    ftok.p_addr=NULL;
    // Loop and find records with same ID and rec key and mark them as deleted. 
    while (fds_record_find(FILE_ID, RECORD_KEY, &my_record_desc, &ftok) == FDS_SUCCESS)
    {
        err_code = fds_record_delete(&my_record_desc);
        APP_ERROR_CHECK(err_code);
        //NRF_LOG_INFO("Deleted record ID: %d \r\n",my_record_desc.record_id);
    }
    return NRF_SUCCESS;
}

/*******************************************************************************
* Function Name  : write serial number to FLASH
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
ret_code_t m_fds_write_serial_number(uint16_t FILE_ID, uint16_t RECORD_KEY, uint8_t write_data[])
{
    //NRF_LOG_INFO("write_data: %s", write_data);
    static uint8_t m_deadbeef[4] = {0}; //0xBADF00D0;
    memcpy(m_deadbeef, write_data, 4*sizeof(write_data));
    //NRF_LOG_INFO("data deadbeef %s: \r\n", m_deadbeef);
   
    uint32_t            err_code;
    fds_record_t        record;
    fds_record_chunk_t  record_chunk;

     // Set up data.
    record_chunk.p_data         = &m_deadbeef;
    record_chunk.length_words   = sizeof(m_deadbeef)/sizeof(uint8_t);
    // Set up record.
    record.file_id              = FILE_ID;
    record.key                  = RECORD_KEY;
    record.data.p_chunks        = &record_chunk;
    record.data.num_chunks      = 1;
    
    err_code = fds_record_write(&my_record_desc, &record);
    if (err_code != FDS_SUCCESS)
    {
        return err_code;
    }
}

/*******************************************************************************
* Function Name  : write serial number to FLASH
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
ret_code_t m_fds_write_type(uint16_t FILE_ID, uint16_t RECORD_KEY, uint8_t write_data[])
{
    static uint8_t   m_deadbeef[1] = {0}; //0xBADF00D0;
    memcpy(m_deadbeef, write_data, sizeof(write_data));
    //NRF_LOG_INFO("data deadbeef: %s \r\n", m_deadbeef);
   
    uint32_t            err_code;
    fds_record_t        record;
    fds_record_chunk_t  record_chunk;

     // Set up data.
    record_chunk.p_data         = &m_deadbeef;
    record_chunk.length_words   = sizeof(m_deadbeef)/sizeof(uint8_t);
    // Set up record.
    record.file_id              = FILE_ID;
    record.key                  = RECORD_KEY;
    record.data.p_chunks        = &record_chunk;
    record.data.num_chunks      = 1;
    
    err_code = fds_record_write(&my_record_desc, &record);
    if (err_code != FDS_SUCCESS)
    {
        return err_code;
    }
}

/*******************************************************************************
* Function Name  : write offset
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
ret_code_t m_fds_write_offset(uint16_t FILE_ID, uint16_t RECORD_KEY, float write_data[])
{
    static float   m_deadbeef[1] = {0}; //0xBADF00D0;
    memcpy(m_deadbeef, write_data, sizeof(write_data));
    //printf("data deadbeef offset %f \n:",m_deadbeef[0]);
   
    uint32_t            err_code;
    fds_record_t        record;
    fds_record_chunk_t  record_chunk;

     // Set up data.
    record_chunk.p_data         = &m_deadbeef;
    record_chunk.length_words   = sizeof(m_deadbeef)/sizeof(float);
    // Set up record.
    record.file_id              = FILE_ID;
    record.key                  = RECORD_KEY;
    record.data.p_chunks        = &record_chunk;
    record.data.num_chunks      = 1;
    
    err_code = fds_record_write(&my_record_desc, &record);
    if (err_code != FDS_SUCCESS)
    {
        return err_code;
    }
}

/*******************************************************************************
* Function Name  : write cal_ref
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
ret_code_t m_fds_write_cal_ref(uint16_t FILE_ID, uint16_t RECORD_KEY, float write_data[])
{
    static float   m_deadbeef[1] = {0}; //0xBADF00D0;
    memcpy(m_deadbeef, write_data, sizeof(write_data));
    //printf("data deadbeef cal_ref %f \n:",m_deadbeef[0]);
   
    uint32_t            err_code;
    fds_record_t        record;
    fds_record_chunk_t  record_chunk;

     // Set up data.
    record_chunk.p_data         = &m_deadbeef;
    record_chunk.length_words   = sizeof(m_deadbeef)/sizeof(float);
    // Set up record.
    record.file_id              = FILE_ID;
    record.key                  = RECORD_KEY;
    record.data.p_chunks        = &record_chunk;
    record.data.num_chunks      = 1;
    
    err_code = fds_record_write(&my_record_desc, &record);
    if (err_code != FDS_SUCCESS)
    {
        return err_code;
    }
}

/*******************************************************************************
* Function Name  : write serial number to FLASH
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
ret_code_t m_fds_write_fact_lin(uint16_t FILE_ID, uint16_t RECORD_KEY, float data [])
{
    static float m_deadbeef[5] = {0}; 
    memcpy(m_deadbeef, data, 5*sizeof(data));

//    for(int i = 0; i<5; i++){
//      printf("data deadbeef %f \n:", m_deadbeef[i]);
//    }
    
    uint32_t            err_code;
    fds_record_t        record;
    fds_record_chunk_t  record_chunk;

     // Set up data.
    record_chunk.p_data         = &m_deadbeef;
    record_chunk.length_words   = sizeof(m_deadbeef)/sizeof(float);
    // Set up record.
    record.file_id              = FILE_ID;
    record.key                  = RECORD_KEY;
    record.data.p_chunks        = &record_chunk;
    record.data.num_chunks      = 1;

    err_code = fds_record_write(&my_record_desc, &record);
    if (err_code != FDS_SUCCESS)
    {
        return err_code;
    }
}

/*******************************************************************************
* Function Name  : write bridge resistor value to FLASH
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
ret_code_t m_fds_write_bridge(uint16_t FILE_ID, uint16_t RECORD_KEY, uint16_t write_data[])
{
    static uint16_t m_deadbeef[1] = {0}; //0xBADF00D0;
    memcpy(m_deadbeef, write_data, sizeof(write_data));
    //NRF_LOG_INFO("data deadbeef %d: \r\n", m_deadbeef[0]);
   
    uint32_t            err_code;
    fds_record_t        record;
    fds_record_chunk_t  record_chunk;

     // Set up data.
    record_chunk.p_data         = &m_deadbeef;
    record_chunk.length_words   = sizeof(m_deadbeef)/sizeof(uint16_t);
    // Set up record.
    record.file_id              = FILE_ID;
    record.key                  = RECORD_KEY;
    record.data.p_chunks        = &record_chunk;
    record.data.num_chunks      = 1;
    
    err_code = fds_record_write(&my_record_desc, &record);
    if (err_code != FDS_SUCCESS)
    {
        return err_code;
    }
}

/*******************************************************************************
* Function Name  : write gain value to FLASH
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
ret_code_t m_fds_write_gain(uint16_t FILE_ID, uint16_t RECORD_KEY, uint16_t write_data[])
{
    static uint16_t m_deadbeef[1] = {0}; //0xBADF00D0;
    memcpy(m_deadbeef, write_data, sizeof(write_data));
    //NRF_LOG_INFO("data deadbeef %d: \r\n", m_deadbeef[0]);
   
    uint32_t            err_code;
    fds_record_t        record;
    fds_record_chunk_t  record_chunk;

     // Set up data.
    record_chunk.p_data         = &m_deadbeef;
    record_chunk.length_words   = sizeof(m_deadbeef)/sizeof(uint16_t);
    // Set up record.
    record.file_id              = FILE_ID;
    record.key                  = RECORD_KEY;
    record.data.p_chunks        = &record_chunk;
    record.data.num_chunks      = 1;
    
    err_code = fds_record_write(&my_record_desc, &record);
    if (err_code != FDS_SUCCESS)
    {
        return err_code;
    }
}

/*******************************************************************************
* Function Name  : read serial number FLASH
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint8_t * m_fds_read_serial_number(uint16_t FILE_ID, uint16_t RECORD_KEY)
{	
  //NRF_LOG_INFO("FILE_ID = %x \r\n",FILE_ID);	
  //NRF_LOG_INFO("RECORD_KEY = %x \r\n",RECORD_KEY);	
  uint8_t * data;
  uint32_t  err_code;

  fds_flash_record_t  flash_record;
  fds_record_desc_t   record_desc;
  fds_find_token_t    ftok;
  /* It is required to zero the token before first use. */
  memset(&ftok, 0x00, sizeof(fds_find_token_t));
  
  err_code = fds_record_find(FILE_ID, RECORD_KEY, &record_desc, &ftok);
  APP_ERROR_CHECK(err_code);

  err_code = fds_record_open(&record_desc, &flash_record);
  APP_ERROR_CHECK(err_code);

  //NRF_LOG_INFO("Found Record ID = %d \r\n",record_desc.record_id);			
  //NRF_LOG_INFO("Data = \r\n");	

  data = (uint8_t *) flash_record.p_data;	
         
  /* Access the record through the flash_record structure. */
  /* Close the record when done. */
  err_code = fds_record_close(&record_desc);
  APP_ERROR_CHECK(err_code);
      
      return data;
}

/*******************************************************************************
* Function Name  : read type FLASH
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint8_t * m_fds_read_type(uint16_t FILE_ID, uint16_t RECORD_KEY)
{	
  uint8_t * data;
  uint32_t  err_code;

  fds_flash_record_t  flash_record;
  fds_record_desc_t   record_desc;
  fds_find_token_t    ftok;
  /* It is required to zero the token before first use. */
  memset(&ftok, 0x00, sizeof(fds_find_token_t));
  
  err_code = fds_record_find(FILE_ID, RECORD_KEY, &record_desc, &ftok);
  APP_ERROR_CHECK(err_code);

  err_code = fds_record_open(&record_desc, &flash_record);
  APP_ERROR_CHECK(err_code);

  //NRF_LOG_INFO("Found Record ID = %d \r\n",record_desc.record_id);			
  data = (uint8_t *) flash_record.p_data;	
          
  /* Access the record through the flash_record structure. */
  /* Close the record when done. */
  err_code = fds_record_close(&record_desc);
  APP_ERROR_CHECK(err_code);
      
      return data;
}

/*******************************************************************************
* Function Name  : read linearization factors FLASH
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
float * m_fds_read_lin_fact(uint16_t FILE_ID, uint16_t RECORD_KEY)
{	
  float * data;
  uint32_t  err_code;

  fds_flash_record_t  flash_record;
  fds_find_token_t    ftok;
  /* It is required to zero the token before first use. */
  memset(&ftok, 0x00, sizeof(fds_find_token_t));
  
  err_code = fds_record_find(FILE_ID, RECORD_KEY, &my_record_desc, &ftok);
  APP_ERROR_CHECK(err_code);

  err_code = fds_record_open(&my_record_desc, &flash_record);
  APP_ERROR_CHECK(err_code);

  //NRF_LOG_INFO("Found Record ID = %d \r\n",my_record_desc.record_id);			
  data = (float *) flash_record.p_data;	
   
  /* Access the record through the flash_record structure. */
  /* Close the record when done. */
  //err_code = fds_record_close(&record_desc);
  err_code = fds_record_close(&my_record_desc);
  APP_ERROR_CHECK(err_code);
      
      return data;
}

/*******************************************************************************
* Function Name  : read tare FLASH
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
float * m_fds_read_tare(uint16_t FILE_ID, uint16_t RECORD_KEY)
{	
  float * data;
  uint32_t  err_code;

  fds_flash_record_t  flash_record;
  fds_find_token_t    ftok;
  /* It is required to zero the token before first use. */
  memset(&ftok, 0x00, sizeof(fds_find_token_t));
  
  err_code = fds_record_find(FILE_ID, RECORD_KEY, &my_record_desc, &ftok);
  APP_ERROR_CHECK(err_code);

  err_code = fds_record_open(&my_record_desc, &flash_record);
  APP_ERROR_CHECK(err_code);

  //NRF_LOG_INFO("Found Record ID = %d \r\n",my_record_desc.record_id);			
 
  data = (float *) flash_record.p_data;	
  
  /* Access the record through the flash_record structure. */
  /* Close the record when done. */
  //err_code = fds_record_close(&record_desc);
  err_code = fds_record_close(&my_record_desc);
  APP_ERROR_CHECK(err_code);
      
      return data;
}

/*******************************************************************************
* Function Name  : read tare FLASH
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
float * m_fds_read_cal_ref(uint16_t FILE_ID, uint16_t RECORD_KEY)
{	
  float * data;
  uint32_t  err_code;

  fds_flash_record_t  flash_record;
  fds_find_token_t    ftok;
  /* It is required to zero the token before first use. */
  memset(&ftok, 0x00, sizeof(fds_find_token_t));
  
  err_code = fds_record_find(FILE_ID, RECORD_KEY, &my_record_desc, &ftok);
  APP_ERROR_CHECK(err_code);

  err_code = fds_record_open(&my_record_desc, &flash_record);
  APP_ERROR_CHECK(err_code);

  //NRF_LOG_INFO("Found Record ID = %d \r\n",my_record_desc.record_id);			
  data = (float *) flash_record.p_data;	
          
  /* Access the record through the flash_record structure. */
  /* Close the record when done. */
  //err_code = fds_record_close(&record_desc);
  err_code = fds_record_close(&my_record_desc);
  APP_ERROR_CHECK(err_code);
      
      return data;
}

/*******************************************************************************
* Function Name  : read type FLASH
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t * m_fds_read_bridge(uint16_t FILE_ID, uint16_t RECORD_KEY)
{	
  uint16_t * data;
  uint32_t  err_code;

  fds_flash_record_t  flash_record;
  fds_record_desc_t   record_desc;
  fds_find_token_t    ftok;
  /* It is required to zero the token before first use. */
  memset(&ftok, 0x00, sizeof(fds_find_token_t));
  
  err_code = fds_record_find(FILE_ID, RECORD_KEY, &record_desc, &ftok);
  APP_ERROR_CHECK(err_code);

  err_code = fds_record_open(&record_desc, &flash_record);
  APP_ERROR_CHECK(err_code);

  //NRF_LOG_INFO("Found Record ID = %d \r\n",record_desc.record_id);			
  data = (uint16_t *) flash_record.p_data;	
          
  /* Access the record through the flash_record structure. */
  /* Close the record when done. */
  err_code = fds_record_close(&record_desc);
  APP_ERROR_CHECK(err_code);
      
      return data;
}

/*******************************************************************************
* Function Name  : read type FLASH
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint8_t * m_fds_read_gain(uint16_t FILE_ID, uint16_t RECORD_KEY)
{	
  uint16_t * data;
  uint32_t  err_code;

  fds_flash_record_t  flash_record;
  fds_record_desc_t   record_desc;
  fds_find_token_t    ftok;
  /* It is required to zero the token before first use. */
  memset(&ftok, 0x00, sizeof(fds_find_token_t));
  
  err_code = fds_record_find(FILE_ID, RECORD_KEY, &record_desc, &ftok);
  APP_ERROR_CHECK(err_code);

  err_code = fds_record_open(&record_desc, &flash_record);
  APP_ERROR_CHECK(err_code);

  //NRF_LOG_INFO("Found Record ID = %d \r\n",record_desc.record_id);			
  data = (uint8_t *) flash_record.p_data;	
          
  /* Access the record through the flash_record structure. */
  /* Close the record when done. */
  err_code = fds_record_close(&record_desc);
  APP_ERROR_CHECK(err_code);
      
      return data;
}


/*
* Process the FSRSensor_TypeDef with adc values to complete the other fields of the structure.
*/
void processForceData(FSRSensor_TypeDef * sensor_data)
{
//    printf("ADCValue %d \n", sensor_data->ADCValue);
//    printf("voltage %d \n", sensor_data->voltage);
//    printf("coeff: %f - %f - %f - %f - %f \n", sensor_data->coefficients[0],
//                                               sensor_data->coefficients[1],
//                                               sensor_data->coefficients[2],
//                                               sensor_data->coefficients[3],
//                                               sensor_data->coefficients[4]);
//    printf("cal_ref %f \n", sensor_data->offset);
//    printf("cal_ref %f \n", sensor_data->cal_ref);
//    printf("force %f \n", sensor_data->force);
//    printf("force_calculated %f \n", sensor_data->force_calculated);

    float force, force_absolute, force_calculated;
    float voltage = (float)sensor_data->voltage / 1000; //(voltage en V);
    //printf("voltage %f \n", voltage);
   
    force_absolute = sensor_data->coefficients[0];  //first coefficient
    for(int i=1;i<5;i++) //the rest of 4 coefficients
    {
      force_absolute +=  sensor_data->coefficients[i] * (pow(voltage,i)); //(aix^i)  i-coefficient
    }
    //printf("force_absolute %f \n", force_absolute);
    sensor_data->force = force_absolute;
    force = force_absolute - sensor_data -> offset;
    force_calculated = force * sensor_data->cal_ref;
    sensor_data->force_calculated = force_calculated;
       
}