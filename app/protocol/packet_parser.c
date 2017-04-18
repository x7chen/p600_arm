#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "app_scheduler.h"
#include "communicate.h"
#include "packet.h"
#include "packet_parser.h"
#include "ble_nus.h"
#include "bles.h"

#define VERSION_MAJOR   1
#define VERSION_MINOR   0

void resolve(Packet_t * packet)
{
	if(packet->data[0] == 0xA0)
	{
		switch(packet->data[1])
		{
			case 0xC8:
				ble_connection_send();
				break;
			default:
				break;
		}
	}
	else
	{
		ble_nus_string_send(&m_nus, packet->data, packet->length);
	}
    packetClear(packet);
}


void ble_connection_send()
{
	uint8_t data[6];
	data[0] = 0xA0;
	data[1] = 0xC8;
	data[2] = 0x01;
	data[3] = get_connect_state();
	data[4] = 0xCF;
	uart_send(data,5);
}

void ble_connection(uint8_t *value,uint16_t length)
{

}