#ifndef IRBLOCKCACHE_H
#define	IRBLOCKCACHE_H

/*
 *  Copyright (C) 2008-2009  Anders Gavare.  All rights reserved.
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

#include "misc.h"

#include "UnitTest.h"


/**
 * \brief A cache for IRBlock native translation allocations.
 *
 * Note that when the cache is all used up, it is completely reset, for
 * simplicity.
 *
 * Usually, there will be one static instance of the %IRBlockCache in the
 * application. The actual memory is only allocated on the first call to
 * Allocate(), so if an %IRBlockCache object is instantiated but never really
 * used, it does not take much memory.
 *
 * Note also that when/if the cache is reset, there will be no callback
 * to any previous callers who might have used Allocate(). The way to
 * deal with this is to always check using GetGeneration() to see that
 * the cache is of the same generation as when Allocate() was called.
 * Only then should the allocation be considered to still be valid.
 *
 * The cache may change size during runtime by calling SetSize(), but this
 * should normally not be necessary, if the cache is of a reasonable size
 * to begin with.
 */
class IRBlockCache
	: public UnitTestable
{
public:
	/**
	 * \brief Constructs an %IRBlockCache instance.
	 *
	 * \param size The size of the cache, measured in bytes.
	 */
	IRBlockCache(size_t size);

	~IRBlockCache();

	/**
	 * \brief Sets the size of the cache.
	 *
	 * Note: This resets the contents of the cache (invalidates any
	 * translations that has been stored in it).
	 *
	 * \param size The new size of the cache, in bytes.
	 */
	void SetSize(size_t size);

	/**
	 * \brief Allocates memory from the cache.
	 *
	 * Should always return a valid pointer, never NULL. If the requested
	 * number of bytes is larger than the total cache size, the cache size
	 * is expanded.
	 *
	 * \param bytes	The amount of bytes to allocate.
	 * \param generation A reference to a generation counter. The allocation
	 *	is only valid for this specific generation.
	 * \return A pointer to the newly allocated memory.
	 */
	void * Allocate(size_t bytes, int& generation);

	/**
	 * \brief Resets the cache.
	 *
	 * This increases the generation count by 1, and resets the "currently
	 * used" amount to zero.
	 */
	void Reset();

	/**
	 * \brief Get the current generation number.
	 *
	 * This function should be called before executing code in the
	 * cache, to make sure that the generation is the same. The generation
	 * number is increased every time the cache overflows and is reset.
	 *
	 * \return The generation number.
	 */
	int GetGeneration() const
	{
		return m_generation;
	}

	/**
	 * \brief Get a pointer to the cache (for debugging).
	 *
	 * This function is only meant for debugging; don't call this function
	 * with the intention of storing anything in the cache. Use
	 * Allocate() instead.
	 *
	 * \return A pointer to the cache data, or NULL if it has not yet
	 *	been allocated.
	 */
	const void * GetCachePointer() const
	{
		return m_cache;
	}

	/**
	 * \brief Gets the current cache size.
	 *
	 * \return The current cache size, in bytes.
	 */
	const size_t GetCacheSize() const
	{
		return m_size;
	}


	/********************************************************************/

	static void RunUnitTests(int& nSucceeded, int& nFailures);

private:
	void InitIfNotYetInitialized();

private:
	size_t	m_size;			// cache size in bytes
	void	*m_cache;		// pointer to the entire cache block
	int	m_generation;		// current cache generation
	size_t	m_currentlyUsed;	// size of cache currently in use
};


#endif	// IRBLOCKCACHE_H
