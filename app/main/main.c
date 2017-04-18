#include <stdint.h>
#include <string.h>
#include "app_error.h"
#include "app_scheduler.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "nrf_delay.h"
#include "boards.h"
#include "nrf_drv_gpiote.h"
#include "bles.h"
#include "communicate.h"
#include "packet_parser.h"
#include "clock.h"
#include "alarm.h"
#include "wdt.h"
#include "app_uart.h"
#include "bsp.h"
#include "bsp_btn_ble.h"
#include "radar.h"
#include "led.h"


#define NRF_LOG_MODULE_NAME "APP"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#define APP_TIMER_PRESCALER             0                               /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE         8                               /**< Size of timer operation queues. */
#define SCHED_MAX_EVENT_DATA_SIZE       8       /**< Maximum size of scheduler events. Note that scheduler BLE stack events do not contain any data, as the events are being pulled from the stack in the event handler. */
#define SCHED_QUEUE_SIZE                32                               /**< Maximum number of events in the scheduler queue. */



                                    
                              

static clock_callbacks_t clock_callbacks;


void radar_send();
void pwm_init();

void mock(uint32_t sec)
{
    
}

void radar_meas_complete(uint8_t * result, uint16_t length)
{
    NRF_LOG_INFO("radar_resault:%02X:%02X:%02X:%02X\r\n",result[0],result[1],result[2],result[3]);  
}
void second_handler(uint32_t second)
{
    wdt_feed();
    if(second%3==2)
    {
        //ble_connection(NULL,0);
        //NRF_LOG_INFO("range_measure\r\n");  
        //range_measure(RANGE_MEAS_MODE_MEAS);
    }
    // if(second<5) return;
    if(second%2==0)
    {
        LED1(1);
        range_measure(RANGE_MEAS_MODE_MEAS);
        //pwm_on();
        //radar_send();
    }
    else
    {
        LED1(0);
        //pwm_off();


    }
        
    //mock(second);
}
void minute_handle(uint32_t minute)
{
    check_alarm();
    //
}
void hour_handle(uint32_t hour)
{

}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void)
{
    // Initialize timer module.
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, NULL);
}


/**@brief Function for starting timers.
*/
static void register_clock_event(void)
{
    clock_callbacks.on_second_update =  (clock_callback_t)second_handler;
    clock_callbacks.on_minute_update = (clock_callback_t)minute_handle;
    clock_callbacks.on_hour_update = (clock_callback_t)hour_handle;
    register_clock_callbacks(&clock_callbacks);
}

static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}

/**@brief Function for the Power manager.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

void bsp_event_handler(bsp_event_t event)
{
    switch (event)
    {
        case BSP_EVENT_SLEEP:

            break;

        case BSP_EVENT_DISCONNECT:

            break;

        case BSP_EVENT_WHITELIST_OFF:

            break;

        default:
            break;
    }
}

/*
static void buttons_leds_init(bool * p_erase_bonds)
{
    bsp_event_t startup_event;

    uint32_t err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS,
                                 APP_TIMER_TICKS(100, APP_TIMER_PRESCALER),
                                 bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}
*/
/**@brief Function for application main entry.
 */
 meas_callbacks_t meas_callbacks;
int main(void)
{
//    uint32_t err_code;
   // bool erase_bonds;
    
                                       
    nrf_drv_gpiote_uninit();
    nrf_drv_gpiote_init();
    (void) NRF_LOG_INIT(NULL);
    timers_init();
    scheduler_init();
	nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);
    //ble_init();
    //buttons_leds_init(&erase_bonds);
    system_clock_init();
    wdt_init(); 
    wdt_start();
    register_clock_event();
    //communicateInit();
    //pwm_on();
    
    NRF_LOG_INFO("Hello\r\n");  
    
    meas_callbacks.on_meas_complete = (meas_callback_t)radar_meas_complete;
    register_range_meas_callbacks(&meas_callbacks);
    
    for (;;)
    {
        
        app_sched_execute();
        power_manage();
    }
}
