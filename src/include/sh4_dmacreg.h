#ifndef	SH4_DMACREG_H
#define	SH4_DMACREG_H

/*
 *  Copyright (C) 2006-2008  Anders Gavare.  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright  
 *     notice, this list of conditions and the following disclaimer in the 
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE   
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 *
 *
 *  $Id: sh4_dmacreg.h,v 1.2.2.1 2008-01-18 19:12:32 debug Exp $
 *
 *  SH4 DMAC (DMA Controller) registers, as listed in the SH-7750 manual.
 */

#define	N_SH4_DMA_CHANNELS	4

#define	SH4_SAR0	0xffa00000	/*  Source Address Register  */
#define	SH4_DAR0	0xffa00004	/*  Destination Address Register  */
#define	SH4_DMATCR0	0xffa00008	/*  Transfer Count Register  */
#define	SH4_CHCR0	0xffa0000c	/*  Channel Control Register  */

#define	SH4_SAR1	0xffa00010
#define	SH4_DAR1	0xffa00014
#define	SH4_DMATCR1	0xffa00018
#define	SH4_CHCR1	0xffa0001c

#define	SH4_SAR2	0xffa00020
#define	SH4_DAR2	0xffa00024
#define	SH4_DMATCR2	0xffa00028
#define	SH4_CHCR2	0xffa0002c

#define	SH4_SAR3	0xffa00030
#define	SH4_DAR3	0xffa00034
#define	SH4_DMATCR3	0xffa00038
#define	SH4_CHCR3	0xffa0003c


/*
 *  Channel Control Register bit definitions:
 */

/*  Source Address Space Attribute Specification:  */
/*  (Only valid for PCMCIA access, in areas 5 and 6.)  */
#define	CHCR_SSA_MASK	0xe0000000
#define	    CHCR_SSA_RESERVED				(0 << 29)
#define	    CHCR_SSA_DYNAMIC_BUS_SIZING			(1 << 29)
#define	    CHCR_SSA_8BIT_IO_SPACE			(2 << 29)
#define	    CHCR_SSA_16BIT_IO_SPACE			(3 << 29)
#define	    CHCR_SSA_8BIT_COMMON_MEMORY_SPACE		(4 << 29)
#define	    CHCR_SSA_16BIT_COMMON_MEMORY_SPACE		(5 << 29)
#define	    CHCR_SSA_8BIT_ATTRIBUTE_MEMORY_SPACE	(6 << 29)
#define	    CHCR_SSA_16BIT_ATTRIBUTE_MEMORY_SPACE	(7 << 29)
#define	CHCR_STC	0x10000000
#define	CHCR_DSA_MASK	0x0e000000
#define	CHCR_DTC	0x01000000
#define	CHCR_DS		0x00080000
#define	CHCR_RL		0x00040000
#define	CHCR_AM		0x00020000
#define	CHCR_AL		0x00010000
#define	CHCR_DM		0x0000c000
#define	CHCR_SM		0x00003000
#define	CHCR_RS		0x00000f00
#define	CHCR_TM		0x00000080
#define	CHCR_TS		0x00000070
#define	CHCR_IE		0x00000004
#define	CHCR_TE		0x00000002
#define	CHCR_TD		0x00000001

#define	SH4_DMAOR	0xffa00040	/*  DMA operation register  */

#endif	/*  SH4_DMACREG_H  */
