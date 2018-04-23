#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "ak8963.h"
#include "nrf_drv_spi.h"
#include "app_error.h"
#include "nrf_gpio.h"
#include "packet_parser.h"
#include "bytewise.h"
#include "app_scheduler.h"
#include "nrf_drv_gpiote.h"
#include "app_util_platform.h"
#include "nrf_delay.h"
#include "led.h"

#define NRF_LOG_MODULE_NAME "APP"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

nrf_drv_spi_t SPIM_INSTANCE = NRF_DRV_SPI_INSTANCE(0);
#define TX_RX_DATA_LENGTH 	32  
#define SPI_READBIT         (0x80)
#define SCK_PIN       	    17                       /**< SPI clock GPIO pin number. */
#define MOSI_PIN      	    18                       /**< SPI Master Out Slave In GPIO pin number. */
#define MISO_PIN      	    19 
#define SS0_PIN       	    20                      /**< SPI Slave Select GPIO pin number. */
#define INT1_PIN     		12
#define CSB					20
#define RESET_PIN           16
#define RESET(ON)           do{ nrf_gpio_cfg_output(RESET_PIN);\
                            if(ON==1) \
                            {nrf_gpio_pin_set(RESET_PIN);}\
                            else \
                            {nrf_gpio_pin_clear(RESET_PIN);}}while(0)
typedef struct
{
    int16_t AXIS_X;
    int16_t AXIS_Y;
    int16_t AXIS_Z;
}AxesRaw_t;
static uint8_t m_tx_data_spi[TX_RX_DATA_LENGTH]; /**< SPI master TX buffer. */
static uint8_t m_rx_data_spi[TX_RX_DATA_LENGTH]; /**< SPI master RX buffer. */

void spi_init()
{
    uint32_t err_code = NRF_SUCCESS;

    // Configure SPI master.
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.sck_pin  = SCK_PIN;
    spi_config.miso_pin = MISO_PIN;
    spi_config.mosi_pin = MOSI_PIN;
    spi_config.ss_pin   = SS0_PIN;
    spi_config.irq_priority   = APP_IRQ_PRIORITY_LOW;
    
    err_code = nrf_drv_spi_init(&SPIM_INSTANCE, &spi_config,NULL);
    APP_ERROR_CHECK(err_code);
}
void spi_uninit()
{
    nrf_drv_spi_uninit(&SPIM_INSTANCE);
}

bool spi_read(uint8_t reg_addr,uint8_t *data,uint8_t length)
{
    uint32_t error;
    m_tx_data_spi[0] = reg_addr|SPI_READBIT;
	error = nrf_drv_spi_transfer(&SPIM_INSTANCE, m_tx_data_spi,1,m_rx_data_spi, length+1);
	memcpy(data,m_rx_data_spi+1,length);
    return error;
}

bool spi_write(uint8_t reg_addr,uint8_t *data,uint8_t length)
{
    uint32_t error;
    m_tx_data_spi[0] = reg_addr;
    memcpy(m_tx_data_spi+1,data,length);
    error = nrf_drv_spi_transfer(&SPIM_INSTANCE, m_tx_data_spi,length+1,m_rx_data_spi,0);
    return error;
}
uint8_t spi_read_register(uint8_t reg)
{
    uint8_t rdata;
    spi_read(reg,&rdata,1);
    return rdata;
}

uint8_t spi_write_register(uint8_t reg, uint8_t data)
{
    spi_write(reg,&data,1);
    return true;
}


void  spi_read_block(uint8_t reg_addr,uint8_t *data,uint16_t length)
{
    spi_read(reg_addr,data,length);
}

