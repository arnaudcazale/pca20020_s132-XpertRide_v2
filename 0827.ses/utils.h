#ifndef UTILS_H__
#define UTILS_H__

#include "fds.h"
#include "macros_common.h"
#include "command_interpreter.h"
#include "nrf_log.h"
#include "math.h"



ret_code_t m_fds_write_serial_number(uint16_t, uint16_t, uint8_t*);
ret_code_t m_fds_write_type(uint16_t, uint16_t, uint8_t *);
ret_code_t m_fds_write_fact_lin(uint16_t, uint16_t, float *);
ret_code_t m_fds_write_offset(uint16_t, uint16_t, float *);
ret_code_t m_fds_write_cal_ref(uint16_t, uint16_t, float *);
ret_code_t m_fds_write_bridge(uint16_t, uint16_t, uint16_t *);
uint8_t * m_fds_read_serial_number(uint16_t, uint16_t);
uint8_t * m_fds_read_type(uint16_t, uint16_t);
float * m_fds_read_lin_fact(uint16_t, uint16_t);
float * m_fds_read_tare(uint16_t, uint16_t);
float * m_fds_read_cal_ref(uint16_t, uint16_t);
uint16_t * m_fds_read_bridge(uint16_t, uint16_t);
uint8_t * m_fds_read_gain(uint16_t, uint16_t);
ret_code_t m_fds_find_and_delete (uint16_t, uint16_t);
ret_code_t m_fds_init(void);

#endif // UTILS_H__