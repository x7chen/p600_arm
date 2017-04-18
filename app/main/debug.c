#include "nrf.h"
#include <stdio.h>
#include "app_error.h"
#include "app_error_weak.h"
#include "nordic_common.h"
#include "sdk_errors.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include <string.h>

void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    static volatile struct
    {
        uint32_t        fault_id;
        uint32_t        pc;
        uint32_t        error_info;
        assert_info_t * p_assert_info;
        error_info_t  * p_error_info;
        ret_code_t      err_code;
        uint32_t        line_num;
        const uint8_t * p_file_name;
    } m_error_data = {0};

    // The following variable helps Keil keep the call stack visible, in addition, it can be set to
    // 0 in the debugger to continue executing code after the error check.
    volatile bool loop = true;
    UNUSED_VARIABLE(loop);

    m_error_data.fault_id   = id;
    m_error_data.pc         = pc;
    m_error_data.error_info = info;

    switch (id)
    {
        case NRF_FAULT_ID_SDK_ASSERT:
            m_error_data.p_assert_info = (assert_info_t *)info;
            m_error_data.line_num      = m_error_data.p_assert_info->line_num;
            m_error_data.p_file_name   = m_error_data.p_assert_info->p_file_name;
            break;

        case NRF_FAULT_ID_SDK_ERROR:
            m_error_data.p_error_info = (error_info_t *)info;
            m_error_data.err_code     = m_error_data.p_error_info->err_code;
            m_error_data.line_num     = m_error_data.p_error_info->line_num;
            m_error_data.p_file_name  = m_error_data.p_error_info->p_file_name;
            break;
    }
    char msg[64]={0};
    memcpy(msg,m_error_data.p_file_name,20);
    NRF_LOG_INFO("LineNum:%06d\r\n",m_error_data.line_num);
    NRF_LOG_INFO("FileName:%s\r\n",nrf_log_push(msg));
    NRF_LOG_ERROR("received a fault! id: 0x%08x, pc: 0x%08x\r\n", id, pc);
    UNUSED_VARIABLE(m_error_data);
    
    //NVIC_SystemReset();
    //while (loop);
}
