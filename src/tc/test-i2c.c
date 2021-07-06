
#include <config.h>
#include <string.h>
#include <stdlib.h>
#include <rtos.h>
#include <i2c.h>
#include <io.h>

#ifdef TESTSUITE_I2C_ENABLED

static uint8_t __iic_data[] = {
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
	0x30, 0x31, 0x33, 0x34, 0x34, 0x35, 0x36, 0x37,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
};

typedef enum  {
	IIC_READ,
	IIC_WRITE,
	IIC_PROBE,
	IIC_AGING,
} IIC_TEST_MODE;

typedef struct __iic_argument {
	int bus;
	uint32_t chip;
	uint32_t addr;
	int alen;
	int len;
	int repeat_start;
	IIC_TEST_MODE mode;
	void *hnd;
} iic_argument;

static iic_argument iic_argc;

#define GPIO_I2C_SDA_0		(1)
#define GPIO_I2C_SCL_0		(2)
#define GPIO_I2C_SDA_1		(3)
#define GPIO_I2C_SCL_1		(4)

#define IIC_ALT_FN(iic) do { \
	} while (0)

#define IIC_ALT_IO(iic) do { \
	} while (0)

#define IIC_ADDR(_v, _a) do { \
	_a[0] = (_v >> 8) & 0xFF; \
	_a[1] = _v & 0xFF; \
	} while (0)

#define TSTC_CTRLC() do { \
    if (Tstc() && isCtrlc(Getc()))    \
        return 0;   \
    } while (0)

#define ERR(MSG...)		Printf(MSG);
#define MSG(MSG...)		Printf(MSG);

#ifdef RTOS_ENABLED
#ifdef CMSIS_ENABLED
#define Delay(_t)		osDelay(_t)
#else
#define Delay(_t)		vTaskDelay((TickType_t)_t)
#endif
#else
#define Delay(_t)		SysTime_Delay(_t)
#endif

/*
 * IIC Firmware Module : I2C_ENABLED
 */
#ifdef I2C_ENABLED
static int iic_fn_probe(iic_argument *iic, uint8_t chip)
{
	IIC_ALT_FN(iic);
	return I2C_Probe(iic->bus, chip);
}

static int iic_fn_read(iic_argument *iic, uint8_t *buffer)
{
	I2C_MSG MSG[2];
	uint8_t addr[2];

	IIC_ALT_FN(iic);
	IIC_ADDR(iic->addr, addr);

	if (iic->repeat_start) {
		/* write chip and addr */
		MSG[0].addr = iic->chip;
		MSG[0].flags = !I2C_M_RD;
		MSG[0].buf = addr;
		MSG[0].len = iic->alen;
		/* read data */
		MSG[1].addr = iic->chip;
		MSG[1].flags = I2C_M_RD;
		MSG[1].buf = buffer;
		MSG[1].len = iic->len;

		if (I2C_XFer(iic->bus, MSG, 2)) {
			ERR("Fail: iic read chip:0x%x [%d]\r\n", iic->chip, iic->len);
			return -1;
		}
		return 0;
	} else {
		MSG[0].addr = iic->chip;
		MSG[0].flags = !I2C_M_RD;
		MSG[0].buf = addr;
		MSG[0].len = iic->alen;

		if (I2C_XFer(iic->bus, MSG, 1)) {
			ERR("Fail: iic read transfer chip:0x%x [%d]\r\n", iic->chip, iic->len);
			return -1;
		}

		MSG[0].addr = iic->chip;
		MSG[0].flags = I2C_M_RD;
		MSG[0].buf = buffer;
		MSG[0].len = iic->len;

		if (I2C_XFer(iic->bus, MSG, 1)) {
			ERR("Fail: iic read receive chip:0x%x [%d]\r\n", iic->chip, iic->len);
			return -1;
		}
		return 0;
	}

	return -1;
}

static int iic_fn_write(iic_argument *iic, uint8_t *buffer, uint8_t *data)
{
	I2C_MSG MSG[2];
	uint8_t addr[2];

	IIC_ALT_FN(iic);
	IIC_ADDR(iic->addr, addr);

	for (int i = 0; i < iic->len; i++)
		data[i] = (uint8_t)(data[i] + 0x1);

	buffer[0] = addr[0];
	buffer[1] = addr[1];
	memcpy(&buffer[2], data, (size_t)iic->len);

    MSG[0].addr = iic->chip;
    MSG[0].flags = !I2C_M_RD;
	MSG[0].buf = buffer;
	MSG[0].len = iic->len + 2;

	if (I2C_XFer(iic->bus, MSG, 1)) {
		ERR("Fail: iic write chip:0x%x [%d]\r\n", iic->chip, iic->len);
		return -1;
	}

	return 0;
}
#endif

/*
 * IIC GPIO Module
 */
