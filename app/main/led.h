#ifndef _LED_H_
#define _LED_H_
#include "nrf_drv_gpiote.h"
#define LED1_PIN            3
#define LED2_PIN            4

#define LED1(Hi)     do{\
                                if(Hi==0) \
                                {nrf_gpio_cfg_input(LED1_PIN,NRF_GPIO_PIN_NOPULL);}\
                                else \
                                {nrf_gpio_cfg_output(LED1_PIN);\
                                nrf_gpio_pin_set(LED1_PIN);}}while(0)
#define LED2(Hi)     do{\
                                if(Hi==0) \
                                {nrf_gpio_cfg_input(LED2_PIN,NRF_GPIO_PIN_NOPULL);}\
                                else \
                                {nrf_gpio_cfg_output(LED2_PIN);\
                                nrf_gpio_pin_set(LED2_PIN);}}while(0)
                                    
                                
#endif

