#ifndef ADDCOMPONENTACTION_H
#define	ADDCOMPONENTACTION_H

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
 * \brief An Action which adds a Component as a child to another %Component.
 *
 * The action is undoable. Undoing the action means removing the %Component.
 */
class AddComponentAction
	: public Action
	, public UnitTestable
{
public:
	/**
	 * \brief Constructs an %AddComponentAction.
	 *
	 * @param componentToAdd A reference counted pointer to the Component
	 *	to add.
	 * @param whereToAddIt A reference counted pointer to the Component
	 *	which will be the parent of the newly added Component.
	 */
	AddComponentAction(refcount_ptr<Component> componentToAdd,
			   refcount_ptr<Component> whereToAddIt);

	virtual ~AddComponentAction();

	/**
	 * \brief When called, adds the specified component.
	 */
	void Execute();

	/**
	 * \brief When called, resets the state to what it was before adding
	 *	the component, by removing the component.
	 */
	void Undo();


	/********************************************************************/

	static void RunUnitTests(int& nSucceeded, int& nFailures);

private:
	refcount_ptr<Component>		m_componentToAdd;
	refcount_ptr<Component>		m_whereToAddIt;	
};


#endif	// ADDCOMPONENTACTION_H
