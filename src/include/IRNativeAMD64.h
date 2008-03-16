#ifndef IRNATIVEAMD64_H
#define	IRNATIVEAMD64_H

/*
 *  Copyright (C) 2008  Anders Gavare.  All rights reserved.
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
 *  $Id: IRNativeAMD64.h,v 1.1 2008/03/12 11:45:41 debug Exp $
 */

#include "misc.h"

#include "IRNative.h"
#include "UnitTest.h"


/**
 * \brief An AMD64 IRNative code generator.
 */
class IRNativeAMD64
	: public IRNative
	, public UnitTestable
{
public:
	/**
	 * \brief Constructs an %IRNativeAMD64 instance.
	 */
	IRNativeAMD64();

	virtual ~IRNativeAMD64() { }

	// These are described in IRNative.h:
	virtual void Add(enum Opcode opcode, int widthInBits = 0,
		size_t arg1 = 0, size_t arg2 = 0, size_t arg3 = 0);
	virtual size_t GetSize() const;
	virtual void Generate(size_t maxSize, uint8_t * dst) const;


	/********************************************************************/

	static void RunUnitTests(int& nSucceeded, int& nFailures);

private:
	// TODO: something like  list<amd64instructions> m_instructions;
};


#endif	// IRNATIVEAMD64_H
