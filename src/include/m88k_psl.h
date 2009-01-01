/*	$OpenBSD: psl.h,v 1.4 2005/09/25 22:41:12 miod Exp $ */
/*
 * Copyright (c) 1996 Nivas Madhur
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Nivas Madhur.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/*
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
 * Copyright (c) 1991 OMRON Corporation
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 */

#ifndef __M88K_PSL_H__
#define __M88K_PSL_H__

/*  #ifdef _KERNEL  */

/*
 * 881x0 control registers
 */

/*
 * processor identification register (PID)
 */
#define M88K_PID_ARN		0x0000ff00 /* architectural revision number */
#define	M88K_ARN_SHIFT		8
#define	M88K_ARN_88100		0x00
#define	M88K_ARN_88110		0x01
#define M88K_PID_VN		0x000000fe	/* version number */
#define	M88K_VN_SHIFT		1
#define M88K_PID_MC		0x00000001	/* 88100 master/checker mode */

/*
 * processor status register
 */

#define M88K_PSR_MODE	0x80000000	/* supervisor/user mode */
#define M88K_PSR_BO	0x40000000	/* byte-ordering 0:big 1:little */
#define M88K_PSR_SER	0x20000000	/* 88110 serial mode */
#define M88K_PSR_C	0x10000000	/* carry */
#define M88K_PSR_SGN	0x04000000	/* 88110 Signed Immediate mode */
#define M88K_PSR_SRM	0x02000000	/* 88110 Serialize Memory */
#define M88K_PSR_TRACE	0x00800000	/* 88110 hardware trace */
#define M88K_PSR_SFD	0x000003e0	/* SFU disable */
#define M88K_PSR_SFD2	0x00000010	/* 88110 SFU2 (Graphics) disable */
#define M88K_PSR_SFD1	0x00000008	/* SFU1 (FPU) disable */
#define M88K_PSR_MXM	0x00000004	/* misaligned access enable */
#define M88K_PSR_IND	0x00000002	/* interrupt disable */
#define M88K_PSR_SFRZ	0x00000001	/* shadow freeze */

#define M88K_FIP_V	0x00000002	/* valid */
#define M88K_FIP_E	0x00000001	/* exception */
#define M88K_FIP_ADDR	0xfffffffc	/* address mask */
#define M88K_NIP_V	0x00000002	/* valid */
#define M88K_NIP_E	0x00000001	/* exception */
#define M88K_NIP_ADDR	0xfffffffc	/* address mask */
#define M88K_XIP_V	0x00000002	/* valid */
#define M88K_XIP_E	0x00000001	/* exception */
#define M88K_XIP_ADDR	0xfffffffc	/* address mask */

/*  #endif  */

#endif /* __M88K_PSL_H__ */
