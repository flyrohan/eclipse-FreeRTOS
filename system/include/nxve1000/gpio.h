#ifndef _GPIO_H_
#define _GPIO_H_

typedef enum {
	GPIO_ALTFUNC_0 = 0,
	GPIO_ALTFUNC_1 = 1,
} GPIO_ALTFUNC;

int GPIO_SetAlt(int pad, GPIO_ALTFUNC fn);
#endif
