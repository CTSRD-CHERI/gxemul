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

#include <assert.h>

#include "actions/ResetAction.h"
#include "components/DummyComponent.h"
#include "GXemul.h"


ResetAction::ResetAction(GXemul& gxemul)
	: Action("reset emulation")
	, m_gxemul(gxemul)
{
}


ResetAction::~ResetAction()
{
}


void ResetAction::Execute()
{
	refcount_ptr<Component> root = m_gxemul.GetRootComponent();
	m_oldComponentTree = root->Clone();
	root->Reset();
}


void ResetAction::Undo()
{
	m_gxemul.SetRootComponent(m_oldComponentTree);
}


/*****************************************************************************/


#ifndef WITHOUTUNITTESTS

static void Test_ResetAction_WithUndoRedo()
{
	refcount_ptr<Component> dummy = new DummyComponent;
	refcount_ptr<Component> dummyChildA = new DummyComponent;

	dummy->AddChild(dummyChildA);

	Checksum checksum0;
	dummy->AddChecksum(checksum0);

	// Change some state variables:
	dummy->SetVariableValue("x", "value 1");
	dummyChildA->SetVariableValue("y", "value 2");

	Checksum checksum1;
	dummy->AddChecksum(checksum1);

	UnitTest::Assert("changing state variables should have changed the"
		" checksum!", checksum0 == checksum1);

	GXemul gxemul(false);
	refcount_ptr<Action> actionA = new ResetAction(gxemul);

	gxemul.GetActionStack().PushActionAndExecute(actionA);

	// Resetting should have caused the state to be reset:
	Checksum checksum2;
	dummy->AddChecksum(checksum2);

	UnitTest::Assert("reset should have succeeded in resetting the state",
	    checksum0 == checksum2);

	// Undo should restore state:
	gxemul.GetActionStack().Undo();

	Checksum checksum3;
	dummy->AddChecksum(checksum3);

	UnitTest::Assert("reset should have succeeded in resetting the state",
	    checksum1 == checksum3);

	// Redo should reset again:
	gxemul.GetActionStack().Redo();

	Checksum checksum4;
	dummy->AddChecksum(checksum4);

	UnitTest::Assert("redo should have succeeded in resetting the state",
	    checksum0 == checksum4);
}

UNITTESTS(ResetAction)
{
	UNITTEST(Test_ResetAction_WithUndoRedo);
}

#endif
