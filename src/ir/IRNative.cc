/*
 *  Copyright (C) 2009  Anders Gavare.  All rights reserved.
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

#include "IRNative.h"

#ifdef NATIVE_CODE_GENERATION


/*
 *  Note: This .cc module needs to be compiled without -ansi -pedantic,
 *  since calling generated code like this gives a warning.
 */
void IRNative::Execute(void *addr)
{
	void (*func)() = (void (*)()) addr;
	func();
}


#endif	// NATIVE_CODE_GENERATION


/*****************************************************************************/


#ifdef WITHUNITTESTS
#ifdef NATIVE_CODE_GENERATION

#include <sys/mman.h>

static int variable;
static void SmallFunction()
{
	variable = 123;
}

static void Test_IRNative_Execute()
{
	// Tests that it is possible to execute code, given a pointer to
	// a void function.

	variable = 42;
	UnitTest::Assert("variable before", variable, 42);

	IRNative::Execute((void*)&SmallFunction);

	UnitTest::Assert("variable after", variable, 123);
}

#endif	// NATIVE_CODE_GENERATION

UNITTESTS(IRNative)
{
#ifdef NATIVE_CODE_GENERATION
	UNITTEST(Test_IRNative_Execute);
#endif	// NATIVE_CODE_GENERATION
}

#endif