#ifdef I2C_GPIO_ENABLED
static int iic_io_probe(iic_argument *iic, uint8_t chip)
{
	I2C_GPIO_BUS bus = {
		.sda = iic->bus == 0 ? GPIO_I2C_SDA_0 : GPIO_I2C_SCL_0,
		.scl = iic->bus == 0 ? GPIO_I2C_SDA_1 : GPIO_I2C_SCL_1,
		.repeat_start = iic->repeat_start,
		.delay = 0,
	};

	IIC_ALT_IO(iic);

	return I2C_GPIO_Probe(&bus, chip);
}

static int iic_io_read(iic_argument *iic, uint8_t *buffer)
{
	I2C_MSG MSG[2];
	I2C_GPIO_BUS bus = {
		.sda = iic->bus == 0 ? GPIO_I2C_SDA_0 : GPIO_I2C_SCL_0,
		.scl = iic->bus == 0 ? GPIO_I2C_SDA_1 : GPIO_I2C_SCL_1,
		.repeat_start = iic->repeat_start,
		.delay = 0,
	};
	uint8_t addr[2];

	IIC_ALT_IO(iic);
	IIC_ADDR(iic->addr, addr);

	if (iic->repeat_start) {
		/* write chip and addr */
		MSG[0].addr = iic->chip;
		MSG[0].flags = !I2C_M_RD;
		MSG[0].buf = addr;
		MSG[0].len = iic->alen;
		/* read data */
		MSG[1].addr = iic->chip;
		MSG[1].flags = I2C_M_RD;
		MSG[1].buf = buffer;
		MSG[1].len = iic->len;

		if (I2C_GPIO_XFer(&bus, MSG, 2)) {
			ERR("Fail: iic io read chip:0x%x [%d]\r\n", iic->chip, iic->len);
			return -1;
		}
		return 0;
	} else {
		MSG[0].addr = iic->chip;
		MSG[0].flags = !I2C_M_RD;
		MSG[0].buf = addr;
		MSG[0].len = iic->alen;

		if (I2C_GPIO_XFer(&bus, MSG, 1)) {
			ERR("Fail: iic io read transfer chip:0x%x [%d]\r\n", iic->chip, iic->len);
			return -1;
		}

		MSG[0].addr = iic->chip;
		MSG[0].flags = I2C_M_RD;
		MSG[0].buf = buffer;
		MSG[0].len = iic->len;

		if (I2C_GPIO_XFer(&bus, MSG, 1)) {
			ERR("Fail: iic io read receive chip:0x%x [%d]\r\n", iic->chip, iic->len);
			return -1;
		}
		return 0;
	}
}

static int iic_io_write(iic_argument *iic, uint8_t *buffer, uint8_t *data)
{
	I2C_MSG MSG[2];
	I2C_GPIO_BUS bus = {
		.sda = iic->bus == 0 ? GPIO_I2C_SDA_0 : GPIO_I2C_SCL_0,
		.scl = iic->bus == 0 ? GPIO_I2C_SDA_1 : GPIO_I2C_SCL_1,
		.repeat_start = iic->repeat_start,
		.delay = 0,
	};
	uint8_t addr[2];

	for (int i = 0; i < iic->len; i++)
		data[i] = (uint8_t)(data[i] + 0x2);

	IIC_ALT_IO(iic);
	IIC_ADDR(iic->addr, addr);

	buffer[0] = addr[0];
	buffer[1] = addr[1];
	memcpy(&buffer[2], data, (size_t)iic->len);

    MSG[0].addr = iic->chip;
    MSG[0].flags = !I2C_M_RD;
	MSG[0].buf = buffer;
	MSG[0].len = iic->len + 2;

	if (I2C_GPIO_XFer(&bus, MSG, 1)) {
		ERR("Fail: iic write chip:0x%x [%d]\r\n", iic->chip, iic->len);
		return -1;
	}
	return 0;
}
#endif

/*
 * #> iic <module> p <bus> 						 	: i2c probe devices
 * #> iic <module> w <bus> <chip id> <addr> <len>   : i2c write data, addr is 16 bit, eeprom id is 0x51
 * #> iic <module> r <bus> <chip id> <addr> <len> n : i2c write data, addr is 16 bit, eeprom id is 0x51, n: no stop/repeat start
 * #> iic <module> a <bus> 						 	: i2c aging test (write and verify)
 */
