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

#include "ActionStack.h"
#include "GXemul.h"


ActionStack::ActionStack(GXemul *gxemul)
	: m_gxemul(gxemul)
{
}


void ActionStack::Clear()
{
	m_undoActions.clear();
	m_redoActions.clear();

	if (m_gxemul != NULL)
		m_gxemul->GetUI()->UpdateUI();
}


void ActionStack::ClearRedo()
{
	m_redoActions.clear();

	if (m_gxemul != NULL)
		m_gxemul->GetUI()->UpdateUI();
}


bool ActionStack::IsUndoPossible() const
{
	return !m_undoActions.empty();
}


bool ActionStack::IsRedoPossible() const
{
	return !m_redoActions.empty();
}


int ActionStack::GetNrOfUndoableActions() const
{
	return m_undoActions.size();
}


int ActionStack::GetNrOfRedoableActions() const
{
	return m_redoActions.size();
}


void ActionStack::PushActionAndExecute(refcount_ptr<Action>& pAction)
{
	if (pAction->IsUndoable()) {
		m_undoActions.push_back(pAction);
		ClearRedo();
	} else {
		Clear();
	}

	if (m_gxemul != NULL)
		m_gxemul->GetUI()->UpdateUI();

	// Note that the action is executed after any modification to the
	// stack itself is done. This is because the Execute function may
	// modify the stack too (e.g. by calling Clear()).
	pAction->Execute();
}


bool ActionStack::Undo()
{
	if (m_undoActions.empty())
		return false;

	refcount_ptr<Action> pAction = m_undoActions.back();
	m_undoActions.pop_back();

	pAction->Undo();

	m_redoActions.push_back(pAction);

	if (m_gxemul != NULL)
		m_gxemul->GetUI()->UpdateUI();

	return true;
}


bool ActionStack::Redo()
{
	if (m_redoActions.empty())
		return false;

	refcount_ptr<Action> pAction = m_redoActions.back();
	m_redoActions.pop_back();

	pAction->Execute();

	m_undoActions.push_back(pAction);

	if (m_gxemul != NULL)
		m_gxemul->GetUI()->UpdateUI();

	return true;
}


/*****************************************************************************/


#ifndef WITHOUTUNITTESTS

/**
 * \brief A dummy Action, for unit testing purposes
 */
class DummyAction : public Action
{
public:
	DummyAction(int& valueRef)
		: Action("dummy action")
		, m_value(valueRef)
	{
	}

	virtual void Execute()
	{
		m_value ++;
	}

	virtual void Undo()
	{
		m_value --;
	}

private:
	int&	m_value;
};


/**
 * \brief A dummy non-undoable Action,
 *	for unit testing purposes
 */
class DummyNonUndoableAction : public Action
{
public:
	DummyNonUndoableAction(int& valueRef)
		: Action("dummy non-undoable action", false)
		, m_value(valueRef)
	{
	}

	virtual void Execute()
	{
		m_value ++;
	}

	virtual void Undo()
	{
		/*  Should never be here.  */
		throw std::exception();
	}

private:
	int&	m_value;
};


static void Test_ActionStack_IsInitiallyEmpty()
{
	ActionStack stack;

	UnitTest::Assert("undo should not be possible",
	    stack.IsUndoPossible() == false);
	UnitTest::Assert("redo should not be possible",
	    stack.IsRedoPossible() == false);

	UnitTest::Assert("undo stack should be empty",
	    stack.GetNrOfUndoableActions() == 0);
	UnitTest::Assert("redo stack should be empty",
	    stack.GetNrOfRedoableActions() == 0);
}

static void Test_ActionStack_PushAction()
{
	ActionStack stack;

	int dummyInt;
	refcount_ptr<Action> pAction1 = new DummyAction(dummyInt);
	refcount_ptr<Action> pAction2 = new DummyAction(dummyInt);

	stack.PushActionAndExecute(pAction1);

	UnitTest::Assert("undo stack should contain one Action",
	    stack.GetNrOfUndoableActions() == 1);

	stack.PushActionAndExecute(pAction2);

	UnitTest::Assert("undo stack should contain two Actions",
	    stack.GetNrOfUndoableActions() == 2);
}

static void Test_ActionStack_UndoRedo()
{
	ActionStack stack;

	int dummyInt;
	refcount_ptr<Action> pAction1 = new DummyAction(dummyInt);
	refcount_ptr<Action> pAction2 = new DummyAction(dummyInt);
	refcount_ptr<Action> pAction3 = new DummyAction(dummyInt);
	refcount_ptr<Action> pAction4 = new DummyAction(dummyInt);

	UnitTest::Assert("undo should not be possible",
	    stack.IsUndoPossible() == false);
	UnitTest::Assert("redo should not be possible",
	    stack.IsRedoPossible() == false);

	stack.PushActionAndExecute(pAction1);
	stack.PushActionAndExecute(pAction2);
	stack.PushActionAndExecute(pAction3);
	stack.PushActionAndExecute(pAction4);

	UnitTest::Assert("undo stack should contain 4 Actions",
	    stack.GetNrOfUndoableActions() == 4);
	UnitTest::Assert("redo stack should contain 0 Actions",
	    stack.GetNrOfRedoableActions() == 0);

	UnitTest::Assert("undo should be possible",
	    stack.IsUndoPossible() == true);
	UnitTest::Assert("redo should not be possible",
	    stack.IsRedoPossible() == false);

	stack.Undo();

	UnitTest::Assert("undo stack should contain 3 Actions",
	    stack.GetNrOfUndoableActions() == 3);
	UnitTest::Assert("redo stack should contain 1 Action",
	    stack.GetNrOfRedoableActions() == 1);

	UnitTest::Assert("undo should be possible",
	    stack.IsUndoPossible() == true);
	UnitTest::Assert("redo should be possible",
	    stack.IsRedoPossible() == true);

	stack.Undo();

	UnitTest::Assert("undo stack should contain 3 Actions",
	    stack.GetNrOfUndoableActions() == 2);
	UnitTest::Assert("redo stack should contain 2 Actions",
	    stack.GetNrOfRedoableActions() == 2);

	stack.Redo();

	UnitTest::Assert("undo stack should contain 3 Actions",
	    stack.GetNrOfUndoableActions() == 3);
	UnitTest::Assert("redo stack should contain 1 Action",
	    stack.GetNrOfRedoableActions() == 1);
}

