#ifndef _AK8963_H_
#define _AK8963_H_

//register map
#define	WIA		0x00
#define	INFO	0x01
#define	ST1		0x02
#define	HXL		0x03
#define	HXH		0x04
#define	HYL		0x05
#define	HYH		0x06
#define	HZL		0x07
#define	HZH		0x08
#define	ST2		0x09
#define	CNTL1	0x0A
#define	CNTL2	0x0B
#define	ASTC	0x0C
#define	TS1		0x0D
#define	TS2		0x0E
#define	I2CDIS	0x0F
#define	ASAX	0x10
#define	ASAY	0x11
#define	ASAZ	0x12
#define	RSV		0x13

#define DOR		0x02
#define DRDY	0x01


uint16_t get_mround(void);
void ak8963_set_mode(void);
void ak8963_single_measure(uint8_t *data);

#endif
