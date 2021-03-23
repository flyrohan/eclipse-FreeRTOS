#include <cmsis_device.h>
#include <io.h>
#include <config.h>

#ifdef PLL_ENABLED

/* PLL Register */
#define PLLCTRL					(PLL_PHY_BASE + 0x0) 	/* OSCCLK_MUXSEL, DIRTYFLAG, UPDATECONFIG_DIRECTLY */
#define	PLLCNT3					(PLL_PHY_BASE + 0x1C) 	/* RUN3_MAXCOUNT */
#define	PLLCFG0					(PLL_PHY_BASE + 0x20) 	/* LINK_BYPASS, LINK_RESETB */
#define	PLLCFG1					(PLL_PHY_BASE + 0x30) 	/* LINK_P, LINK_M */
#define PLLCFG2					(PLL_PHY_BASE + 0x40) 	/* LINK_S, LINK_ICP*/
#define PLLLOCKINT0				(PLL_PHY_BASE + 0x60) 	/* RUNDONE_INTPEND_CLR */
#define ALIVEFSMOSCSOURCE		(ALIVE_FSM_PHY_BASE + 0x178) /* ALIVETIMERSOURCE */

/* CMU Offset */
#define CLKSEL_BP					0
#define CLKSEL_BW					1

/* PLL Offset */
#define RUN_CHANGE_BP				0
#define RUN_CHANGE_BW				1
#define DIRTYFLAG_BP				1
#define DIRTYFLAG_BW				1
#define OSCCLK_MUXSEL_BP			3
#define OSCCLK_MUXSEL_BW			1
#define UPDATECONFIG_DIRECTLY_BP	15
#define UPDATECONFIG_DIRECTLY_BW	1
#define LINK_RESETB_BP				0
#define LINK_RESETB_BW				1
#define LINK_BYPASS_BP				1
#define LINK_BYPASS_BW				1
#define LINK_P_BP					0
#define LINK_P_BW					2
#define LINK_M_M1_BP				16
#define LINK_M_M1_BW				7
#define LINK_M_M2_BP				24
#define LINK_M_M2_BW				8
#define LINK_S_BP					0
#define LINK_S_BW					2
#define LINK_ICP_BP					16
#define LINK_ICP_BW					2
#define RUNDONE_INTENB_BP			5
#define RUNDONE_INTENB_BW			1
#define RUNDONE_INTPEND_CLR_BP		4
#define RUNDONE_INTPEND_CLR_BW		1
#define RUN3_MAXCOUNT_BP			0
#define RUN3_MAXCOUNT_BW			32

/* ALIVE Offset */
#define ALIVETIMERSOURCE_BP			8
#define ALIVETIMERSOURCE_BW			1
#define SET_ALIVETIMERSOURCE_BP 	12
#define SET_ALIVETIMERSOURCE_BW		1

#define _WRITE_FIELD(_addr, _field, _val)	do { \
		_writebits(_addr, _field##_BP, _field##_BW, _val);	\
        } while (0)

#define _READ_FIELD(_addr, _field)		_readbits(_addr, _field##_BP, _field##_BW)

typedef struct {
	int p;
	int m1;
	int m2;
	int s;
	int bypass;
	int icp;
	uint32_t hz;
} PLLPMS;

typedef enum {
	PLL_HZ_24MHZ = 0,
	PLL_HZ_48MHZ = 1,
	PLL_HZ_96MHZ = 2,
	PLL_HZ_NONE = 3,
} PLL_HZ_TYPE;

static PLLPMS pms_table[] = {
	[PLL_HZ_24MHZ] = { .bypass = 0, .icp = 0, .p = 0, .m1 = 0x1D, .m2 = 0x65, .s = 2, .hz = 24000000},
	[PLL_HZ_48MHZ] = { .bypass = 0, .icp = 0, .p = 0, .m1 = 0x1D, .m2 = 0x65, .s = 1, .hz = 48000000 },
	[PLL_HZ_96MHZ] = { .bypass = 0, .icp = 0, .p = 0, .m1 = 0x1D, .m2 = 0x65, .s = 0, .hz = 96000000 },
};

static void PLL_Change_PMS(PLLPMS *pms)
{
	 __disable_irq();

	_WRITE_FIELD(PLLCFG0, LINK_BYPASS, pms->bypass);
	_WRITE_FIELD(CMU_PHY_BASE, CLKSEL, 0x0);
	_WRITE_FIELD(PLLCTRL, OSCCLK_MUXSEL, 0x0);
	_WRITE_FIELD(ALIVEFSMOSCSOURCE, ALIVETIMERSOURCE, 0x1);
	_WRITE_FIELD(ALIVEFSMOSCSOURCE, SET_ALIVETIMERSOURCE, 0x1);
	_WRITE_FIELD(PLLCFG1, LINK_P, pms->p);
	_WRITE_FIELD(PLLCFG1, LINK_M_M1, pms->m1);
	_WRITE_FIELD(PLLCFG1, LINK_M_M2, pms->m2);
	_WRITE_FIELD(PLLCFG2, LINK_S, pms->s);
	_WRITE_FIELD(PLLCFG2, LINK_ICP, pms->icp);
	_WRITE_FIELD(PLLCFG0, LINK_BYPASS, pms->bypass);
	_WRITE_FIELD(PLLCFG0, LINK_RESETB, 0);
	_WRITE_FIELD(PLLCTRL, DIRTYFLAG, 0x1);
	_WRITE_FIELD(PLLCTRL, UPDATECONFIG_DIRECTLY, 0x1);
	_WRITE_FIELD(PLLCFG0, LINK_RESETB, 1);
	_WRITE_FIELD(PLLCTRL, DIRTYFLAG, 0x1);
	_WRITE_FIELD(PLLCTRL, UPDATECONFIG_DIRECTLY, 0x1);
	_WRITE_FIELD(PLLCTRL, RUN_CHANGE, 0x1);
	_WRITE_FIELD(PLLLOCKINT0, RUNDONE_INTENB, 0x1);

	do {
		if (_READ_FIELD(PLLLOCKINT0, RUNDONE_INTPEND_CLR))
			break;
	} while (1);

	_WRITE_FIELD(PLLLOCKINT0, RUNDONE_INTPEND_CLR, 0x1);
	_WRITE_FIELD(PLLLOCKINT0, RUNDONE_INTENB, 0x0);
	_WRITE_FIELD(PLLCTRL, OSCCLK_MUXSEL, 0x1);
	_WRITE_FIELD(CMU_PHY_BASE, CLKSEL, 0x1);

	 __enable_irq();
}

uint32_t PLL_SetFrequency(uint32_t frequency)
{
	for (int i = 0; i < PLL_HZ_NONE; i++) {
		if (pms_table[i].hz == frequency) {
			PLL_Change_PMS(&pms_table[i]);
			return pms_table[i].hz;
		}
	}

	return 0;
}
#endif
