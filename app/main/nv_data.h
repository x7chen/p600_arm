#ifndef _NV_DATA_H_
#define _NV_DATA_H_

#include <stdint.h>
#include <string.h>

#define NV_PARAM_SIZE           256
#define CRC16_OFFEST            0
#define FACTORY_SN_OFFEST       4
#define ALARMS_OFFEST           64
#define USER_PROFILE_OFFEST     136
#define WEAR_HAND_OFFEST        144
#define DAILY_TARGET_OFFEST     152
#define LONGSIT_PARAM_OFFEST    160
#define HOUR_FORMAT_OFFEST      176

#define ACTIVE_OFFSET           0
#define LENTGTH_OFFSET          2
#define DATA_OFFSET             4

#define PSTORAGE_BLOCK_PRAM     0

typedef enum
{
    NV_PARAM_ALL,
    NV_PARAM_FACTORY_SN,
    NV_PARAM_ALARMS,
    NV_PARAM_USER_PROFILE,
    NV_PARAM_WEAR_HAND,
    NV_PARAM_DAILY_TARGET,
    NV_PARAM_LONGSIT_PARAM,
    NV_PARAM_HOUR_FORMAT,
}nv_param_type_t;

void user_config_pstorage_init(void);
void health_data_storage_init(void);

uint32_t save_nv_param(nv_param_type_t nv_param_type);
uint32_t load_nv_param(nv_param_type_t nv_param_type);

#endif
