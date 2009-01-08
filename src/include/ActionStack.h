#ifndef ACTIONSTACK_H
#define	ACTIONSTACK_H

/*
 *  Copyright (C) 2007-2009  Anders Gavare.  All rights reserved.
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
#include "UnitTest.h"


class GXemul;

/**
 * \brief A stack of Action objects, for implementing undo/redo functionality.
 *
 * The main purpose of this class, together with the Action class, is to 
 * enable undo/redo functionality for the end user via the GUI, but the
 * functionality is also available from the text-only interface.
 *
 * Technically, the %ActionStack consists of two lists, one for Undo, and one
 * for Redo. By using lists, the time required for pushing and poping is
 * O(1).
 *
 * All modifications to the GXemul instance' Component tree should be done
 * using Actions, which are executed only using the ActionStack's
 * PushActionAndExecute() function, or by the Undo/Redo functions.
 */
class ActionStack
	: public UnitTestable
{
public:
	/**
	 * \brief Constructs an %ActionStack.
	 *
	 * \param gxemul A pointer to the owner GXemul instance. If non-NULL,
	 *	then UI updates will be triggered whenever the contents of
	 *	the %ActionStack changes.
	 */
	ActionStack(GXemul* gxemul = NULL);

	/**
	 * \brief Clears both the Undo and the Redo stacks.
	 */
	void Clear();

	/**
	 * \brief Clears the Redo stack.
	 */
	void ClearRedo();

	/**
	 * \brief Checks if the undo stack has any items.
	 *
	 * The main purpose of this is for indication in the GUI. If the 
	 * stack is empty, the undo button (or undo menu entry) can be 
	 * greyed out.
	 *
	 * @return true if there are undo actions, false if the undo
	 *		stack is empty
	 */
	bool IsUndoPossible() const;

	/**
	 * \brief Checks if the redo stack has any items.
	 *
	 * The main purpose of this is for indication in the GUI. If the stack
	 * is empty, the redo button (or redo menu entry) can be greyed out.
	 *
	 * @return true if there are redo actions, false if the redo
	 *		stack is empty
	 */
	bool IsRedoPossible() const;

	/**
	 * \brief Checks how many actions are in the undo stack.
	 *
	 * Note: This is not guaranteed to be O(1).
	 *
	 * The main purpose of this function is for use in unit tests!
	 *
	 * @return the number of undoable actions
	 */
	int GetNrOfUndoableActions() const;

	/**
	 * \brief Checks how many actions are in the redo stack.
	 *
	 * Note: This is not guaranteed to be O(1).
	 *
	 * The main purpose of this function is for use in unit tests!
	 *
	 * @return the number of redoable actions
	 */
	int GetNrOfRedoableActions() const;

	/**
	 * \brief Pushes an Action onto the undo stack, and executes it.
	 *
	 * The redo stack is always cleared when this function is used.
	 * In practice, this means that as long as the user does not issue
	 * <i>new</i> actions, it is possible to move back and forward in
	 * time by using undo/redo. As soon as the user issues a new action,
	 * this breaks the possibility to go forward in the old redo history.
	 *
	 * If the %Action is not undoable, the undo stack is cleared.
	 * Otherwise, the undo stack is kept, and the action is pushed
	 * onto the stack.
	 *
	 * (Note: The "Push" part of the function's name is thus only valid
	 * for undoable actions. If the action is not undoable, nothing
	 * is really pushed.)
	 *
	 * @param pAction a pointer to the Action to push
	 */
	void PushActionAndExecute(refcount_ptr<Action>& pAction);

	/**
	 * \brief Undoes the last pushed Action, if any.
	 *
	 * The Action's Undo function is executed, and the %Action is moved
	 * to the redo stack from the undo stack.
	 *
	 * @return true if an action was undone, false if the undo stack
	 *	was empty
	 */
	bool Undo();

	/**
	 * \brief Redoes an Action from the redo stack, if any is available.
	 *
	 * The Action's Execute function is executed, and the %Action is moved
	 * to the undo stack from the redo stack.
	 *
	 * @return true if an action was redone, false if the redo stack
	 *	was empty
	 */
	bool Redo();


	/********************************************************************/

	static void RunUnitTests(int& nSucceeded, int& nFailures);

private:
	GXemul *			m_gxemul;
	list< refcount_ptr<Action> >	m_undoActions;
	list< refcount_ptr<Action> >	m_redoActions;
};


#endif	// ACTIONSTACK_H
