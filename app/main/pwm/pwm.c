#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "nrf_delay.h"
#include "app_error.h"
#include "boards.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#include "nrf_drv_gpiote.h"
#include "nordic_common.h"

#define NRF_LOG_MODULE_NAME "APP"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#define RADAR_POS_PIN           22
#define RADAR_NEG_PIN           23

#define RADAR_PWR_PIN           10


#define RADAR_POWER(ON)     do{\
                                nrf_gpio_cfg_output(RADAR_PWR_PIN);\
                                if(ON==1) \
                                {nrf_gpio_pin_set(RADAR_PWR_PIN);}\
                                else \
                                {nrf_gpio_pin_clear(RADAR_PWR_PIN);}}while(0)


const nrf_drv_timer_t timer2 = NRF_DRV_TIMER_INSTANCE(2);

nrf_ppi_channel_t ppi_channel1;


// Timer even handler. Not used since timer is used only for PPI.
void m_timer_event_handler(nrf_timer_event_t event_type, void * p_context){}

/** @brief Function for initializing the PPI peripheral.
*/
static void ppi_init(void)
{
    uint32_t err_code = NRF_SUCCESS;

    err_code = nrf_drv_ppi_init();
    APP_ERROR_CHECK(err_code);

    // Configure 1st available PPI channel to stop TIMER0 counter on TIMER1 COMPARE[0] match, which is every even number of seconds.
    err_code = nrf_drv_ppi_channel_alloc(&ppi_channel1);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_ppi_channel_assign(ppi_channel1,
                                          nrf_drv_timer_event_address_get(&timer2, NRF_TIMER_EVENT_COMPARE0),
                                          nrf_drv_gpiote_out_task_addr_get(RADAR_POS_PIN));
    APP_ERROR_CHECK(err_code);

    // Enable both configured PPI channels
    err_code = nrf_drv_ppi_channel_enable(ppi_channel1);
    APP_ERROR_CHECK(err_code);
}

__STATIC_INLINE nrf_timer_frequency_t pwm_calculate_timer_frequency(uint32_t period_us)
{
    uint32_t f   = (uint32_t) NRF_TIMER_FREQ_16MHz;
    uint32_t min = (uint32_t) NRF_TIMER_FREQ_31250Hz;

    while ((period_us > 4095) && (f < min))
    {
        period_us >>= 1;
        ++f;
    }


    return (nrf_timer_frequency_t) f;
}


/** @brief Function for Timer 2 initialization.
 *  @details Initializes Timer 2 peripheral, creates event and interrupt every 2 seconds
 *           by configuring CC[0] to half of timer overflow value. Events are created at odd number of seconds.
 *           For example, events are created at 1,3,5,... seconds. This event can be used to start Timer 0
 *           with Timer2->Event_Compare[0] triggering Timer 0 TASK_START through PPI.
*/
static uint32_t timer2_init(void)
{
    uint32_t err_code = NRF_SUCCESS;
    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;

    err_code = nrf_drv_timer_init(&timer2, &timer_cfg,
                                  m_timer_event_handler);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    uint32_t ticks = 25*8;          //40kHz
    nrf_drv_timer_clear(&timer2);
    nrf_drv_timer_extended_compare(&timer2, NRF_TIMER_CC_CHANNEL0,
                                    ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, false);
    nrf_drv_timer_compare_int_disable(&timer2, NRF_TIMER_CC_CHANNEL0);
    
    return err_code;
}

void gpio_init()
{
    nrf_drv_gpiote_init();
    nrf_drv_gpiote_out_config_t config = GPIOTE_CONFIG_OUT_TASK_TOGGLE(false);
    nrf_drv_gpiote_out_init(RADAR_POS_PIN, &config);
    //nrf_drv_gpiote_out_task_force(RADAR_POS_PIN, 1);
    nrf_drv_gpiote_out_task_enable(RADAR_POS_PIN);
}
/**
 * @brief Function for application main entry.
 */
void pwm_on(void)
{
    static bool isInitialized = false;
    if(!isInitialized)
    {
        gpio_init();
        timer2_init(); // Timer to generate events on odd number of seconds.
        ppi_init();    // PPI to redirect the event to timer start/stop tasks.
        RADAR_POWER(1);
        nrf_drv_timer_enable(&timer2);
        isInitialized = true;
    }
    //NRF_POWER->TASKS_CONSTLAT = 1;
//RADAR_POWER(1);
    // Start clock.
    
    nrf_drv_timer_resume(&timer2);

    // Loop and increment the timer count value and capture value into LEDs. @note counter is only incremented between TASK_START and TASK_STOP.
    // while (false)
    // {


// nrf_drv_timer_capture(&timer2, NRF_TIMER_CC_CHANNEL0);
        // /* increment the counter */
        // nrf_drv_timer_increment(&timer2);

        // nrf_delay_ms(100);

    // }
    
}

void pwm_off(void)
{
    //nrf_drv_timer_disable(&timer2);
    nrf_drv_timer_pause(&timer2);
    //RADAR_POWER(0);
}