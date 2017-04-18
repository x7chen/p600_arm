#include <stdint.h>
#include <string.h>
#include "packet.h"
#include "crc16.h"
#include "bytewise.h"
#include "checksum.h"

Packet_L1_Header_t mL1Header;
void packetInit(Packet_t *packet,uint8_t *buff)
{
	packet->data = buff;
	packetClear(packet);
}

void packetClear(Packet_t *packet)
{
	packet->length = 0;
	packet->state = 0;
}

void setL1Header(Packet_t *packet,Packet_L1_Header_t *l1Header)
{
	if(packet->state&PACKET_STATE_L1HEADER)
    {
        
    }
    else
    {
        packet->length += L1_HEADER_LENGTH;
        packet->state |= PACKET_STATE_L1HEADER;
    }
    if(l1Header!=NULL)
    {
        memcpy(packet->data+L1_HEADER_OFFSET,(uint8_t *)l1Header,L1_HEADER_LENGTH);
    }	
}
Packet_L1_Header_t *getL1Header(Packet_t *packet)
{
	
	memcpy((uint8_t *)&mL1Header,packet->data+L1_HEADER_OFFSET,L1_HEADER_LENGTH);
	return &mL1Header;
}
void setL2Header(Packet_t *packet,Packet_L2_Header_t *l2Header)
{
    if(packet->state&PACKET_STATE_L2HEADER)
    {
        
    }
    else
    {
        packet->length += L2_HEADER_LENGTH;
        packet->state |= PACKET_STATE_L2HEADER;
    }
    if(l2Header!=NULL)
    {
        memcpy(packet->data+L2_HEADER_OFFSET,(uint8_t *)l2Header,L2_HEADER_LENGTH);
    }
}

void appendValue(Packet_t *packet,Packet_Value_t *value)
{
	if((packet->state&PACKET_STATE_L1HEADER)==0)
    {
        setL1Header(packet,NULL);
    }
    if((packet->state&PACKET_STATE_L2HEADER)==0)
    {
        setL2Header(packet,NULL);
    }
    
    appendData(packet,&(value->length),1);
    appendData(packet,value->data,value->length);
    packet->state++;
    
}
void appendData(Packet_t *packet,uint8_t *data,uint16_t length)
{
    memcpy(packet->data + packet->length,data,length);
	packet->length+=length;
}
void genL1Header(Packet_t *packet)
{
	Packet_L1_Header_t l1Header;
    uint8_t check_sum;
	uint8_t end_byte = 0xEF;
	if((packet->state&PACKET_STATE_VALUE_CNT_MASK)\
        ||((packet->state&PACKET_STATE_L2HEADER)))
	{
        l1Header.start_code = 0xEF;
		setL1Header(packet,&l1Header);
        check_sum = checksum(packet->data,packet->length);
        appendData(packet,&check_sum,1);
		appendData(packet,&end_byte,1);
	}
    
}

uint32_t packetCheck(Packet_t *packet)
{
	if(packet->length >200)
    {
        return 0x01;
    }
    //没有L1Header
//	if(packet->length < L1_HEADER_LENGTH)
//	{
//		return 0x03;
//	}
    //检验包头
	Packet_L1_Header_t * l1Header = (Packet_L1_Header_t *)(packet->data);
//	if(l1Header->start_code!=0x65||l1Header->id != 0x85)
//	{
//		return 0x05;
//	}
    if(packet->length==1)
	{
		if((l1Header->start_code==0xEF)||(l1Header->start_code==0xA0))
		{
			
		}
		else
		{
			return 0x05;
		}
	}
		
	
    //数据长度不够校验
	if(packet->length < 3)
	{
		return 0x07;
	}
    //校验数据
	uint16_t length = packet->data[2]+5;
    if(packet->length < length)
	{
		return 0x08;
	}
//    return 0x0;
	if(packet->length > length)
	{
		return 0x09;
	}
	if(packet->data[length-1]==0xCF)
	{
		return 0x0;
	}
	else
	{
		return 0x0A;
	}
    
/* 	uint8_t check_sum;
	check_sum = checksum(packet->data,packet->length-1);
    //校验成功
	if (check_sum == packet->data[packet->length-1])
	{
		return 0x0;
	}
    //校验失败
	else
	{
		return 0x0B;
	}
*/	
} 

