#ifndef IRNATIVE_H
#define	IRNATIVE_H

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
 *  $Id: IRNative.h,v 1.1 2008/03/12 11:45:41 debug Exp $
 */

#include "misc.h"

class IRInstruction;


/**
 * \brief A baseclass for native code generators.
 *
 * A native code generator is fed IRInstruction elements, and as output it
 * should be able to generate native machine code in memory which
 * corresponds to the IR code.
 */
class IRNative
{
public:
	enum Opcode
	{
		OpcodeAdd2,
		OpcodeAdd3,
		OpcodeMove,
		OpcodeStore
	};

public:
	/**
	 * \brief Constructs an %IRNative instance.
	 */
	IRNative()
	{
	}

	virtual ~IRNative()
	{
	}

	/**
	 * \brief Adds an instruction to the native code generator.
	 *
	 * \param opcode The main opcode.
	 * \param widthInBits Width in bits (for some opcodes).
	 * \param arg1 Optional argument 1.
	 * \param arg2 Optional argument 2.
	 * \param arg3 Optional argument 3.
	 */
	virtual void Add(enum Opcode opcode, int widthInBits = 0,
		size_t arg1 = 0, size_t arg2 = 0, size_t arg3 = 0) = 0;
	
	/**
	 * \brief Calculates the size of the code which is to be generated.
	 *
	 * \return The size in bytes.
	 */
	virtual size_t GetSize() const = 0;
	
	/**
	 * \brief Generates code into a buffer.
	 *
	 * Note: For some archs, after the code has been generated, the dst
	 * buffer is invalidated in the instruction cache, so that it can
	 * be executed. (Not needed on all architectures, e.g. amd64.)
	 *
	 * \param maxSize The size of the buffer in bytes.
	 * \param dst The pointer to the buffer.
	 */
	virtual void Generate(size_t maxSize, uint8_t * dst) const = 0;
};


#endif	// IRNATIVE_H
