#ifndef FILELOADER_ELF_H
#define	FILELOADER_ELF_H

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

#include "Component.h"
#include "UnitTest.h"


/**
 * \brief ELF binary loader.
 *
 * TODO: Longer comment.
 */
class FileLoader_ELF
	: public UnitTestable
{
public:
	/**
	 * \brief Constructs a %FileLoader_ELF.
	 *
	 * \param filename The filename to load. This must be an ELF file.
	 */
	FileLoader_ELF(const string& filename);

	/**
	 * \brief Loads the ELF into a Component.
	 *
	 * \param component The AddressDataBus component to load the file
	 *	into. (This is usually a CPUComponent.)
	 * \return True if loading succeeded, false otherwise.
	 */
	bool LoadIntoComponent(refcount_ptr<Component> component);


	/********************************************************************/

	static void RunUnitTests(int& nSucceeded, int& nFailures);

private:
	// Disallow construction without arguments.
	FileLoader_ELF();

private:
	const string		m_filename;
};


#endif	// FILELOADER_ELF_H
