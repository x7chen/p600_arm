#include <stdint.h>
#include <string.h>
#include "app_error.h"
#include "softdevice_handler.h"
#include "communicate.h"
#include "packet.h"
#include "packet_parser.h"
#include "app_timer.h"
#include "app_uart.h"
#include "bles.h"
#include "boards.h"
#include "app_scheduler.h"
#include "ble_nus.h"

#define UART_TX_BUF_SIZE           32                                /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE           32                                  /**< UART RX buffer size. */

APP_TIMER_DEF(delay_send_wait_timer);
APP_TIMER_DEF(receive_time_out_timer);
APP_TIMER_DEF(user_action_delay_timer);

static Packet_t sPacket;
static Packet_t rPacket;
static uint8_t receivebuff[64];
static uint8_t sendbuff[64];
uint8_t communicate_state = STATE_IDLE;

static nus_callbacks_t nus_callbacks;
uint32_t sendACK();

void delaySend(void)
{
}
void receive_time_out_handle(void)
{

}
void user_action_timeout_handle(void)
{
}
Packet_t * getSendPacket(void)
{
	return &sPacket;
}
Packet_t * getReceivePacket(void)
{
	return &rPacket;
}

/**@brief   Function for handling app_uart events.
 *
 * @details This function will receive a single character from the app_uart module and append it to 
 *          a string. The string will be be sent over BLE when the last character received was a 
 *          'new line' i.e '\n' (hex 0x0D) or if the string has reached a length of 
 *          @ref NUS_MAX_DATA_LENGTH.
 */
/**@snippet [Handling the data received over UART] */
void uart_event_handle(app_uart_evt_t * p_event)
{
	uint8_t data_array[20];

	switch (p_event->evt_type)
	{
		case APP_UART_DATA_READY:
			UNUSED_VARIABLE(app_uart_get(&data_array[0]));
			
			receive(data_array,1);
			// app_sched_event_put(data_array,1,(app_sched_event_handler_t)receive);
			// app_sched_event_put(data_array,1,(app_sched_event_handler_t)ebike_receive);
			
			break;

		case APP_UART_COMMUNICATION_ERROR:
			//APP_ERROR_HANDLER(p_event->data.error_communication);
			break;

		case APP_UART_FIFO_ERROR:
			//APP_ERROR_HANDLER(p_event->data.error_code);
			break;

		default:
			break;
	}
}

void uart_error_handle(app_uart_evt_t * p_event)
{
	if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
	{
		APP_ERROR_HANDLER(p_event->data.error_communication);
	}
	else if (p_event->evt_type == APP_UART_FIFO_ERROR)
	{
		APP_ERROR_HANDLER(p_event->data.error_code);
	}
}

static void uart_init(void)
{
	uint32_t                     err_code;
	const app_uart_comm_params_t comm_params =
	{
		RX_PIN_NUMBER,
		TX_PIN_NUMBER,
		RTS_PIN_NUMBER,
		CTS_PIN_NUMBER,
		APP_UART_FLOW_CONTROL_DISABLED,
		false,
		UART_BAUDRATE_BAUDRATE_Baud115200
	};

	APP_UART_FIFO_INIT( &comm_params,
			UART_RX_BUF_SIZE,
			UART_TX_BUF_SIZE,
			uart_event_handle,
			APP_IRQ_PRIORITY_LOW,
			err_code);
	APP_ERROR_CHECK(err_code);
}
void communicateInit(void)
{
	packetInit(&sPacket,sendbuff);
	packetInit(&rPacket,receivebuff);
	uart_init();
	nus_callbacks.on_tx_complete_cb = (nus_callback_t)sendACK;
	register_nus_callbacks(&nus_callbacks);
}
void receive(uint8_t * p_data, uint16_t length)
{
	static uint32_t checkresult;
	appendData(&rPacket,p_data,length);
	checkresult = packetCheck(&rPacket);
	switch(checkresult)
	{
		case 0x01:
		case 0x05: 
		case 0x09:
		case 0x0B:    
			packetClear(&rPacket);
			break;
		case 0x00:
			resolve(getReceivePacket());
			break;
		default:
			break;
	}
}

uint32_t send()
{
	uint32_t error_code = 0;
	uint16_t i;
	for(i=0;i<sPacket.length;i++)
	{
		app_uart_put(sPacket.data[i]);
	}
	return error_code;
}
void uart_send(uint8_t *data, uint16_t length)
{
	uint16_t i;
	for(i=0;i<length;i++)
	{
		app_uart_put(data[i]);
	}
}

uint32_t sendACK()
{
	uint32_t error_code = 0;
	uint16_t i;
	app_uart_put(0xA0);
	for(i=1;i<5;i++)
	{
		app_uart_put(getReceivePacket()->data[i]);
	}
	return error_code;
}

void nus_receive(ble_nus_t * p_nus, uint8_t * p_data, uint16_t length)
{	
	uart_send(p_data,length);
}