#ifndef RESETACTION_H
#define	RESETACTION_H

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

#include "Action.h"
#include "Component.h"
#include "UnitTest.h"

class GXemul;


/**
 * \brief An Action which resets components to their initial state.
 *
 * The action is undoable. The action keeps a cloned copy of the entire
 * component tree, and then resets the original only. Undoing the action
 * means to go back to the clone again.
 */
class ResetAction
	: public Action
	, public UnitTestable
{
public:
	/**
	 * \brief Constructs a %ResetAction.
	 *
	 * @param gxemul A reference to the GXemul instance.
	 */
	ResetAction(GXemul& gxemul);

	virtual ~ResetAction();


	/********************************************************************/

	static void RunUnitTests(int& nSucceeded, int& nFailures);

private:
	/**
	 * \brief When called, resets the specified Component.
	 *
	 * The position in the child vector where the %Component was is
	 * remembered, in order to allow Undo to work.
	 */
	void Execute();

	/**
	 * \brief When called, restores the state to what it was before
	 *	resetting the Component, by re-adding the component (inserting
	 *	it at the position where it was before it was reset).
	 */
	void Undo();

private:
	GXemul&				m_gxemul;
	refcount_ptr<Component>		m_oldComponentTree;
	bool				m_oldDirtyFlag;
};


#endif	// RESETACTION_H
