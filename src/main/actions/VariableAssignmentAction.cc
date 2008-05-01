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

#include "actions/VariableAssignmentAction.h"
#include "components/DummyComponent.h"
#include "EscapedString.h"
#include "GXemul.h"


VariableAssignmentAction::VariableAssignmentAction(
		GXemul& gxemul,
		const string& componentPath,
		const string& variableName,
		const string& expression)
	: Action("variable assignment")
	, m_gxemul(gxemul)
	, m_componentPath(componentPath)
	, m_variableName(variableName)
	, m_expression(expression)
{
}


VariableAssignmentAction::~VariableAssignmentAction()
{
}


void VariableAssignmentAction::Execute()
{
	refcount_ptr<Component> component = m_gxemul.GetRootComponent()->
	    LookupPath(m_componentPath);

	StateVariable* var = component->GetVariable(m_variableName);
	if (var == NULL) {
		m_gxemul.GetUI()->ShowDebugMessage(
		    _("Unknown variable? (Internal error.)\n"));
		throw std::exception();
	}

	m_oldValue = var->ToString();
	if (var->GetType() == StateVariable::String) {
		EscapedString escaped(m_oldValue);
		m_oldValue = escaped.Generate();
	}

	bool success = var->SetValue(m_expression);
	if (!success) {
		m_gxemul.GetUI()->ShowDebugMessage(
		    _("Assignment failed. (Wrong "
		    "type?)\n"));
	}

	m_oldDirtyFlag = m_gxemul.GetDirtyFlag();
	m_gxemul.SetDirtyFlag(true);
}


void VariableAssignmentAction::Undo()
{
	refcount_ptr<Component> component = m_gxemul.GetRootComponent()->
	    LookupPath(m_componentPath);

	StateVariable* var = component->GetVariable(m_variableName);

	bool success = var->SetValue(m_oldValue);
	if (!success) {
		m_gxemul.GetUI()->ShowDebugMessage(
		    _("Unable to Undo assignment. Internal error?\n"));
		throw std::exception();
	}

	m_gxemul.SetDirtyFlag(m_oldDirtyFlag);
}


/*****************************************************************************/


#ifndef WITHOUTUNITTESTS

static void Test_VariableAssignmentAction_WithUndoRedo()
{
//	GXemul gxemulDummy(false);

// TODO
}

UNITTESTS(VariableAssignmentAction)
{
	UNITTEST(Test_VariableAssignmentAction_WithUndoRedo);
}

#endif
