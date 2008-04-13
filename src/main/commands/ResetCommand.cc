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

#include "commands/ResetCommand.h"
#include "GXemul.h"


ResetCommand::ResetCommand()
	: Command("reset", "")
{
}


ResetCommand::~ResetCommand()
{
}


void ResetCommand::Execute(GXemul& gxemul, const vector<string>& arguments)
{
	gxemul.SetRunState(GXemul::NotRunning);

	// TODO: Reset all components.
}


string ResetCommand::GetShortDescription() const
{
	return _("Resets the current emulation.");
}


string ResetCommand::GetLongDescription() const
{
	return _("Resets the emulation, by clearing the state of all\n"
	    "components, and setting the current RunState to NotRunning.");
}


/*****************************************************************************/


#ifndef WITHOUTUNITTESTS

static void Test_ResetCommand_Affect_RunState()
{
	refcount_ptr<Command> cmd = new ResetCommand;
	vector<string> dummyArguments;
	
	GXemul gxemul(false);

	UnitTest::Assert("the default GXemul instance should be NotRunning",
	    gxemul.GetRunState() == GXemul::NotRunning);

	gxemul.SetRunState(GXemul::Paused);

	UnitTest::Assert("the runstate should now be Paused",
	    gxemul.GetRunState() == GXemul::Paused);

	cmd->Execute(gxemul, dummyArguments);

	UnitTest::Assert("runstate should have been changed to NotRunning",
	    gxemul.GetRunState() == GXemul::NotRunning);
}

UNITTESTS(ResetCommand)
{
	UNITTEST(Test_ResetCommand_Affect_RunState);
}

#endif
