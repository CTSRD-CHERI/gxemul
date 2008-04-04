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
 */

#include "IRNativeAMD64.h"

#ifdef NATIVE_ABI_AMD64


IRNativeAMD64::IRNativeAMD64()
	: IRNative()
{
}


void IRNativeAMD64::Add(enum Opcode opcode, int widthInBits,
	size_t arg1, size_t arg2, size_t arg3)
{
	switch (opcode) {

	case OpcodeAdd2:

		break;

	case OpcodeStore:

		break;

	default:
		std::cerr << "IRNativeAMD64::Add: Unimplemented opcode "
		    << opcode << ".\n";
		throw std::exception();
	}
}


size_t IRNativeAMD64::GetSize() const
{
	return 1;	// TODO
}


void IRNativeAMD64::Generate(size_t maxSize, uint8_t * dst) const
{
	// Function prelude: Nothing on amd64.

	// TODO
	
	// Function postlude: Just a retq will do.
	dst[0] = 0xc3;
}


#endif	// NATIVE_ABI_AMD64


/*****************************************************************************/


#ifndef WITHOUTUNITTESTS

#include <sys/mman.h>

// TODO: If these tests are written more or less architecture independent,
// then they could be reused for e.g. Alpha as well.

static void Test_IRNativeAMD64_DoNothing()
{
#ifdef NATIVE_ABI_AMD64
	IRNativeAMD64 native;

	size_t size = native.GetSize();
	UnitTest::Assert("size should be non-zero, since the return instruction"
	    " must be in there somewhere", size >= 1);

	uint8_t * buf = (uint8_t *) mmap(NULL, size,
	    PROT_EXEC | PROT_WRITE | PROT_READ, MAP_ANON | MAP_PRIVATE, -1, 0);
	UnitTest::Assert("unable to mmap?", buf != MAP_FAILED && buf != NULL);

	native.Generate(size, buf);

	void (*func)() = (void (*)()) buf;
	func();

	munmap(buf, size);
#endif	// NATIVE_ABI_AMD64
}

UNITTESTS(IRNativeAMD64)
{
	UNITTEST(Test_IRNativeAMD64_DoNothing);
}

#endif
