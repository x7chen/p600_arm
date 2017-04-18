#include <stdint.h>
#include <string.h>
#include "app_error.h"
#include "nrf_delay.h"
#include "pstorage.h"
#include "nrf_soc.h"
#include "storage_queue.h"
#include "alarm.h"
#include "nv_data.h"
#include "crc16.h"

__align(4) static uint8_t nv_param[NV_PARAM_SIZE];

pstorage_handle_t   nv_param_storage_handle;
uint8_t pstorage_wait_flag = 0;
pstorage_block_t pstorage_wait_handle = 0;

void nv_param_pstorage_cb_handler(   pstorage_handle_t  * handle,
                                        uint8_t              op_code,
                                        uint32_t             result,
                                        uint8_t            * p_data,
                                        uint32_t             data_len)
{
    if(handle->block_id == pstorage_wait_handle)
    {
        pstorage_wait_flag = 0;
    }
}
void nv_param_pstorage_init()
{
    pstorage_module_param_t param;
    uint32_t                retval;

    param.block_size  = 256;                  
    param.block_count = 4;                
    param.cb          = nv_param_pstorage_cb_handler;   //Set the pstorage callback handler

    retval = pstorage_register(&param, &nv_param_storage_handle);
    if (retval != NRF_SUCCESS)
    {
        //printf("pstorage_register NOT SUCCESS\r\n");
    }
    memset(nv_param,0,NV_PARAM_SIZE);
}

uint32_t load_nv_param(nv_param_type_t nv_param_type)
{
    pstorage_handle_t block_handle;
    uint32_t    err_code;
    err_code = pstorage_block_identifier_get(&nv_param_storage_handle,PSTORAGE_BLOCK_PRAM,&block_handle);
    APP_ERROR_CHECK(err_code);
    pstorage_load(nv_param, &block_handle, NV_PARAM_SIZE, 0);
    
    uint16_t crc16;
    uint16_t original = 0x0000;
	crc16 = crc16_compute(nv_param+4,NV_PARAM_SIZE-4,&original);
	uint16_t nv_crc16 = nv_param[0]*256+nv_param[1];
    //校验
	if (nv_crc16 != crc16)
    {
        return 1;
    }

    else
    {
        if(nv_param[FACTORY_SN_OFFEST+ACTIVE_OFFSET]==0x0A
            &&(nv_param_type==NV_PARAM_ALL||nv_param_type==NV_PARAM_FACTORY_SN))
        {
            
        }
        if(nv_param[ALARMS_OFFEST+ACTIVE_OFFSET]==0x0A
            &&(nv_param_type==NV_PARAM_ALL||nv_param_type==NV_PARAM_ALARMS))
        {
            alarm_union_t m_alarm;
            uint8_t cnt = nv_param[ALARMS_OFFEST+LENTGTH_OFFSET]/5;
            uint8_t i;
            clear_alarms();
            for(i=0;i<cnt;i++)
            {
                m_alarm.data = nv_param[ALARMS_OFFEST+DATA_OFFSET+0+i*5];
                m_alarm.data <<= 8;
                m_alarm.data |= nv_param[ALARMS_OFFEST+DATA_OFFSET+1+i*5];
                m_alarm.data <<= 8;
                m_alarm.data |= nv_param[ALARMS_OFFEST+DATA_OFFSET+2+i*5];
                m_alarm.data <<= 8;
                m_alarm.data |= nv_param[ALARMS_OFFEST+DATA_OFFSET+3+i*5];
                m_alarm.data <<= 8;
                m_alarm.data |= nv_param[ALARMS_OFFEST+DATA_OFFSET+4+i*5];
                add_alarm(&m_alarm);
            }
        }
        if(nv_param[USER_PROFILE_OFFEST+ACTIVE_OFFSET]==0x0A
            &&(nv_param_type==NV_PARAM_ALL||nv_param_type==NV_PARAM_USER_PROFILE))
        {
            
        }
        if(nv_param[WEAR_HAND_OFFEST+ACTIVE_OFFSET]==0x0A
            &&(nv_param_type==NV_PARAM_ALL||nv_param_type==NV_PARAM_WEAR_HAND))
        {
            
        }
        if(nv_param[DAILY_TARGET_OFFEST+ACTIVE_OFFSET]==0x0A
            &&(nv_param_type==NV_PARAM_ALL||nv_param_type==NV_PARAM_DAILY_TARGET))
        {
            
        }
        if(nv_param[LONGSIT_PARAM_OFFEST+ACTIVE_OFFSET]==0x0A
            &&(nv_param_type==NV_PARAM_ALL||nv_param_type==NV_PARAM_LONGSIT_PARAM))
        {
            
        }
        if(nv_param[HOUR_FORMAT_OFFEST+ACTIVE_OFFSET]==0x0A
            &&(nv_param_type==NV_PARAM_ALL||nv_param_type==NV_PARAM_HOUR_FORMAT))
        {
            
        }
        return 0;
    }
}
uint32_t save_nv_param(nv_param_type_t nv_param_type)
{
    pstorage_handle_t block_handle;
    uint32_t    err_code;
    uint8_t i;
    uint8_t temp[40];
    if(nv_param_type==NV_PARAM_ALL||nv_param_type==NV_PARAM_FACTORY_SN)
    {
        
    }
    if(nv_param_type==NV_PARAM_ALL||nv_param_type==NV_PARAM_ALARMS)
    {
        
        for(i=0;i<get_alarm_count();i++)
        {
            temp[i*5+0] = get_alarm(i)->data>>32;
            temp[i*5+1] = get_alarm(i)->data>>24;
            temp[i*5+2] = get_alarm(i)->data>>16;
            temp[i*5+3] = get_alarm(i)->data>>8;
            temp[i*5+4] = get_alarm(i)->data;
        }
        nv_param[ALARMS_OFFEST+ACTIVE_OFFSET] = 0x0A;
        nv_param[ALARMS_OFFEST+LENTGTH_OFFSET] = get_alarm_count()*5;
        memcpy(&nv_param[ALARMS_OFFEST+DATA_OFFSET],temp,get_alarm_count()*5);
    }
    if(nv_param_type==NV_PARAM_ALL||nv_param_type==NV_PARAM_USER_PROFILE)
    {
        
    }
    if(nv_param_type==NV_PARAM_ALL||nv_param_type==NV_PARAM_WEAR_HAND)
    {
        
    }
    if(nv_param_type==NV_PARAM_ALL||nv_param_type==NV_PARAM_DAILY_TARGET)
    {
        
    }
    if(nv_param_type==NV_PARAM_ALL||nv_param_type==NV_PARAM_LONGSIT_PARAM)
    {
        
    }
    if(nv_param_type==NV_PARAM_ALL||nv_param_type==NV_PARAM_HOUR_FORMAT)
    {
        
    }
    uint16_t crc16;
    uint16_t original = 0x0000;
    crc16 = crc16_compute(nv_param+4,NV_PARAM_SIZE-4,&original);
	nv_param[CRC16_OFFEST] = crc16>>8;
    nv_param[CRC16_OFFEST+1] = crc16;
    err_code = pstorage_block_identifier_get(&nv_param_storage_handle,PSTORAGE_BLOCK_PRAM,&block_handle);
    APP_ERROR_CHECK(err_code);
    err_code = pstorage_store(&block_handle,nv_param,NV_PARAM_SIZE,0); 
    return 0;
}

