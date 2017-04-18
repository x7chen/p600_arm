#ifndef _RADAR_H_
#define _RADAR_H_

#include <stdint.h>

#define RANGE_MEAS_MODE_CAL          0x10
#define RANGE_MEAS_MODE_MEAS         0x00

typedef void (*meas_callback_t)(void * content, uint16_t length);
typedef struct{
    meas_callback_t on_meas_complete;
    meas_callback_t on_meas_fault;
}meas_callbacks_t;

uint32_t range_measure_timer_start(void);
void range_measure(uint8_t mode);
void register_range_meas_callbacks(meas_callbacks_t *callbacks);
#endif
