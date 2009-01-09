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

#include <assert.h>

#include "actions/RemoveComponentAction.h"
#include "components/DummyComponent.h"
#include "GXemul.h"


RemoveComponentAction::RemoveComponentAction(
		GXemul& gxemul,
		refcount_ptr<Component> componentToRemove,
		refcount_ptr<Component> whereToRemoveItFrom)
	: Action("remove component")
	, m_gxemul(gxemul)
	, m_componentToRemove(componentToRemove->GeneratePath())
	, m_whereToRemoveItFrom(whereToRemoveItFrom->GeneratePath())
	, m_insertPositionForUndo((size_t) -1)
{
}


RemoveComponentAction::~RemoveComponentAction()
{
}


void RemoveComponentAction::Execute()
{
	refcount_ptr<Component> whereToRemoveItFrom =
	    m_gxemul.GetRootComponent()->LookupPath(m_whereToRemoveItFrom);
	m_componentToAddOnUndo =
	    m_gxemul.GetRootComponent()->LookupPath(m_componentToRemove);

	m_insertPositionForUndo =
	    whereToRemoveItFrom->RemoveChild(m_componentToAddOnUndo);

	assert(m_componentToAddOnUndo->GetParent() == NULL);

	m_oldDirtyFlag = m_gxemul.GetDirtyFlag();
	m_gxemul.SetDirtyFlag(true);
}


void RemoveComponentAction::Undo()
{
	refcount_ptr<Component> whereToRemoveItFrom =
	    m_gxemul.GetRootComponent()->LookupPath(m_whereToRemoveItFrom);

	whereToRemoveItFrom->AddChild(m_componentToAddOnUndo,
	    m_insertPositionForUndo);

	m_componentToAddOnUndo = NULL;

	m_gxemul.SetDirtyFlag(m_oldDirtyFlag);
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

#include "actions/ResetAction.h"

static void Test_RemoveComponentAction_WithUndoRedo()
{
	GXemul gxemulDummy;

	refcount_ptr<Component> dummy = new DummyComponent;
	refcount_ptr<Component> dummyChildA = new DummyComponent;
	refcount_ptr<Component> dummyChildA1 = new DummyComponent;
	refcount_ptr<Component> dummyChildA2 = new DummyComponent;

	dummy->SetVariableValue("x", "\"value 1\"");
	dummy->SetVariableValue("y", "\"value 2\"");
	dummyChildA1->SetVariableValue("x", "\"value\nhello\"");
	dummyChildA2->SetVariableValue("something", "");
	dummyChildA2->SetVariableValue("numericTest", "123");
	dummyChildA2->SetVariableValue("numericTest2", "0");
	dummyChildA2->SetVariableValue("numericTest3", "-1");

	dummyChildA->AddChild(dummyChildA1);
	dummyChildA->AddChild(dummyChildA2);
	dummy->AddChild(dummyChildA);
	gxemulDummy.GetRootComponent()->AddChild(dummy);

	Checksum checksum0;
	dummy->AddChecksum(checksum0);

	ActionStack& actionStack = gxemulDummy.GetActionStack();

	refcount_ptr<Action> actionA = new RemoveComponentAction(
	    gxemulDummy, dummyChildA1, dummyChildA);

	UnitTest::Assert("dummyChildA should have two children initially",
	    dummyChildA->GetChildren().size(), 2);

	actionStack.PushActionAndExecute(actionA);

	UnitTest::Assert("dummyChildA should have 1 child after the Remove",
	    dummyChildA->GetChildren().size(), 1);

	Checksum checksumAfterRemove;
	dummy->AddChecksum(checksumAfterRemove);

	UnitTest::Assert("Remove should have changed checksum",
	    checksum0 != checksumAfterRemove);

	actionStack.Undo();

	UnitTest::Assert("dummyChildA should have two children after Undo",
	    dummyChildA->GetChildren().size(), 2);

	Checksum checksumAfterUndo;
	dummy->AddChecksum(checksumAfterUndo);

	UnitTest::Assert("Undo should have restored everything",
	    checksum0 == checksumAfterUndo);

	actionStack.Redo();

	UnitTest::Assert("dummyChildA should have 1 child after Redo",
	    dummyChildA->GetChildren().size(), 1);

	Checksum checksumAfterRedo;
	dummy->AddChecksum(checksumAfterRedo);

	UnitTest::Assert("Redo should have same checksum as after the"
		" initial Remove",
	    checksumAfterRedo == checksumAfterRemove);
}

static void Test_RemoveComponentAction_RemoveNotUsingReferences()
{
	GXemul gxemulDummy;
	
	refcount_ptr<Component> dummy = new DummyComponent;
	refcount_ptr<Component> dummyChildA = new DummyComponent;

	gxemulDummy.GetRootComponent()->AddChild(dummyChildA);
	gxemulDummy.GetRootComponent()->AddChild(dummy);

	UnitTest::Assert("there should be two children initially",
	    gxemulDummy.GetRootComponent()->GetChildren().size(), 2);

	ActionStack& actionStack = gxemulDummy.GetActionStack();

	refcount_ptr<Action> actionA = new RemoveComponentAction(
	    gxemulDummy, dummyChildA, gxemulDummy.GetRootComponent());

	actionStack.PushActionAndExecute(actionA);

	UnitTest::Assert("there should now be one child",
	    gxemulDummy.GetRootComponent()->GetChildren().size(), 1);

	refcount_ptr<Action> actionReset = new ResetAction(gxemulDummy);
	actionStack.PushActionAndExecute(actionReset);

	actionStack.Undo();
	actionStack.Undo();

	UnitTest::Assert("there should be two children again",
	    gxemulDummy.GetRootComponent()->GetChildren().size(), 2);
}

UNITTESTS(RemoveComponentAction)
{
	UNITTEST(Test_RemoveComponentAction_WithUndoRedo);
	UNITTEST(Test_RemoveComponentAction_RemoveNotUsingReferences);
}

#endif
