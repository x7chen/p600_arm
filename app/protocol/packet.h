#ifndef _PACKET_H_
#define _PACKET_H_
#include "stdint.h"
#define L1_HEADER_VERSION			(0)
#define PACKET_PROPERTY_NOMAL		(L1_HEADER_VERSION)
#define PACKET_PROPERTY_ACK			(0x10+L1_HEADER_VERSION)
#define PACKET_PROPERTY_ERROR_ACK	(0x30+L1_HEADER_VERSION)

#define L1_HEADER_OFFSET			(0)
#define L1_HEADER_LENGTH			(1)
#define L2_HEADER_OFFSET			(L1_HEADER_LENGTH)
#define L2_HEADER_LENGTH			(1)
#define L1L2_HEADER_LENGTH			(L1_HEADER_LENGTH+L2_HEADER_LENGTH)
#define FIRST_VALUE_OFFSET			(L2_HEADER_OFFSET+L2_HEADER_LENGTH)
#define VALUE_HEADER_LENGTH			(1)
#define HEADERS_LENGTH              (L1L2_HEADER_LENGTH+VALUE_HEADER_LENGTH)

#define PACKET_STATE_L1HEADER		(1<<7)
#define PACKET_STATE_L2HEADER		(1<<6)
#define PACKET_STATE_VALUE_CNT		(0)
#define PACKET_STATE_VALUE_CNT_MASK (0x1F)

typedef struct 
{
	uint8_t start_code;
	uint8_t id;
}Packet_L1_Header_t;

typedef struct 
{
	uint8_t command;
}Packet_L2_Header_t;

typedef struct 
{
	uint8_t length;
	uint8_t *data;
}Packet_Value_t;

typedef struct 
{
	uint16_t length;
	uint8_t  state;
	uint8_t  *data;
}Packet_t;
void packetInit(Packet_t *packet,uint8_t *buff);
void packetClear(Packet_t *packet);
void setL1Header(Packet_t *packet,Packet_L1_Header_t *l1Header);
Packet_L1_Header_t *getL1Header(Packet_t *packet);
void setL2Header(Packet_t *packet,Packet_L2_Header_t *l2Header);
void appendValue(Packet_t *packet,Packet_Value_t *value);
void genL1Header(Packet_t *packet);

void appendData(Packet_t *packet,uint8_t *data,uint16_t length);
uint32_t packetCheck(Packet_t *packet);

#endif
