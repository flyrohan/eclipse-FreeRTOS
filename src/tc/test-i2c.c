
#include <config.h>
#include <i2c.h>
#include <io.h>

#ifdef TS_I2C_ENABLED
#define DATA_OFFS	(0x10)

static uint8_t __iic_data[] = {
	0x21 + DATA_OFFS,
	0x22 + DATA_OFFS,
	0x23 + DATA_OFFS,
	0x24 + DATA_OFFS,
	0x25 + DATA_OFFS,
	0x26 + DATA_OFFS,
	0x27 + DATA_OFFS,
	0x28 + DATA_OFFS,
	0x29 + DATA_OFFS,
	0x2A + DATA_OFFS,
	0x2B + DATA_OFFS,
	0x2C + DATA_OFFS,
	0x2D + DATA_OFFS,
	0x2E + DATA_OFFS,
	0x2F + DATA_OFFS,
	0x30 + DATA_OFFS,
	0x31 + DATA_OFFS,
	0x32 + DATA_OFFS,
	0x33 + DATA_OFFS,
	0x34 + DATA_OFFS,
	0x35 + DATA_OFFS,
	0x36 + DATA_OFFS,
	0x37 + DATA_OFFS,
	0x38 + DATA_OFFS,
	0x39 + DATA_OFFS,
	0x3A + DATA_OFFS,
	0x3B + DATA_OFFS,
	0x3C + DATA_OFFS,
	0x3D + DATA_OFFS,
	0x3E + DATA_OFFS,
	0x3F + DATA_OFFS,
	0x40,
};

typedef enum  {
	IIC_READ,
	IIC_WRITE,
	IIC_PROBE,
} IIC_TEST_MD;

/*
 * #> iic p						: Probe i2cdevices
 * #> iic w <id> <addr> <len>	: Write data, addr is 16 bit, eeprom id is 0x51
 * #> iic r <id> <addr> <len>	: Read data , addr is 16 bit, eeprom id is 0x51
 */
static int do_iic(int argc, char * const argv[])
{
	IIC_TEST_MD mode;
	uint32_t chip, reg, ret;
	uint8_t addr[2];
	struct i2c_msg msg[2];
	uint8_t iic_buffer[ARRAY_SIZE(__iic_data) + 2];
	int ch = 0, len;

	switch (argv[1][0]) {
	case 'w':
	case 'r':
		if (argc < 5)
			return -1;
		chip = strtoul(argv[2], NULL, 16);
		reg = strtoul(argv[3], NULL, 16);
		len  = strtoul(argv[4], NULL, 10);
		addr[0] = (reg >> 8) & 0xFF;
		addr[1] = reg & 0xFF;
		mode = argv[1][0] == 'w' ? IIC_WRITE : IIC_READ;
		break;
	case 'p':
		mode = IIC_PROBE;
		break;
	default:
		return -1;
	}

	if (mode == IIC_PROBE) {
		for (int i = 0; i < 255; i++) {
			ret = i2c_probe(ch, (uint8_t)i);
			if (!ret)
				Printf("DETECT [0x%02x]\r\n", i);
		}
	} else if (mode == IIC_READ) {
		/*
		 * Read No-Stop
		 */

		/* write chip and addr */
	    msg[0].addr = chip;
		msg[0].flags = !I2C_M_RD;
		msg[0].buf = addr;
		msg[0].len = sizeof(addr);
		/* read data */
	    msg[1].addr = chip;
		msg[1].flags = I2C_M_RD;
		msg[1].buf = iic_buffer;
		msg[1].len = len;

		ret = i2c_xfer(ch, msg, 2);
		if (ret) {
			Printf("Fail: iic rx chip:0x%x [%d]\r\n", chip, len);
			return -1;
		}

		for (int i = 0; i < len; i++)
			Printf("[0x%02x] ", iic_buffer[i]);
		Printf("\r\n");

	} else if (mode == IIC_WRITE) {
		iic_buffer[0] = addr[0];
		iic_buffer[1] = addr[1];
		memcpy(&iic_buffer[2], __iic_data, len);

	    msg[0].addr = chip;
	    msg[0].flags = !I2C_M_RD;
		msg[0].buf = iic_buffer;
		msg[0].len = len + 2;

		ret = i2c_xfer(ch, msg, 1);
		if (ret) {
			Printf("Fail: iic tx chip:0x%x [%d]\r\n", chip, len);
			return -1;
		}

		for (int i = 0; i < len; i++)
			Printf("[0x%02x] ", __iic_data[i]);
		Printf("\r\n");
	}

	return 0;
}
CMD_DEFINE(iic, do_iic);
#endif