void spi_write_block(uint8_t reg_addr,uint8_t *data,uint16_t length)
{
    spi_write(reg_addr,data,length);  
}
/*
static long long algorithm_timestamp = 0;
static uint16_t rd=0;
void ddd()
{
    static AxesRaw_t Mag;
	static int32_t index = 0;
    spi_init();
	if(spi_read_register(ST1)&DRDY)
	{
		spi_read_block(HXL,(uint8_t*)&Mag,6);
	}
	spi_uninit();
    algorithm_timestamp += 20;
    uint16_t r= (uint16_t)tachymeter_algorithm_data_in_accelerate(Mag.AXIS_X,Mag.AXIS_Y,Mag.AXIS_Z,0,algorithm_timestamp);
    if(r>0)
    {
        rd++;
       indication(INDICATION_COUNT_ADD);
    }
//	if(++index%100 == 0)
//	app_sched_event_put((uint8_t*)&Mag,6,(app_sched_event_handler_t)return_test);
}
uint16_t get_mround()
{
    return rd;
}

*/
#define FLITER_BUFFER_SIZE     10
static AxesRaw_t MagBase = {0,0,0};
static bool magbase_need_cal = true;
bool moto_ajust_f0(int16_t input)
{
    static int16_t fliter_buffer[FLITER_BUFFER_SIZE];
    static uint16_t fliter_size = 0;
    static uint16_t fliter_index = 0;
    int32_t fliter_sum=0;
    int32_t fliter_average=0;
    if(fliter_size==0)
    {
        for(int i=0;i<fliter_size;i++)
        {
            fliter_buffer[i] = input;
        }
        fliter_size = FLITER_BUFFER_SIZE;
        return false;
    }
    
    for(int i=0;i<FLITER_BUFFER_SIZE;i++)
    {
        fliter_sum+=fliter_buffer[i];
    }
    fliter_average = fliter_sum/FLITER_BUFFER_SIZE;    
    
    if(abs((int)fliter_average-(int)input)>30)
    {
        for(int i=0;i<fliter_size;i++)
        {
            fliter_buffer[i] = input;
        }
        fliter_size = FLITER_BUFFER_SIZE;
        return true;
    }
    else
    {
        fliter_buffer[fliter_index]=input;
        fliter_index>(FLITER_BUFFER_SIZE-1)?fliter_index=0:fliter_index++;
        return false;
    }    
}
bool moto_ajust_f1(int16_t input)
{
    static int16_t fliter_average=0;
    static bool first = true;
    bool res;
    if(first)
    {
        first = false;
        fliter_average = input;
    }
    //NRF_LOG_INFO("mag:%6d:%6d\r\n",fliter_average,input); 
    if(abs((int)fliter_average-(int)input)>30)
    {
        res = true;
        fliter_average = input;
    }
    else
    {
        res = false;
    }
    fliter_average = fliter_average*(FLITER_BUFFER_SIZE-1)/FLITER_BUFFER_SIZE+input/FLITER_BUFFER_SIZE;
    return res;
}
#define CAL_THRESHOLD_MAX   1000
#define CAL_THRESHOLD_MIN   300
#define DELTA_MAG       30
bool moto_ajust_f2(AxesRaw_t *input_axes)
{
    if( abs(input_axes->AXIS_X)>CAL_THRESHOLD_MAX
        ||abs(input_axes->AXIS_Y)>CAL_THRESHOLD_MAX
        ||abs(input_axes->AXIS_Z)>CAL_THRESHOLD_MAX)
    {
        magbase_need_cal = true;
        return true;
    }
    else if(magbase_need_cal
        &&abs(input_axes->AXIS_X)<CAL_THRESHOLD_MIN
        &&abs(input_axes->AXIS_Y)<CAL_THRESHOLD_MIN
        &&abs(input_axes->AXIS_Z)<CAL_THRESHOLD_MIN)
    {

        MagBase=*input_axes;
        magbase_need_cal = false;
        return false;
    }
    
    if(abs(MagBase.AXIS_X-input_axes->AXIS_X)>DELTA_MAG
        ||abs(MagBase.AXIS_Y-input_axes->AXIS_Y)>DELTA_MAG
        ||abs(MagBase.AXIS_Z-input_axes->AXIS_Z)>DELTA_MAG)
    {
        return true;
    }
    else
    {
        return false;
    }
}
static uint16_t meas_cnt = 0;
bool moto_ajust_f3(AxesRaw_t *input_axes)
{
    if( abs(input_axes->AXIS_X)>CAL_THRESHOLD_MAX
        ||abs(input_axes->AXIS_Y)>CAL_THRESHOLD_MAX
        ||abs(input_axes->AXIS_Z)>CAL_THRESHOLD_MAX)
    {
        magbase_need_cal = true;
        return true;
    }
    else if(magbase_need_cal
        &&abs(input_axes->AXIS_X)<CAL_THRESHOLD_MIN
        &&abs(input_axes->AXIS_Y)<CAL_THRESHOLD_MIN
        &&abs(input_axes->AXIS_Z)<CAL_THRESHOLD_MIN)
    {
        NRF_LOG_INFO("----cal mag-----\r\n");
        MagBase=*input_axes;
        magbase_need_cal = false;
        
        return false;
    }
    
    if(abs(MagBase.AXIS_X-input_axes->AXIS_X)>DELTA_MAG
        ||abs(MagBase.AXIS_Y-input_axes->AXIS_Y)>DELTA_MAG
        ||abs(MagBase.AXIS_Z-input_axes->AXIS_Z)>DELTA_MAG)
    {
        meas_cnt = 0;
        return true;
    }
    else
    {
        if(++meas_cnt>10)
        {
            magbase_need_cal = true;
            meas_cnt = 0;
        }
        return false;
    }
}

