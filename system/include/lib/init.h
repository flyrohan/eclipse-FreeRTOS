#ifndef _INIT_H_
#define _INIT_H_

typedef void (*initcall_t)(void);

#if !defined(__ARMCC_VERSION)
#define SYSTEM_INIT(fn) \
	void fn(void); \
	initcall_t _init_##fn __attribute__ ((section(".init_array."#fn""))) = fn
#else
#define SYSTEM_INIT(fn) \
    void fn(void) __attribute__((constructor(0)));
#endif

#endif
