#include <config.h>
#include <rtos.h>
#include <time.h>
#include <i2c.h>

#ifdef I2C_GPIO_ENABLED

#define I2C_GET_BIT(io) 		((uint8_t)io)
#define I2C_SDA_BIT(io, bit)	do { io = bit; } while (0)
#define I2C_SCL_BIT(io, bit)	do { io = bit; } while (0)
#ifdef RTOS_ENABLED
#ifdef CMSIS_ENABLED
#define I2C_DELAY(_t)		osDelay(_t)
#else
#define I2C_DELAY(_t)		vTaskDelay((TickType_t)_t)
#endif
#else
#define I2C_DELAY(_t)		SysTime_Delay(_t)
#endif

#ifdef __DEBUG__
#define DBG(msg...)	Printf(msg);
#else
#define DBG(msg...)
#endif
#define ERR(msg...)	Printf(msg);

typedef enum {
	BIT_L	= 0,
	BIT_H	= 1,
} BIT_LEVEL;

static void I2C_GPIO_START(I2C_GPIO_BUS *bus)
{
	I2C_DELAY(bus->delay);
	I2C_SDA_BIT(bus->sda, BIT_H);
	I2C_DELAY(bus->delay);
	I2C_SCL_BIT(bus->scl, BIT_H);
	I2C_DELAY(bus->delay);
	I2C_SDA_BIT(bus->sda, BIT_L);
	I2C_DELAY(bus->delay);
}

static void I2C_GPIO_STOP(I2C_GPIO_BUS *bus)
{
	I2C_SCL_BIT(bus->scl, BIT_L);
	I2C_DELAY(bus->delay);
	I2C_SDA_BIT(bus->sda, BIT_L);
	I2C_DELAY(bus->delay);
	I2C_SCL_BIT(bus->scl, BIT_H);
	I2C_DELAY(bus->delay);
	I2C_SDA_BIT(bus->sda, BIT_H);
	I2C_DELAY(bus->delay);
}

static int I2C_GPIO_TransByte(I2C_GPIO_BUS *bus, uint8_t data)
{
	int Nack, value = (int)data;

	for(int i = 0; i < 8; i++) {
		I2C_SCL_BIT(bus->scl, BIT_L);
		I2C_DELAY(bus->delay);
		I2C_SDA_BIT(bus->sda, (value & 0x80) ? BIT_H : BIT_L);
		I2C_DELAY(bus->delay);
		I2C_SCL_BIT(bus->scl, BIT_H);
		I2C_DELAY(bus->delay);
		I2C_DELAY(bus->delay);
		value <<= 1;
	}

	I2C_SCL_BIT(bus->scl, BIT_L);
	I2C_DELAY(bus->delay);
	I2C_SDA_BIT(bus->sda, BIT_H);
	I2C_DELAY(bus->delay);
	I2C_SCL_BIT(bus->scl, BIT_H);
	I2C_DELAY(bus->delay);
	I2C_DELAY(bus->delay);

	Nack = I2C_GET_BIT(bus->sda);

	I2C_SCL_BIT(bus->scl, BIT_L);
	I2C_DELAY(bus->delay);

	return (Nack);	/* not a nack is an ack */
}

static uint8_t I2C_GPIO_RecvByte(I2C_GPIO_BUS *bus, int ack)
{
	int data = 0;

	I2C_SDA_BIT(bus->sda, BIT_H);

	for(int i = 0; i < 8; i++) {
		I2C_SCL_BIT(bus->scl, BIT_L);
		I2C_DELAY(bus->delay);
		I2C_SCL_BIT(bus->scl, BIT_H);
		I2C_DELAY(bus->delay);
		data <<= 1;
		data |= I2C_GET_BIT(bus->sda);
		I2C_DELAY(bus->delay);
	}

	/* ACK */
	I2C_SCL_BIT(bus->scl, BIT_L);
	I2C_DELAY(bus->delay);
	I2C_SDA_BIT(bus->sda, ack);
	I2C_DELAY(bus->delay);
	I2C_SCL_BIT(bus->scl, BIT_H);
	I2C_DELAY(bus->delay);
	I2C_DELAY(bus->delay);
	I2C_SCL_BIT(bus->scl, BIT_L);
	I2C_DELAY(bus->delay);

	return (uint8_t)data;
}

static int I2C_GPIO_Msg(I2C_GPIO_BUS *bus, I2C_MSG *msg, int seq __attribute__((unused)))
{
	int is_read = msg->flags & I2C_M_RD;
	uint32_t addr;
	int ret, i;

	addr = msg->addr << 1;
	if (is_read)
		addr |= 1;

	DBG("i2c msg : chip %02x, %s, len:%d, seq:%d\r\n",
			addr, is_read ? "rx" : "tx", msg->len, seq);

	I2C_GPIO_START(bus);

	ret = I2C_GPIO_TransByte(bus, (uint8_t)addr);
	if (ret)
		goto err;

	if (is_read) {
		for (i = 0; i < msg->len; i++)
			msg->buf[i] = I2C_GPIO_RecvByte(bus, i == (msg->len - 1));
	} else {
		for (i = 0; !ret && i < msg->len; i++)
			ret = I2C_GPIO_TransByte(bus, msg->buf[i]);
	}

err:
	return ret;

}

int I2C_GPIO_XFer(I2C_GPIO_BUS *bus, I2C_MSG *msg, int nmsgs)
{
	int ret, i;

	DBG("i2c xfer: msgs:%d\r\n", nmsgs);

	for (ret = 0, i = 0; !ret && i < nmsgs; i++)
		ret = I2C_GPIO_Msg(bus, &msg[i], i);

	I2C_GPIO_STOP(bus);

	return ret ? -I2C_NOK : I2C_OK;
}

int I2C_GPIO_Probe(I2C_GPIO_BUS *bus, uint8_t addr)
{
	int ret;

	I2C_GPIO_START(bus);
	ret = I2C_GPIO_TransByte(bus, (uint8_t)((addr << 1) | 0));
	I2C_GPIO_STOP(bus);

	return ret;
}
#endif