void ccc()
{
    static AxesRaw_t Mag;
    bool ch;
    // static int res;
    spi_init();
	if(spi_read_register(ST1)&DRDY)
	{
		spi_read_block(HXL,(uint8_t*)&Mag,6);
	}
	spi_uninit();
    
    NRF_LOG_INFO("mag:%6d:%6d:%6d\r\n",Mag.AXIS_X,Mag.AXIS_Y,Mag.AXIS_Z); 
    //NRF_LOG_INFO("ajust_f0:%d\r\n",moto_ajust_f0(Mag.AXIS_Z)); 
    //NRF_LOG_INFO("ajust_f1:%d\r\n",ch=moto_ajust_f1(Mag.AXIS_Z));
    NRF_LOG_INFO("ajust_f2:%d\r\n",ch=moto_ajust_f3(&Mag));
    LED1(ch);
    // if(ch)
    // {
        // res++; 
        // LED1(ch);
    // }
    
}
void INT_Handle()
{
	app_sched_event_put(NULL,0,(app_sched_event_handler_t)ccc);
}

uint32_t INT_ON()
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
    nrf_drv_gpiote_in_config_t gpiote_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
    gpiote_config.pull = GPIO_PIN_CNF_PULL_Pulldown;
    nrf_drv_gpiote_in_init(INT1_PIN, &gpiote_config, (nrf_drv_gpiote_evt_handler_t)INT_Handle);
    nrf_drv_gpiote_in_event_enable(INT1_PIN, true);
    return err_code;
}
uint32_t INT_OFF()
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
    nrf_drv_gpiote_in_event_disable(INT1_PIN);
    nrf_drv_gpiote_in_uninit(INT1_PIN);
    
    return err_code;
}


void ak8963_set_mode()
{
	RESET(0);
    nrf_delay_ms(1);
    RESET(1);
    spi_init();
    INT_ON();
	spi_write_register(CNTL1,0x16);
	spi_write_register(CNTL2,0x00);
	
	spi_uninit();
}

void ak8963_single_measure(uint8_t *data)
{
    RESET(0);
    nrf_delay_ms(1);
    RESET(1);
	spi_init();
    INT_ON();
	spi_write_register(CNTL1,0x11);
	spi_write_register(CNTL2,0x00);
    
    spi_uninit();
}




