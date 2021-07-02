#include <config.h>
#include <rtos.h>
#include <i2c.h>
#include <io.h>

#ifdef I2C_ENABLED

typedef struct __I2C_REG {
	uint32_t	I2CCON;
	uint32_t	I2CSTAT;
	uint32_t	I2CADD;
	uint32_t	I2CDS;
	uint32_t	I2CLC;
} I2C_REG;

typedef struct __I2C_BUS {
	I2C_REG *reg;
} I2C_BUS;

/* I2C Controller bits */
#define I2CSTAT_BSY		0x20	/* Busy bit */
#define I2CSTAT_NACK	0x01	/* Nack bit */
#define I2CCON_ACKGEN	0x80	/* Acknowledge generation */
#define I2CCON_IRPND	0x10	/* Interrupt pending bit */
#define I2C_MODE_MT		0xC0	/* Master Transmit Mode */
#define I2C_MODE_MR		0x80	/* Master Receive Mode */
#define I2C_START_STOP	0x20	/* START / STOP */
#define I2C_TXRX_ENA	0x10	/* I2C Tx/Rx enable */
#define I2C_TIMEOUT_MS 	(100)	/* 10 ms */

static const I2C_BUS i2c_bus[2] = {
	[0] = { .reg = (I2C_REG *)0x43000000, },
	[1] = { .reg = (I2C_REG *)0x43010000, },
};


#define __DEBUG__
#ifdef __DEBUG__
#define DBG(msg...)		Printf(msg);
#else
#define DBG(msg...)
#endif
#define ERR(msg...)		Printf(msg);

#ifdef RTOS_ENABLED
 #ifndef CMSIS_ENABLED
 #define GetTime()		xTaskGetTickCount()
 #else
 #define GetTime()		osKernelGetTickCount()
 #endif
#else
#define GetTime()		SysTime_GetTime()
#endif

static int WaitForXfer(I2C_REG *i2c, uint32_t timeout)
{
	uint32_t start_time = GetTime();

	do {
		if (readl(&i2c->I2CCON) & I2CCON_IRPND)
			return (readl(&i2c->I2CSTAT) & I2CSTAT_NACK) ?
				I2C_NACK : I2C_OK;
	} while ((GetTime() - start_time) < timeout);

	return I2C_NOK_TOUT;
}

static void read_write_byte(I2C_REG *i2c)
{
	clr_bit(&i2c->I2CCON, I2CCON_IRPND);
}

static int i2c_do_msg(int ch, struct i2c_msg *msg, int seq)
{
	I2C_REG *i2c = i2c_bus[ch].reg;
	int is_read = msg->flags & I2C_M_RD;
	uint32_t i2cstat;
	uint32_t addr;
	int ret, i;

	if (!seq)
		set_bit(&i2c->I2CCON, I2CCON_ACKGEN);

	/* Get the slave chip address going */
	addr = msg->addr << 1;
	writel(addr, &i2c->I2CDS);
	i2cstat = I2C_TXRX_ENA | I2C_START_STOP;

	if (is_read)
		i2cstat |= I2C_MODE_MR;
	else
		i2cstat |= I2C_MODE_MT;

	writel(i2cstat, &i2c->I2CSTAT);

	if (seq)
		read_write_byte(i2c);

	/* Wait for chip address to transmit */
	ret = WaitForXfer(i2c, I2C_TIMEOUT_MS);
	if (ret)
		goto err;

	if (is_read) {
		for (i = 0; !ret && i < msg->len; i++) {
			if (i == msg->len - 1)
				clr_bit(&i2c->I2CCON, I2CCON_ACKGEN);

			read_write_byte(i2c);
			ret = WaitForXfer(i2c, I2C_TIMEOUT_MS);

			msg->buf[i] = (uint8_t)readl(&i2c->I2CDS);
		}

		if (ret == I2C_NACK)
			ret = I2C_OK; /* Normal terminated read */

	} else {
		for (i = 0; !ret && i < msg->len; i++) {
			writel(msg->buf[i], &i2c->I2CDS);
			read_write_byte(i2c);
			ret = WaitForXfer(i2c, I2C_TIMEOUT_MS);
		}
	}

err:
	return ret;
}

static void i2c_set_bus(int ch)
{
	I2C_REG *i2c = i2c_bus[ch].reg;
	int filter_enable = 1;
	int sda_delay = 1;

	ERR("Set CLK Enable And Set Alt Function !!!");
	return;

	/* set divider and set ACKGEN, IRQ */
	writel(0xA0 | 0x40,  &i2c->I2CCON);

	/* set SLAVE REVEIVE and set slaveaddr */
	writel(0, &i2c->I2CSTAT);
	writel(0, &i2c->I2CADD);

	/* program Master Transmit (and implicit STOP) */
	writel(I2C_MODE_MT | I2C_TXRX_ENA, &i2c->I2CSTAT);
	writel(((sda_delay & 0x3) | filter_enable << 2), &i2c->I2CLC);
}

int i2c_xfer(int ch, struct i2c_msg *msg, int nmsgs)
{
	I2C_REG *i2c = i2c_bus[ch].reg;
	uint32_t start_time;
	int ret, i;

	i2c_set_bus(ch);

	start_time = GetTime();
	while (readl(&i2c->I2CSTAT) & I2CSTAT_BSY) {
		if ((GetTime() - start_time)  > I2C_TIMEOUT_MS) {
			ERR("Timeout\r\n");
			return -I2C_NOK_TOUT;
		}
	}

	for (ret = 0, i = 0; !ret && i < nmsgs; i++)
		ret = i2c_do_msg(ch, &msg[i], i);

	/* Send STOP */
	writel(I2C_MODE_MR | I2C_TXRX_ENA, &i2c->I2CSTAT);
	read_write_byte(i2c);

	return ret ? -I2C_NOK : I2C_OK;
}

int i2c_probe(int ch, uint32_t chip)
{
	uint8_t buf;
    struct i2c_msg msg = {
    	.addr = chip,
    	.flags = !I2C_M_RD,
    	.buf = &buf,
    	.len = 1,
    };

	return (int)i2c_xfer(ch, &msg, 1);
}
#endif
