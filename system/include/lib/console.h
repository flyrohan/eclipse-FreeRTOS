#ifndef _CONSOLE_H_
#define _CONSOLE_H_

typedef struct {
	void	(*Putc)(const char c);
	int 	(*Getc)(void);
	int 	(*Tstc)(void);
} Console_Op;

void Console_Register(Console_Op *console);
void Putc(const char c);
void Puts(const char *s);
int Getc(void);
int isCtrlc(int c);
int Tstc(void);
int Printf(const char *fmt, ...);

#endif
