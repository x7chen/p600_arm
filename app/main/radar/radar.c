#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "app_error.h"
#include "nrf_delay.h"
#include "nrf_drv_timer.h"
#include "nrf_drv_gpiote.h"
#include "app_timer.h"
#include "app_scheduler.h"
#include "radar.h"
#include "app_pwm.h"
#include "bytewise.h"
#include "led.h"
#include "nrf_log.h"


#define RADAR_ECHO_PIN          25

#define RADAR_PWR_PIN           10
#define RADAR_POS_PIN           22
#define RADAR_NEG_PIN           23
#define RADAR_TRI_PIN           21
#define MEAS_DEVIATION          200
#define APP_TIMER_PRESCALER     0
#define MEAS_INTERVAL           APP_TIMER_TICKS(100, APP_TIMER_PRESCALER)

#define RADAR_POWER(ON)     do{\
                                nrf_gpio_cfg_output(RADAR_PWR_PIN);\
                                if(ON==1) \
                                {nrf_gpio_pin_set(RADAR_PWR_PIN);}\
                                else \
                                {nrf_gpio_pin_clear(RADAR_PWR_PIN);}}while(0)

APP_TIMER_DEF(m_range_meas_timer_id);
APP_PWM_INSTANCE(PWM1,2); 
const nrf_drv_timer_t TIMER_RANGE_MEASRUE = NRF_DRV_TIMER_INSTANCE(1);

bool measure_data_updated = false;
bool measure_is_Initialized = false;
bool measure_is_Idle = true; 
static uint32_t single_value=0;                                    
static uint32_t measure_value=0;
static uint32_t measure_base_value= 300 ; 
static uint8_t meas_mode = 0;                                    
static meas_callbacks_t * meas_callbacks = NULL;
void measure_handle(uint8_t mode); 
uint32_t range_measure_int_off();
static volatile bool ready_flag; 
void pwm_ready_callback(uint32_t pwm_id)    // PWM callback function
{
    ready_flag = true;
}

