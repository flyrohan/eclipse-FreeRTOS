#include <stdarg.h>
#include <cmsis_device.h>
#include <config.h>

#ifdef CONSOLE_ENABLED

#define FMTFLG_ZERO     0x01    /*!< \brief Set, if zero padding required */
#define FMTFLG_SIGNED   0x02    /*!< \brief Set, if signed value */
#define FMTFLG_PLUS     0x04    /*!< \brief Set to force sign */
#define FMTFLG_MINUS    0x08    /*!< \brief Set to force left justification */
#define FMTFLG_CAPITAL  0x10    /*!< \brief Set for capital letter digits */

/*
 * Write a buffer to the debug output.
 */
static int PrintOutput(const char *data, int len)
{
	int ret = 0;

	while(len) {
		Putc(*data++);
		ret++;
		len--;
	}

	return ret;
}

/*
 * Print a numeric value on the debug output.
 */
static int PrintNumeric(unsigned long val, unsigned char radix, int width, unsigned char flags)
{
	unsigned char *digits;
	unsigned char raw[16];
	unsigned char rem, sign;
	int result, cnt;
	unsigned char large[] = "0123456789ABCDEF";
	unsigned char small[] = "0123456789abcdef";

	if(radix < 2)
	    radix = 10;
	result = 0;

	sign = 0;
	if(flags & FMTFLG_PLUS)
	    sign = '+';
    if(flags & FMTFLG_SIGNED)
	    if((long)val < 0) {
	        sign = '-';
	        val = -(unsigned long)val;
	    }

	/*
	 * Fill scratch buffer with raw digits.
	 */
	digits = (flags & FMTFLG_CAPITAL) ? large : small;
	cnt = 0;
	do {
	    rem = (unsigned char)(val % (unsigned long)radix);
	    val = val / (unsigned long)radix;
	    raw[cnt++] = digits[rem];
	} while(val);

    /*
     * Calculate the remaining width for padding
     */
    if(width > cnt) {
        if(sign)
            width--;
        width -= cnt;
    }
    else
        width = 0;

    /*
     * If not zero padded and not left justified,
     * we put enough spaces in front.
     */
    if((flags & (FMTFLG_ZERO | FMTFLG_MINUS)) == 0) {
        while(width) {
        	PrintOutput(" ", 1);
            result++;
            width--;
        }
    }
    if(sign) {
    	PrintOutput((const char*)(&sign), 1);
        result++;
    }
    if((flags & (FMTFLG_ZERO | FMTFLG_MINUS)) == FMTFLG_ZERO) {
        result += width;
        while(width) {
        	PrintOutput("0", 1);
            width--;
        }
    }
    while(cnt-- > 0) {
    	PrintOutput((const char*)(&raw[cnt]), 1);
        result++;
    }
    while(width) {
    	PrintOutput(" ", 1);
        result++;
        width--;
    }

    return result;
}

/*
 * Print parameters using a format string.
 */
int Printf(const char *fmt, ...)
{
    va_list ap;
    unsigned char *s, *cp;
    int width, result, len;
    unsigned long val;
    unsigned char radix, flags;
    char isize;

    va_start(ap, fmt);

    for(result = 0; *fmt; ++fmt) {
        if(*fmt != '%') {
            if(PrintOutput(fmt, 1) < 0)
                return -1;
            result++;
            continue;
        }

        flags = 0;
        while(1) {
            if(*++fmt == '-')
                flags |= FMTFLG_MINUS;
            else if(*fmt == '+')
                flags |= FMTFLG_PLUS;
            else if(*fmt == '0')
                flags |= FMTFLG_ZERO;
            else
                break;
        }

        if(*fmt == '*') {
            ++fmt;
            width = va_arg(ap, int);
            if(width < 0) {
                width = -width;
                flags |= FMTFLG_MINUS;
            }
        }
        else {
            width = 0;
            while(*fmt >= '0' && *fmt <= '9')
                width = width * 10 + (*fmt++ - '0');
        }

        isize = 's';
        if(*fmt == 'l') {
            isize = *fmt;
            ++fmt;
        }

        if(*fmt == 'c' || *fmt == 's') {
            s = 0;
            if(*fmt == 's') {
                if((s = ((unsigned char*)va_arg(ap, char *))) == 0) {
                    s = (unsigned char*)"(NULL)";
                    len = 6;
                }
                else {
                    len = 0;
                    cp = s;
                    while(*cp++)
                        len++;
                }
            }
            else
                len = 1;
            if((flags & FMTFLG_MINUS) == 0)
                while(width > len) {
                	PrintOutput(" ", 1);
                    result++;
                    width--;
                }
            if(s) {
            	PrintOutput((const char*)s, len);
                result += len;
            }
            else {
            	unsigned char ch = (unsigned char)va_arg(ap, int);
            	PrintOutput((const char*)&ch, 1);
                result++;
            }
            while(width > len) {
            	PrintOutput(" ", 1);
                result++;
                width--;
            }
            continue;
        }

        radix = 10;
        if(*fmt == 'o')
            radix = 8;
        else if(*fmt == 'X' || *fmt == 'x') {
            if(*fmt == 'X')
                flags |= FMTFLG_CAPITAL;
            radix = 16;
        }
        else if(*fmt == 'd')
            flags |= FMTFLG_SIGNED;
        else if(*fmt != 'u') {
            if(*fmt != '%') {
            	PrintOutput("%", 1);
                result++;
            }
            if(*fmt) {
            	PrintOutput(fmt, 1);
                result++;
            }
            else
                fmt--;
            continue;
        }

        if(isize == 'l') {
            if(flags & FMTFLG_SIGNED)
                val = (unsigned long)va_arg(ap, long);
            else
                val = va_arg(ap, unsigned long);
        }
        else {
            if(flags & FMTFLG_SIGNED)
                val = (unsigned long)va_arg(ap, int);
            else
                val = (unsigned long)va_arg(ap, unsigned int);
        }
        result += PrintNumeric(val, radix, width, flags);
    }

    va_end(ap);

    return result;
}

static Console_Op *console_op;

void Putc(const char c)
{
	if (console_op->Putc)
		console_op->Putc(c);
}

void Puts(const char *s)
{
	while (*s) {
		Putc(*s++);
	}
}

int Getc(void)
{
	int c = -1;

	if (console_op->Getc)
		c = console_op->Getc();

	return c;
}

int Tstc(void)
{
	int ret = 0;

	if (console_op->Tstc)
		ret = console_op->Tstc();

	return ret;
}

int isCtrlc(int c)
{
	return (c == 0x03) ? 1 : 0;
}

void Console_Register(Console_Op *console)
{
	console_op = console;
}
#else
void Putc(const char c __attribute__((unused))) { }
void Puts(const char *s __attribute__((unused))) { }
int Getc(void) { return 0; }
int isCtrlc(int c __attribute__((unused))) { return 1; }
int Tstc(void) { return 0; }
int Printf(const char *fmt __attribute__((unused)), ...) { return 0; }
void Console_Register(Console_Op *console __attribute__((unused))) { }
#endif
