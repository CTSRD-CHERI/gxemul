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

#include <sys/types.h>
#include <sys/mman.h>
#include "IRBlockCache.h"


IRBlockCache::IRBlockCache(size_t size)
	: m_size(size)
	, m_cache(NULL)
	, m_generation(0)
	, m_currentlyUsed(0)
{
	// Note: The cache memory is not allocated here, but "lazily" in
	// InitIfNotYetInitialized() below.
}


IRBlockCache::~IRBlockCache()
{
	if (m_cache != NULL) {
		munmap(m_cache, m_size);
		m_cache = NULL;
	}
}


void IRBlockCache::SetSize(size_t size)
{
	if (m_cache != NULL) {
		munmap(m_cache, m_size);
		m_cache = NULL;
	}

	m_size = size;

	Reset();
}


void IRBlockCache::InitIfNotYetInitialized()
{
	if (m_cache != NULL)
		return;

	// PROT_EXEC is the important thing here, since the translation
	// cache will contain generated Executable code.
	m_cache = mmap(NULL, m_size, PROT_WRITE | PROT_READ | PROT_EXEC,
	    MAP_ANON | MAP_PRIVATE, -1, 0);

	if (m_cache == MAP_FAILED || m_cache == NULL) {
		std::cerr << "IRBlockCache: Could not allocate " << m_size
		    << " bytes for the translation cache. Aborting.\n";
		throw std::exception();
	}
}


void IRBlockCache::Reset()
{
	++ m_generation;
	m_currentlyUsed = 0;

#ifndef NDEBUG
	std::cout << "[ IRBlockCache::Reset() ]\n";
#endif
}


void * IRBlockCache::Allocate(size_t bytes, int& generation)
{
	// Make sure that bytes is reasonably aligned... (e.g. 64 bytes.)
	bytes = ((bytes - 1) | 63) + 1;

	if (bytes > m_size)
		SetSize(bytes);

	InitIfNotYetInitialized();

	size_t newUsedSize = m_currentlyUsed + bytes;
	if (newUsedSize > m_size) {
		Reset();
		newUsedSize = bytes;
	}

	void * returnValue = ((uint8_t *) m_cache) + m_currentlyUsed;
	m_currentlyUsed = newUsedSize;

	generation = m_generation;
	return returnValue;
}


/*****************************************************************************/


#ifndef WITHOUTUNITTESTS

static void Test_IRBlockCache_Constructor()
{
	IRBlockCache irbc(200);
	
	UnitTest::Assert("initial size",
	    irbc.GetCacheSize(), 200);
	UnitTest::Assert("initial generation should be zero",
	    irbc.GetGeneration(), 0);
	UnitTest::Assert("no cache data should have been allocated yet (lazy)",
	    irbc.GetCachePointer() == NULL);
}

static void Test_IRBlockCache_Reset()
{
	IRBlockCache irbc(200);
	
	UnitTest::Assert("initial generation should be zero",
	    irbc.GetGeneration(), 0);

	irbc.Reset();

	UnitTest::Assert("size should not be changed by reset",
	    irbc.GetCacheSize(), 200);
	UnitTest::Assert("reset should increase the generation count",
	    irbc.GetGeneration(), 1);
	UnitTest::Assert("no cache data should have been allocated by reset",
	    irbc.GetCachePointer() == NULL);
}

static void Test_IRBlockCache_SetSize()
{
	IRBlockCache irbc(200);
	
	UnitTest::Assert("initial generation should be zero",
	    irbc.GetGeneration(), 0);
	UnitTest::Assert("initial size",
	    irbc.GetCacheSize(), 200);

	int genA = 42;
	void * ptrA = irbc.Allocate(100, genA);
	UnitTest::Assert("cache data should have been allocated",
	    irbc.GetCachePointer() != NULL);

	irbc.SetSize(500);

	UnitTest::Assert("setsize should have changed the size",
	    irbc.GetCacheSize(), 500);
	UnitTest::Assert("setsize should increase the generation count",
	    irbc.GetGeneration(), 1);
	UnitTest::Assert("cache data should have been freed by setsize",
	    irbc.GetCachePointer() == NULL);

	ptrA = irbc.Allocate(100, genA);
	UnitTest::Assert("cache data should have been allocated again",
	    irbc.GetCachePointer() != NULL);
}

static void Test_IRBlockCache_Allocate()
{
	IRBlockCache irbc(1048576);

	int genA = 42;
	void * ptrA = irbc.Allocate(100000, genA);

	UnitTest::Assert("cache data should have been allocated",
	    irbc.GetCachePointer() != NULL);

	UnitTest::Assert("allocate A should have succeeded", ptrA != NULL);
	UnitTest::Assert("generation of first allocation should be zero",
	    genA, 0);

	int genB = 123;
	void * ptrB = irbc.Allocate(100000, genB);

	UnitTest::Assert("allocate B should have succeeded", ptrB != NULL);
	UnitTest::Assert("generation of second allocation should also be zero",
	    genB, 0);

	// Allocate so much that the cache needs to be reset:
	int genC = -20;
	void * ptrC = irbc.Allocate(1048500, genC);

	UnitTest::Assert("allocate C should have succeeded", ptrC != NULL);
	UnitTest::Assert("generation of third allocation should be one",
	    genC, 1);
}

static void Test_IRBlockCache_AllocateTooMuch()
{
	IRBlockCache irbc(200);

	int genA = 42;
	void * ptrA = irbc.Allocate(100000, genA);

	UnitTest::Assert("cache data should have been allocated",
	    irbc.GetCachePointer() != NULL);

	// Note: greater-than-or-equal in this test, because the cache size
	// can be aligned upwards:
	UnitTest::Assert("size should have been increased implicitly",
	    irbc.GetCacheSize() >= 100000);

	UnitTest::Assert("allocate A should have succeeded", ptrA != NULL);
	UnitTest::Assert("generation of the allocation should be one "
		"(increased because of implicit resize)", genA, 1);
}

UNITTESTS(IRBlockCache)
{
	UNITTEST(Test_IRBlockCache_Constructor);
	UNITTEST(Test_IRBlockCache_Reset);
	UNITTEST(Test_IRBlockCache_SetSize);
	UNITTEST(Test_IRBlockCache_Allocate);
	UNITTEST(Test_IRBlockCache_AllocateTooMuch);
}

#endif