void pwm_init()
{
    ret_code_t err_code;  
    
    app_pwm_config_t pwm1_cfg = APP_PWM_DEFAULT_CONFIG_2CH(25, RADAR_POS_PIN, RADAR_NEG_PIN);
    //app_pwm_config_t pwm1_cfg = APP_PWM_DEFAULT_CONFIG_1CH(25L, RADAR_TRI_PIN);
    /* Switch the polarity of the second channel. */
    //pwm1_cfg.pin_polarity[1] = APP_PWM_POLARITY_ACTIVE_HIGH;
    pwm1_cfg.pin_polarity[1] = APP_PWM_POLARITY_ACTIVE_LOW;
    /* Initialize and enable PWM. */
    err_code = app_pwm_init(&PWM1,&pwm1_cfg,pwm_ready_callback);
    APP_ERROR_CHECK(err_code);
    // NVIC_SetPriority(TIMER2_IRQn, 3); 
    app_pwm_enable(&PWM1);
    RADAR_POWER(1);
 
    // while ((app_pwm_channel_duty_set(&PWM1, 0, 50) == NRF_ERROR_BUSY));
    // while ((app_pwm_channel_duty_set(&PWM1, 1, 50) == NRF_ERROR_BUSY));
}
void radar_send()
{
/*
    int timeout = 1000;
    while ((timeout--)&&(app_pwm_channel_duty_set(&PWM1, 0, 50) == NRF_ERROR_BUSY));
    timeout = 1000;
    while ((timeout--)&&(app_pwm_channel_duty_set(&PWM1, 1, 50) == NRF_ERROR_BUSY));
    nrf_delay_us(200);
    timeout = 1000;
    while ((timeout--)&&(app_pwm_channel_duty_set(&PWM1, 0, 0) == NRF_ERROR_BUSY));
    timeout = 1000;
    while ((timeout--)&&(app_pwm_channel_duty_set(&PWM1, 1, 0) == NRF_ERROR_BUSY));
*/ 
    while ((app_pwm_channel_duty_set(&PWM1, 0, 50) == NRF_ERROR_BUSY));
    while ((app_pwm_channel_duty_set(&PWM1, 1, 50) == NRF_ERROR_BUSY)); 
    nrf_delay_us(400);
    while ((app_pwm_channel_duty_set(&PWM1, 0, 0) == NRF_ERROR_BUSY));
    while ((app_pwm_channel_duty_set(&PWM1, 1, 0) == NRF_ERROR_BUSY));  
// app_pwm_enable(&PWM1);
    // nrf_delay_us(10000);
// app_pwm_disable(&PWM1);    
    // pwm_on();
    // nrf_delay_us(600);
    // pwm_off();
    
}



                                    
void timer_event_handler(nrf_timer_event_t event_type, void* p_context)
{

}
void range_measure_int_handle()
{
    range_measure_int_off();
    //single_value = nrf_drv_timer_capture_get(&TIMER_RANGE_MEASRUE,NRF_TIMER_CC_CHANNEL0);
    single_value = nrf_drv_timer_capture(&TIMER_RANGE_MEASRUE,NRF_TIMER_CC_CHANNEL0);
    measure_value += single_value;
    //NRF_LOG_INFO("measure_value:%08X",measure_value);
    measure_data_updated = true;
    // static uint8_t i=0;
    // i++;
    // LED2((0x01&i));
    LED2(1);
}
uint32_t range_measure_int_on()
{
    uint32_t err_code = 0;
    if (!nrf_drv_gpiote_is_init())
    {
        err_code = nrf_drv_gpiote_init();
        if (err_code != NRF_SUCCESS)
        {
            return err_code;
        }
    }
    nrf_drv_gpiote_in_config_t gpiote_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    gpiote_config.pull = NRF_GPIO_PIN_NOPULL;
    nrf_drv_gpiote_in_init(RADAR_ECHO_PIN, &gpiote_config, (nrf_drv_gpiote_evt_handler_t)range_measure_int_handle);
    nrf_drv_gpiote_in_event_enable(RADAR_ECHO_PIN, true);
    return err_code;
}
uint32_t range_measure_int_off()
{
    uint32_t err_code = 0;
    nrf_drv_gpiote_in_event_disable(RADAR_ECHO_PIN);
    nrf_drv_gpiote_in_uninit(RADAR_ECHO_PIN);
    return err_code;
}
void range_measure_init()
{
    uint32_t err_code =0;
    nrf_drv_timer_config_t timer_config = NRF_DRV_TIMER_DEFAULT_CONFIG;
    timer_config.frequency = NRF_TIMER_FREQ_1MHz;
    RADAR_POWER(1);
    nrf_delay_ms(5);
    err_code = nrf_drv_timer_init(&TIMER_RANGE_MEASRUE, &timer_config, timer_event_handler);

    APP_ERROR_CHECK(err_code);
    nrf_drv_timer_enable(&TIMER_RANGE_MEASRUE);

    //range_measure_int_on();
    
    pwm_init();
    measure_is_Initialized = true;
}
void range_measure_uninit()
{
    //RADAR_POWER(0);
    nrf_drv_timer_disable(&TIMER_RANGE_MEASRUE);
    nrf_drv_timer_uninit(&TIMER_RANGE_MEASRUE);
    app_pwm_uninit(&PWM1);
    range_measure_int_off();
    measure_is_Initialized = false;
}
void range_app_timer_scheduler()
{
    measure_handle(meas_mode|0x01);
    LED2(0);
}
void range_app_timer_handle(void * p_context)
{
    (void) p_context;
    app_sched_event_put(NULL,0,(app_sched_event_handler_t)range_app_timer_scheduler);
}
uint32_t range_measure_timer_start()
{
    uint32_t err_code =0;
    static bool meas_timer_is_initialized = false;
    if(!meas_timer_is_initialized)
    {
        err_code = app_timer_create(&m_range_meas_timer_id,APP_TIMER_MODE_SINGLE_SHOT,range_app_timer_handle);
        APP_ERROR_CHECK(err_code);
        meas_timer_is_initialized = true;
    }
    err_code = app_timer_start(m_range_meas_timer_id,MEAS_INTERVAL,NULL);
    APP_ERROR_CHECK(err_code);
    
    
    return err_code;
}

