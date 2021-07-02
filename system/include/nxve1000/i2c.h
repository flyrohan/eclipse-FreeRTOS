/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012 Samsung Electronics
 */

#ifndef _S3C24X0_I2C_H
#define _S3C24X0_I2C_H

/*
 * Not all of these flags are implemented in the U-Boot API
 */
enum dm_i2c_msg_flags {
    I2C_M_TEN       = 0x0010, /* ten-bit chip address */
    I2C_M_RD        = 0x0001, /* read data, from slave to master */
    I2C_M_STOP      = 0x8000, /* send stop after this message */
    I2C_M_NOSTART       = 0x4000, /* no start before this message */
    I2C_M_REV_DIR_ADDR  = 0x2000, /* invert polarity of R/W bit */
    I2C_M_IGNORE_NAK    = 0x1000, /* continue after NAK */
    I2C_M_NO_RD_ACK     = 0x0800, /* skip the Ack bit on reads */
    I2C_M_RECV_LEN      = 0x0400, /* length is first received byte */
};

/**
 * struct i2c_msg - an I2C message
 *
 * @addr:   Slave address
 * @flags:  Flags (see enum dm_i2c_msg_flags)
 * @len:    Length of buffer in bytes, may be 0 for a probe
 * @buf:    Buffer to send/receive, or NULL if no data
 */
struct i2c_msg {
    uint32_t addr;
    uint32_t flags;
    uint8_t *buf;
    int len;
};

typedef enum {
	I2C_OK		 = 0,
	I2C_NOK		 = 1,
	I2C_NACK	 = 2,
	I2C_NOK_TOUT = 3,	/* time out */
} I2C_STATUS;

int i2c_probe(int ch, uint32_t chip);
int i2c_xfer(int ch, struct i2c_msg *msg, int nmsgs);

#endif /* _S3C24X0_I2C_H */
