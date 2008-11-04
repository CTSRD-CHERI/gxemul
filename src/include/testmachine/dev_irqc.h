#ifndef	TESTMACHINE_IRQC_H
#define	TESTMACHINE_IRQC_H

/*
 *  Definitions used by the "irqc" device in GXemul.
 *
 *  $Id: dev_irqc.h,v 1.1 2007-05-26 03:47:34 debug Exp $
 *  This file is in the public domain.
 */

#define DEV_IRQC_ADDRESS 	0x0000000016000000
#define DEV_IRQC_LENGTH		0x12

#define DEV_IRQC_IRQ		0x0
#define DEV_IRQC_MASK		0x4
#define DEV_IRQC_UNMASK		0x8

#endif	/*  TESTMACHINE_IRQC_H  */