static iic_argument *do_parse(int argc, char * const argv[], const char *module)
{
	iic_argument *iic = &iic_argc;

	iic->bus = 0;
	iic->repeat_start = 0;
	iic->alen = 2;

	switch (argv[2][0]) {
	case 'a':
	case 'w':
	case 'r':
		if (argc < 6)
			return NULL;
		iic->bus  = (int32_t) strtoul(argv[3], NULL, 16);
		iic->chip = (uint32_t)strtoul(argv[4], NULL, 16);
		iic->addr = (uint32_t)strtoul(argv[5], NULL, 16);
		iic->len  = (int32_t)strtoul(argv[6], NULL, 10);
		iic->mode = argv[2][0] == 'w' ? IIC_WRITE : IIC_READ;
		if (argc > 7) {
			if (argv[7][0] == 'n')
				iic->repeat_start = 1;
		}
		if (argv[2][0] == 'a')
			iic->mode = IIC_AGING;
		break;
	case 'p':
		iic->mode = IIC_PROBE;
		iic->bus  = (int32_t)strtoul(argv[3], NULL, 16);
		break;
	default:
		return NULL;
	}

	if (iic->len > (int)sizeof(__iic_data)) {
		ERR("Fail: length is over buffer size:%d\r\n", sizeof(__iic_data));
		return NULL;
	}

	MSG("module : %s\r\n", module);
	MSG("mode   : %d\r\n", iic->mode);
	MSG("STOP   : %s\r\n", iic->repeat_start ? "NOSTOP" : "STOP/START");
	MSG("bus    : %d\r\n", iic->bus);
	MSG("chip   : 0x%x\r\n", iic->chip);
	MSG("addr   : 0x%x\r\n", iic->addr);
	MSG("len    : %d\r\n", iic->len);

	return iic;
}

typedef struct __testcase_t {
	const char *module;
	int (*probe)  (iic_argument *iic, uint8_t chip);
	int (*read)  (iic_argument *iic, uint8_t *buffer);
	int (*write) (iic_argument *iic, uint8_t *buffer, uint8_t *data);
} testcase_t;

static testcase_t iic_testcase[] = {
#ifdef I2C_ENABLED
	{
		.module = "fn",
		.probe = iic_fn_probe,
		.read = iic_fn_read,
		.write = iic_fn_write,
	},
#endif
#ifdef I2C_GPIO_ENABLED
	{
		.module = "io",
		.probe = iic_io_probe,
		.read = iic_io_read,
		.write = iic_io_write,
	},
#endif
};

/*
 * #> iic <module> p <bus> 						 	: i2c probe devices
 * #> iic <module> w <bus> <chip id> <addr> <len>   : i2c write data, addr is 16 bit, eeprom id is 0x51
 * #> iic <module> r <bus> <chip id> <addr> <len> n : i2c write data, addr is 16 bit, eeprom id is 0x51, n: no stop/repeat start
 * #> iic <module> a <bus> 						 	: i2c aging test (write and verify), CTRL + C : Exit
 *
 * EX: iic firmware module
 * iic fn p
 * iic fn w 0 51 0 8
 * iic fn r 0 51 0 8
 * iic fn r 0 51 0 8 n
 * iic fn a 0 51 0 8 <n: option>
 *
 * EX: iic gpio module
 * iic io p
 * iic io w 0 51 0 8
 * iic io r 0 51 0 8
 * iic io r 0 51 0 8 n
 * iic io a 0 51 0 8 <n: option>
 *
 */
static int do_iic_tc(int argc, char * const argv[])
{
	uint8_t buffer[ARRAY_SIZE(__iic_data) + 2];
	testcase_t *p;
	int i, ret = -1;

	for (i = 0; i < ARRAY_SIZE(iic_testcase); i++) {
		p = &iic_testcase[i];

		if (!strcmp(argv[1], p->module)) {
			iic_argument *iic = do_parse(argc, argv, p->module);
			if (!iic)
				return -1;

			if (iic->mode == IIC_PROBE) {
				for (i = 0; i < 255; i++) {
					ret = p->probe(iic, (uint8_t)i);
					if (!ret)
						MSG("iic detect chip: 0x%02x\r\n", i);
				}
			} else if (iic->mode == IIC_READ) {
				ret = p->read(iic, buffer);
				if (ret)
					return ret;

				for (i = 0; i < iic->len; i++) {
					if (i != 0 && !(i%8))
						MSG("\r\n");
					MSG("[0x%02x] ", buffer[i]);
				}
				MSG("\r\n");
			} else if (iic->mode == IIC_WRITE) {
				ret = p->write(iic, buffer, __iic_data);
				if (ret)
					return ret;

				for (i = 0; i < iic->len; i++) {
					if (i != 0 && !(i%8))
						MSG("\r\n");
					MSG("[0x%02x] ", __iic_data[i]);
				}
				MSG("\r\n");
			} else if (iic->mode == IIC_AGING) {
				uint32_t count = 0;
				while (1) {
					TSTC_CTRLC();

					ret = p->write(iic, buffer, __iic_data);
					if (ret)
						return ret;

					Delay(10);

					ret = p->read(iic, buffer);
					if (ret)
						return ret;

					for (i = 0; i < iic->len; i++) {
						if (__iic_data[i] != buffer[i]) {
							ERR("Fail:(%d) [0x%02x] [0x%02x]\r\n", i, __iic_data[i], buffer[i]);
							return -1;
						}
					}
					MSG("\r OK, count:%d", count++);
				}
			}
		}
	}
	return ret;
}
CMD_DEFINE(iic, do_iic_tc);
#endif
