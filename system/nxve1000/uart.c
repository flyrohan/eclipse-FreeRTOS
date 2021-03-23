#include <cmsis_device.h>
#include <io.h>
#include <config.h>

#ifdef UART_ENABLED

#ifndef UART_BAUDRATE
#define UART_BAUDRATE		115200
#endif

#define	UART_PHY_OFFSET		0x60000
#define	UART_REG_RBR		0x00
#define	UART_REG_IER		0x04
#define	UART_REG_FCR		0x08
#define	UART_REG_LCR		0x0C
#define	UART_REG_MCR		0x10
#define	UART_REG_LSR		0x14
#define	UART_REG_MSR		0x18
#define	UART_REG_SPR		0x1C
#define	UART_REG_THR		UART_REG_RBR
#define	UART_REG_IIR		UART_REG_FCR
#define	UART_REG_DLL		UART_REG_RBR
#define	UART_REG_DLM		UART_REG_IER

/*
 * These are the definitions for the FIFO Control Register
 */
#define UART_FCR_FIFO_EN		0x01 /* Fifo enable */
#define UART_FCR_CLEAR_RCVR		0x02 /* Clear the RCVR FIFO */
#define UART_FCR_CLEAR_XMIT		0x04 /* Clear the XMIT FIFO */
#define UART_FCR_DMA_SELECT		0x08 /* For DMA applications */
#define UART_FCR_TRIGGER_MASK	0xC0 /* Mask for the FIFO trigger range */
#define UART_FCR_TRIGGER_1		0x00 /* Mask for trigger set at 1 */
#define UART_FCR_TRIGGER_4		0x40 /* Mask for trigger set at 4 */
#define UART_FCR_TRIGGER_8		0x80 /* Mask for trigger set at 8 */
#define UART_FCR_TRIGGER_14		0xC0 /* Mask for trigger set at 14 */

#define UART_FCR_RXSR			0x02 /* Receiver soft reset */
#define UART_FCR_TXSR			0x04 /* Transmitter soft reset */


/* Clear & enable FIFOs */
#define UART_FCR_DEFVAL (UART_FCR_FIFO_EN | \
			UART_FCR_RXSR |	\
			UART_FCR_TXSR)

/*
 * These are the definitions for the Control Register
 */
#define UART_MCR_DTR			0x01		/* DTR   */
#define UART_MCR_RTS			0x02		/* RTS   */
#define UART_LCR_BKSE			0x80		/* Bank select enable */

/*
 * These are the definitions for the Line Status Register
 */
#define UART_LSR_DR				0x01		/* Data ready */
#define UART_LSR_OE				0x02		/* Overrun */
#define UART_LSR_PE				0x04		/* Parity error */
#define UART_LSR_FE				0x08		/* Framing error */
#define UART_LSR_BI				0x10		/* Break */
#define UART_LSR_THRE			0x20		/* Xmit holding register empty */
#define UART_LSR_TEMT			0x40		/* Xmitter empty */
#define UART_LSR_ERR			0x80		/* Error */

/* useful defaults for LCR */
#define UART_LCR_8N1			0x03

#define UART_LCRVAL 			UART_LCR_8N1						/* 8 data, 1 stop, no parity */
#define UART_MCRVAL 			(UART_MCR_DTR | UART_MCR_RTS)		/* RTS/DTR */

#define DIV_ROUND_CLOSEST(x, divisor)(          \
{                           \
    typeof(x) __x = x;              \
    typeof(divisor) __d = divisor;          \
    (((typeof(x))-1) > 0 ||             \
     ((typeof(divisor))-1) > 0 || (__x) > 0) ?  \
        (((__x) + ((__d) / 2)) / (__d)) :   \
        (((__x) - ((__d) / 2)) / (__d));    \
}                           \
)

static void *uart_handle = (void *)UART_PHY_BASE;

static void serial_out(int c, uint32_t offset)
{
	writel((uint32_t)c, (uint32_t)uart_handle + offset);
}

static uint32_t serial_in(uint32_t offset)
{
	return readl((uint32_t)uart_handle + offset);
}

static int serial_calc_divisor(unsigned int clock, unsigned int baudrate)
{
	const unsigned int mode_x_div = 16;

	return (int)DIV_ROUND_CLOSEST(clock, mode_x_div * baudrate);
}

int UART_Tstc(void)
{
	return (serial_in(UART_REG_LSR) & UART_LSR_DR) != 0;
}

int UART_ReadByte(void)
{
	while ((serial_in(UART_REG_LSR) & UART_LSR_DR) == 0) {
		;
	}
	return (int)serial_in(UART_REG_RBR);
}

void UART_WriteByte(const char c)
{
	while ((serial_in(UART_REG_LSR) & UART_LSR_THRE) == 0) {
		;
	}
	serial_out(c, UART_REG_THR);
}

static void UART_SetBaudRate(unsigned int clock, unsigned int baudrate)
{
	int baud_divisor;

	baud_divisor = serial_calc_divisor(clock, baudrate);

	serial_out(UART_LCR_BKSE | UART_LCRVAL, UART_REG_LCR);
	serial_out(baud_divisor & 0xff, UART_REG_DLL);
	serial_out((baud_divisor >> 8) & 0xff, UART_REG_DLM);
	serial_out(UART_LCRVAL, UART_REG_LCR);
}

void UART_Init(int ch, unsigned int clock)
{
	if (ch == 1)
		uart_handle = (void *)(UART_PHY_BASE + UART_PHY_OFFSET);

	while (!(serial_in(UART_REG_LSR) & UART_LSR_TEMT))
		;

	serial_out(0x0, UART_REG_IER);
	serial_out(0, UART_REG_MCR);
	serial_out(UART_FCR_DEFVAL, UART_REG_FCR);

	UART_SetBaudRate(clock, UART_BAUDRATE);
}

static Console_Op UART_Op = {
	.Getc = UART_ReadByte,
	.Putc = UART_WriteByte,
	.Tstc = UART_Tstc,
};

void UART_ConsoleRegister(int ch, unsigned int clock)
{
	UART_Init(ch, clock);
	Console_Register(&UART_Op);
}
#endif