uint16_t compute_range(uint32_t time)
{
    return (uint16_t)(time/100);   
}


/*****************************************************************************
* -----||----------50ms-----------|-----------50ms-----------|-----------  ...
*       ^                        
*1.开启测量指示灯                 ^
*2.开始充电                       | 
*3.开启定时器                     |
*                            1.开始放电
*                            2.读数据
*                            3.开始充电
*                            4.下一轮定时
******************************************************************************
* mode = 0x00 : 测量开始
* mode = 0x01 ：测量进行中
* mode = 0x10 : 校准开始
* mode = 0x11 : 校准进行中
*****************************************************************************/
#define MEAS_REPEAT      1
void measure_handle(uint8_t mode)
{
#if 0    
    static uint8_t index=0;
    measure_base_value = index;
    if((mode&0x0f) == 0)
    {
        index=0;
    }
    if(!measure_is_Initialized)
    {
        range_measure_init();
    }
    index++;
    LED2((0x01&index));
    radar_send();
    if(index < MEAS_REPEAT)
    {
        range_measure_timer_start();
    }
    else
    {
        measure_is_Idle = true; 
    }
#else

    static uint8_t index=0;
    uint16_t range;
    uint8_t buffer[4];
    if((mode&0x0f) == 0)
    {
        index=0;
    }
    if(!measure_is_Initialized)
    {
        range_measure_init();
    }
    //LED2((0x01&index));
    //radar_send();
    //NRF_LOG_INFO("index:%d\r\n",index);  
    if(index == 0)
    {
        //测量开始，指示灯提示
        measure_value = 0;
        index++;
        range_measure_timer_start();
        nrf_drv_timer_clear(&TIMER_RANGE_MEASRUE);
        range_measure_int_on();
        radar_send();
        return;
    }
    if(index < MEAS_REPEAT)
    {
        index++;
        //开启下一次测量
        range_measure_timer_start();
        nrf_drv_timer_clear(&TIMER_RANGE_MEASRUE);
        range_measure_int_on();
        radar_send();
    }
    else if(index == MEAS_REPEAT)
    {
        index++;
        measure_value /= MEAS_REPEAT;
        if((mode & 0xF0) == RANGE_MEAS_MODE_CAL)
        {
            measure_base_value = measure_value;
            range_measure_uninit();
            measure_is_Idle = true; 
            return;
        }
        //measure_value -= (measure_base_value-50);
        range = compute_range(measure_value);
        range=range;
        buffer[0] = (measure_value>>24)&0xff;
        buffer[1] = (measure_value>>16)&0xff;
        buffer[2] = (measure_value>>8)&0xff;
        buffer[3] = (measure_value)&0xff;
        buffer[3] = buffer[3];
        NRF_LOG_INFO("measure_value:%8d\r\n",measure_value);
        if(meas_callbacks != NULL)
        {
            //meas_callbacks->on_meas_complete(buffer,4);
        }
        range_measure_timer_start();
        // nrf_drv_timer_clear(&TIMER_RANGE_MEASRUE);
        // range_measure_int_on();
        // radar_send();
    }
    else
    {
        index++;
        range_measure_uninit();
        measure_is_Idle = true;
    }
#endif
}

void range_measure(uint8_t mode)
{
    if(measure_is_Idle)
    {
        meas_mode = mode;
        measure_handle(meas_mode);
        measure_is_Idle = false;
    }
    else
    {
        if(meas_callbacks != NULL)
        {
            meas_callbacks->on_meas_fault(NULL,0);
        }
    }
}

void meas_callback_is_null(void * content, uint16_t length)
{
    
}
void register_range_meas_callbacks(meas_callbacks_t *callbacks)
{
    meas_callbacks = callbacks;
    if(meas_callbacks->on_meas_complete == NULL)
    {
        meas_callbacks->on_meas_complete = meas_callback_is_null;
    }
    if(meas_callbacks->on_meas_fault == NULL)
    {
        meas_callbacks->on_meas_fault = meas_callback_is_null;
    }
}

