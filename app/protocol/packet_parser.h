#ifndef _PACKET_PARSER_H_
#define _PACKET_PARSER_H_
#include "stdint.h"
#include "packet.h"


#define L1VERSION       (0)
#define L2VERSION       (0)

void resolve(Packet_t * packet);
void ble_connection_send(void);
void ble_connection(uint8_t *value,uint16_t length);

#endif
