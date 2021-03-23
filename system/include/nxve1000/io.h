#ifndef _IO_H_
#define _IO_H_

#define BIT(nr)         				(1UL << (nr))
#define setbit(_a, _bit)				((_a) |= (1 << _bit))
#define clrbit(_a, _bit)				((_a) &= ~(1 << _bit))

#define readl(_a)						(*(volatile uint32_t *)(_a))
#define writel(_v, _a)					(*(volatile uint32_t *)(_a) = (_v))

#define _mask(_v, _m)					 (_v & ~(uint32_t)_m)
#define _maskbits(_p, _w)               (((1 << _w) - 1) << _p)
#define _getsftbits(_d, _p, _w)         (((_d) & _maskbits(_p, _w)) >> _p)
#define _clrsetbits(_d, _p, _w, _v)		(((_d) & (uint32_t)~_maskbits(_p, _w)) | \
										(uint32_t)(((_v) << _p) & _maskbits(_p, _w)))
#define _writebits(_addr, _p, _w, _v)	writel(_clrsetbits(readl(_addr), _p, _w, _v), _addr)
#define _readbits(_addr, _p, _w)		_getsftbits(readl(_addr), _p, _w)

#define ARRAY_SIZE(x)	(int)(sizeof(x) / sizeof((x)[0]))

#endif

