#ifndef REDOCOMMAND_H
#define	REDOCOMMAND_H

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

#include "Command.h"


/**
 * \brief A Command which Re-does the next redoable action (if any) in a
 *	GXemul instance' ActionStack.
 */
class RedoCommand
	: public Command
{
public:
	/**
	 * \brief Constructs an %RedoCommand.
	 */
	RedoCommand();

	virtual ~RedoCommand();

	/**
	 * \brief Executes the command: Re-does the next redoable action
	 *	(if any) in a GXemul instance.
	 *
	 * @param gxemul A reference to the GXemul instance.
	 * @param arguments A vector of zero or more string arguments.
	 */
	virtual void Execute(GXemul& gxemul, const vector<string>& arguments);

	virtual string GetShortDescription() const;

	virtual string GetLongDescription() const;
};


#endif	// REDOCOMMAND_H
