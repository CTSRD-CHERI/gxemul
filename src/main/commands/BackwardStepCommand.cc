/*
 *  Copyright (C) 2009  Anders Gavare.  All rights reserved.
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

#include "commands/BackwardStepCommand.h"
#include "GXemul.h"


BackwardStepCommand::BackwardStepCommand()
	: Command("backward-step", "")
{
}


BackwardStepCommand::~BackwardStepCommand()
{
}


bool BackwardStepCommand::Execute(GXemul& gxemul, const vector<string>& arguments)
{
	gxemul.SetRunState(GXemul::BackwardsSingleStepping);
	return true;
}


string BackwardStepCommand::GetShortDescription() const
{
	return "Runs one step of the emulation backwards.";
}


string BackwardStepCommand::GetLongDescription() const
{
	return "Runs one step of the emulation backwards.";
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

static void Test_BackwardStepCommand_Affect_RunState()
{
	refcount_ptr<Command> cmd = new BackwardStepCommand;
	vector<string> dummyArguments;
	
	GXemul gxemul;

	UnitTest::Assert("the default GXemul instance should be Paused",
	    gxemul.GetRunState() == GXemul::Paused);

	cmd->Execute(gxemul, dummyArguments);

	UnitTest::Assert("runstate should have been changed to BackwardsSingleStepping",
	    gxemul.GetRunState() == GXemul::BackwardsSingleStepping);
}

UNITTESTS(BackwardStepCommand)
{
	UNITTEST(Test_BackwardStepCommand_Affect_RunState);
}

#endif
