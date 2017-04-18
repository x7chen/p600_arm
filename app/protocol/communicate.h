#ifndef _COMMUNICATE_H_
#define _COMMUNICATE_H_
#include "stdint.h"
#include "stdbool.h"
#include "packet.h"
#include "ble_nus.h"

#define STATE_IDLE 		0x00
#define STATE_SENDING 	0x10
#define STATE_RECEIVING 0x20


void communicateInit(void);
Packet_t * getSendPacket(void);
Packet_t * getReceivePacket(void);
void receive(uint8_t * p_data, uint16_t length);
uint32_t send(void);
void uart_send(uint8_t *data, uint16_t length);
void nus_receive(ble_nus_t * p_nus, uint8_t * p_data, uint16_t length);
#endif
