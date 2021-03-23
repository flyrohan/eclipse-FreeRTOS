#include <string.h>
#include <cmsis_device.h>
#include <config.h>

#ifdef CLI_ENABLED

#ifndef CLI_BUFFER_SIZE
#define CLI_BUFFER_SIZE 128
#endif
#ifndef CLI_PROMPT
#define CLI_PROMPT       "NXVE1000$ "
#endif

#define ERASE_SEQ	"\b \b"	/* erase sequence */

static char *del_char(char *p, int *colp, int *np)
{
	if (*np == 0)
		return p;

	Puts(ERASE_SEQ);
	(*colp)--;
	(*np)--;

	return (--p);
}

static int CLI_ReadLine(const char *const prompt, char *buffer)
{
	char *p = buffer;
	char *p_buf = p;
	int n = 0; /* buffer index		*/
	int plen = 0; /* prompt length	*/
	int col; /* output column cnt	*/
	char c;

	p[0] = '\0';

	/* print prompt */
	if (prompt) {
		plen = (int)strlen(prompt);
		Puts(prompt);
	}
	col = plen;

	for (;;) {
		c = (char)Getc();
		/*
		 * Special character handling
		 */
		switch (c) {
		case '\r':			/* Enter */
		case '\n':
			*p = '\0';
			Puts("\r");
			Puts("\n");
			return p - p_buf;

		case '\0':			/* nul */
			continue;

		case 0x03:			/* ^C - break */
			p_buf[0] = '\0';	/* discard input */
			Puts("\r");
			Puts("\n");
			return -1;

		case 0x08:			/* ^H  - backspace */
		case 0x7F:			/* DEL - backspace */
			p = del_char(p, &col, &n);
			continue;

		default:
			/*
			 * Must be a normal character then
			 */
			if (n < CLI_BUFFER_SIZE - 2) {
				char buf[2];

				/*
				 * Echo input using Puts() to force an
				 * LCD flush if we are using an LCD
				 */
				++col;
				buf[0] = c;
				buf[1] = '\0';
				Puts(buf);
				*p++ = c;
				++n;
			}
		}
	}
	return 0;
}

void CLI_RunLoop(void)
{
	static char cli_buffer[CLI_BUFFER_SIZE + 1] = { 0, };

	for (;;) {
		int len = CLI_ReadLine(CLI_PROMPT, cli_buffer);
		if (len != -1)
			CMD_Run(cli_buffer);
	}
}
#endif