static void Test_ActionStack_PushShouldClearRedoStack()
{
	ActionStack stack;

	int dummyInt;
	refcount_ptr<Action> pAction1 = new DummyAction(dummyInt);
	refcount_ptr<Action> pAction2 = new DummyAction(dummyInt);
	refcount_ptr<Action> pAction3 = new DummyAction(dummyInt);
	refcount_ptr<Action> pAction4 = new DummyAction(dummyInt);

	stack.PushActionAndExecute(pAction1);
	stack.PushActionAndExecute(pAction2);
	stack.PushActionAndExecute(pAction3);
	stack.PushActionAndExecute(pAction4);

	UnitTest::Assert("undo stack should contain 4 Actions",
	    stack.GetNrOfUndoableActions() == 4);

	UnitTest::Assert("redo stack should contain 0 Actions",
	    stack.GetNrOfRedoableActions() == 0);

	stack.Undo();

	UnitTest::Assert("redo stack should contain 1 Action",
	    stack.GetNrOfRedoableActions() == 1);

	stack.Undo();

	UnitTest::Assert("redo stack should contain 2 Actions",
	    stack.GetNrOfRedoableActions() == 2);

	refcount_ptr<Action> pAction5 = new DummyAction(dummyInt);
	stack.PushActionAndExecute(pAction5);

	UnitTest::Assert("redo stack should contain 0 Actions",
	    stack.GetNrOfRedoableActions() == 0);
}

static void Test_ActionStack_NonUndoableAction()
{
	ActionStack stack;

	int dummyInt = 42;
	refcount_ptr<Action> pAction1 = new DummyAction(dummyInt);
	refcount_ptr<Action> pAction2 = new DummyAction(dummyInt);
	refcount_ptr<Action> pAction3 = new DummyNonUndoableAction(dummyInt);
	refcount_ptr<Action> pAction4 = new DummyAction(dummyInt);

	UnitTest::Assert("A) undo should not be possible",
	    stack.IsUndoPossible() == false);

	stack.PushActionAndExecute(pAction1);
	stack.PushActionAndExecute(pAction2);

	UnitTest::Assert("B) undo should be possible",
	    stack.IsUndoPossible() == true);
	UnitTest::Assert("undo stack should contain 2 Actions",
	    stack.GetNrOfUndoableActions() == 2);

	stack.PushActionAndExecute(pAction3);

	UnitTest::Assert("C) undo should not be possible",
	    stack.IsUndoPossible() == false);
	UnitTest::Assert("undo stack should contain 0 Actions",
	    stack.GetNrOfUndoableActions() == 0);

	stack.PushActionAndExecute(pAction4);

	UnitTest::Assert("undo stack should contain 1 Actions",
	    stack.GetNrOfUndoableActions() == 1);
}

static void Test_ActionStack_ExecuteAndUndo()
{
	ActionStack stack;

	int dummyInt = 42;
	refcount_ptr<Action> pAction1 = new DummyAction(dummyInt);
	refcount_ptr<Action> pAction2 = new DummyAction(dummyInt);
	refcount_ptr<Action> pAction3 = new DummyAction(dummyInt);

	dummyInt = 0;
	UnitTest::Assert("A: dummyInt should be 0", dummyInt == 0);

	stack.PushActionAndExecute(pAction1);
	UnitTest::Assert("A: dummyInt should be 1", dummyInt == 1);

	stack.PushActionAndExecute(pAction2);
	UnitTest::Assert("A: dummyInt should be 2", dummyInt == 2);

	stack.PushActionAndExecute(pAction3);
	UnitTest::Assert("A: dummyInt should be 3", dummyInt == 3);

	stack.Undo();
	UnitTest::Assert("B: dummyInt should be 2", dummyInt == 2);

	stack.Undo();
	UnitTest::Assert("B: dummyInt should be 1", dummyInt == 1);

	stack.Undo();
	UnitTest::Assert("B: dummyInt should be 0", dummyInt == 0);

	UnitTest::Assert("Undo() should return false", !stack.Undo());
	UnitTest::Assert("C: dummyInt should be 0", dummyInt == 0);

	stack.Redo();
	UnitTest::Assert("C: dummyInt should be 1", dummyInt == 1);

	stack.Redo();
	UnitTest::Assert("C: dummyInt should be 2", dummyInt == 2);

	stack.Redo();
	UnitTest::Assert("C: dummyInt should be 3", dummyInt == 3);
	UnitTest::Assert("Redo should return false", !stack.Redo());
	UnitTest::Assert("D: dummyInt should be 3", dummyInt == 3);
}

UNITTESTS(ActionStack)
{
	// Tests for number of elements on the undo and redo stacks:
	UNITTEST(Test_ActionStack_IsInitiallyEmpty);
	UNITTEST(Test_ActionStack_PushAction);
	UNITTEST(Test_ActionStack_UndoRedo);
	UNITTEST(Test_ActionStack_PushShouldClearRedoStack);
	UNITTEST(Test_ActionStack_NonUndoableAction);

	// Tests for execution in forward and reverse order:
	UNITTEST(Test_ActionStack_ExecuteAndUndo);
}

#endif
