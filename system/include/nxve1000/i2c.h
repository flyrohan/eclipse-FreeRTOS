#ifndef _I2C_H
#define _I2C_H

typedef enum {
    I2C_M_RD = 0x0001, /* read data, from slave to master */
} I2C_MSG_FLAGS;

/**
 * I2C_MSG - an I2C message
 *
 * @addr:   Slave address
 * @flags:  Flags (see enum dm_i2c_msg_flags)
 * @len:    Length of buffer in bytes, may be 0 for a probe
 * @buf:    Buffer to send/receive, or NULL if no data
 */
typedef struct {
    uint32_t addr;
    uint32_t flags;
    uint8_t *buf;
    int len;
} I2C_MSG;

typedef enum {
	I2C_OK		 = 0,
	I2C_NOK		 = 1,
	I2C_NACK	 = 2,
	I2C_NOK_TOUT = 3,	/* time out */
} I2C_STATUS;

int I2C_Probe(int ch, uint32_t chip);
int I2C_XFer(int ch, I2C_MSG *msg, int nmsgs);

typedef struct {
	int32_t	scl, sda;
	int32_t	delay;
	int32_t	repeat_start;
} I2C_GPIO_BUS;

int I2C_GPIO_XFer(I2C_GPIO_BUS *bus, I2C_MSG *msg, int nmsgs);
int I2C_GPIO_Probe(I2C_GPIO_BUS *bus, uint8_t addr);

#endif /* _I2C_H */
