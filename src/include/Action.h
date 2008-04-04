#ifndef ACTION_H
#define	ACTION_H

/*
 *  Copyright (C) 2007-2008  Anders Gavare.  All rights reserved.
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


/**
 * \brief Actions are wrappers around undoable/redoable function calls.
 *
 * The main purpose of this class, together with the ActionStack class, is 
 * to enable undo/redo functionality for the end user via the GUI, but the 
 * functionality is also available from the text-only interface.
 */
class Action
	: public ReferenceCountable
{
public:
	/**
	 * \brief Base constructor for an Action.
	 *
	 * @param description	A short description describing the
	 *			action. For e.g. the "add component"
	 *			action, it can be "add component".
	 * @param undoable	True if the action is undoable, false
	 *			if it should clear the undo stack on
	 *			execution.
	 */
	Action(const string& description, bool undoable = true);

	virtual ~Action();

	/**
	 * \brief Called to execute the %Action.
	 */
	virtual void Execute() = 0;

	/**
	 * \brief Called to execute the %Action in reverse, i.e. undo it.
	 */
	virtual void Undo() = 0;

	/**
	 * \brief Checks if the %Action is undoable.
	 *
	 * @return true if the action is undoable (i.e. if the Undo
	 *		function is implemented in a meaningful way),
	 *		false if undo is not possible
	 */
	bool IsUndoable() const;

	/**
	 * \brief Retrieves the description of the %Action.
	 *
	 * @return The description of the %Action.
	 */
	const string& GetDescription() const;

private:
	string	m_description;
	bool	m_undoable;
};


#endif	// ACTION_H
