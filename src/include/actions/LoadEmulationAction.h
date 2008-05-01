#ifndef LOADEMULATIONACTION_H
#define	LOADEMULATIONACTION_H

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

#include "misc.h"

#include "Action.h"
#include "Component.h"
#include "UnitTest.h"

class GXemul;


/**
 * \brief An Action which loads a component tree from a file, and adds it
 *	to a GXemul instance' current emulation setup, <i>or</i> replaces
 *	the %GXemul instance' root component completely.
 *
 * The action is undoable if a component was loaded to a specific place
 * in the tree. If the root component was replaced, the ActionStack is
 * cleared, which in practice makes the action non-undoable.
 *
 * The %GXemul instance' root component is first reset to a dummy root node,
 * so that the current emulation is freed from memory. A new emulation is
 * then loaded.
 *
 * If the target component path was "", then the %GXemul instance' emulation
 * filename will be updated as well.
 *
 * (Note that specifying "root" as the target component path will load
 * the file and <i>add</i> that to the "root".)
 */
class LoadEmulationAction
	: public Action
	, public UnitTestable
{
public:
	/**
	 * \brief Constructs an %LoadEmulationAction.
	 *
	 * @param gxemul The GXemul instance.
	 * @param filename The filename to load.
	 * @param path The component path where to add this emulation. This
	 *	should be an empty string to replace the root node. (If this
	 *	is set to "root", then the loaded component tree is <i>added</i>
	 *	as a child to the root node.)
	 */
	LoadEmulationAction(GXemul& gxemul, const string& filename,
		const string& path);

	virtual ~LoadEmulationAction();


	/********************************************************************/

	static void RunUnitTests(int& nSucceeded, int& nFailures);

private:
	/**
	 * \brief When called, loads a component from a file, and either
	 *	replaces the GXemul instance' root component with it, or
	 *	adds it somewhere in the current emulation tree.
	 */
	void Execute();

	/**
	 * \brief Removes the added component, if possible.
	 */
	void Undo();

private:
	GXemul&		m_gxemul;
	string		m_filename;
	string		m_path;

	// For Undo:
	string		m_addedComponentPath;
	string		m_whereItWasAdded;
	bool		m_oldDirtyFlag;
};


#endif	// LOADEMULATIONACTION_H
