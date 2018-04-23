#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "ak8963.h"
#include "m_twi_master.h"
#include "app_error.h"
#include "nrf_gpio.h"

#define I2C_DEVICE_ADDR		0x0C
#define I2C_SDA_PIN   		16
#define I2C_SCL_PIN      	15 

#define TX_RX_DATA_LENGTH 	32  
static uint8_t m_twi_tx_buf[TX_RX_DATA_LENGTH]; /**< translate buffer. */
void i2c_init()
{
    twi_master_init(I2C_SDA_PIN,I2C_SCL_PIN);
}

void i2c_uninit()
{
    nrf_gpio_cfg_default(I2C_SDA_PIN);
    nrf_gpio_cfg_default(I2C_SCL_PIN);
}

bool i2c_read(uint8_t reg_addr,uint8_t *data,uint8_t length)
{
    bool transfer_succeeded;
    transfer_succeeded = twi_master_transfer((I2C_DEVICE_ADDR<<1), &reg_addr, 1, TWI_DONT_ISSUE_STOP);
    transfer_succeeded &= twi_master_transfer((I2C_DEVICE_ADDR<<1)|TWI_READ_BIT, data, length, TWI_ISSUE_STOP);   
    return transfer_succeeded;
}

bool i2c_write(uint8_t reg_addr,uint8_t *data,uint8_t length)
{
    bool transfer_succeeded;
    m_twi_tx_buf[0] = reg_addr;
    memcpy(m_twi_tx_buf+1,data,length);
    transfer_succeeded = twi_master_transfer((I2C_DEVICE_ADDR<<1), m_twi_tx_buf, length+1, TWI_ISSUE_STOP); 
    return transfer_succeeded;
}

uint8_t i2c_read_register(uint8_t reg_addr)
{
    uint8_t data;
    i2c_read(reg_addr,&data,1);    
    return data;
}

void i2c_write_register(uint8_t reg_addr,uint8_t data)
{
    i2c_write(reg_addr,&data,1);  
}

void  i2c_read_block(uint8_t reg_addr,uint8_t *data,uint16_t length)
{
    i2c_read(reg_addr,data,length);
}

void i2c_write_block(uint8_t reg_addr,uint8_t *data,uint16_t length)
{
    i2c_write(reg_addr,data,length);  
}

void ak8963_set_mode()
{
	i2c_init();
	i2c_write_register(CNTL1,0x16);
	i2c_write_register(CNTL2,0x00);
	i2c_uninit();
}

void ak8963_single_measure(uint8_t *data)
{
	uint16_t wait=1000;
	memset(data,0,6);
	i2c_init();
	i2c_write_register(CNTL1,0x11);
	i2c_write_register(CNTL2,0x00);
	while(wait--)
	{
		if(i2c_read_register(ST1)&DRDY)
		{
			i2c_read_block(HXL,data,6);
			break;
		}
	}
	i2c_uninit();
}




