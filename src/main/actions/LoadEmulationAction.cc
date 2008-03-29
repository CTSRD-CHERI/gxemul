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
#include <stdlib.h>
#include <string.h>

#include <fstream>

#include "actions/LoadEmulationAction.h"
#include "components/DummyComponent.h"
#include "GXemul.h"


LoadEmulationAction::LoadEmulationAction(GXemul& gxemul, const string& filename,
		const string& path)
	: Action("load emulation")
	, m_gxemul(gxemul)
	, m_filename(filename)
	, m_path(path)
{
}


LoadEmulationAction::~LoadEmulationAction()
{
}


static void ShowMsg(GXemul& gxemul, const string& msg)
{
	gxemul.GetUI()->ShowDebugMessage(msg);
}


void LoadEmulationAction::Execute()
{
	const string extension = ".gxemul";
	if (m_filename.length() < extension.length() || m_filename.substr(
	    m_filename.length() - extension.length()) != extension)
		ShowMsg(m_gxemul, "Warning: the name " + m_filename +
		    " does not have a .gxemul extension. Continuing anyway.\n");

	refcount_ptr<Component> whereToAddIt;
	if (m_path != "") {
		vector<string> matches = m_gxemul.GetRootComponent()->
		    FindPathByPartialMatch(m_path);
		if (matches.size() == 0) {
			ShowMsg(m_gxemul, m_path +
			    " is not a path to a known component.\n");
			return;
		}
		if (matches.size() > 1) {
			ShowMsg(m_gxemul, m_path +
			    " matches multiple components:\n");
			for (size_t i=0; i<matches.size(); i++)
				ShowMsg(m_gxemul, "  " + matches[i] + "\n");
			return;
		}

		whereToAddIt =
		    m_gxemul.GetRootComponent()->LookupPath(matches[0]);
		if (whereToAddIt.IsNULL()) {
			ShowMsg(m_gxemul, "Lookup of " + m_path + " failed.\n");
			return;
		}
	}
	
	if (whereToAddIt.IsNULL())
		m_gxemul.ClearEmulation();

	refcount_ptr<Component> component;

	// Load from the file
	std::ifstream file(m_filename.c_str());
	if (file.fail()) {
		ShowMsg(m_gxemul, "Unable to open " + m_filename +
		    " for reading.\n");
		return;
	}

	// Figure out the file's size:
	file.seekg(0, std::ios::end);
	std::streampos fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	// Read the entire file into a string.
	// TODO: This is wasteful, of course. It actually takes twice the
	// size of the file, since the string constructor generates a _copy_.
	// But string takes care of unicode and such (if compiled as ustring).
	char* buf = (char*) malloc(fileSize);
	if (buf == NULL)
		throw std::exception();

	memset(buf, 0, fileSize);
	file.read(buf, fileSize);
	string str(buf, fileSize);
	free(buf);
	file.close();

	size_t strPos = 0;
	component = Component::Deserialize(str, strPos);

	if (component.IsNULL()) {
		ShowMsg(m_gxemul, "Loading from " + m_filename + " failed; "
		    "no component found?\n");
		return;
	}

	if (whereToAddIt.IsNULL()) {
		const StateVariable* name = component->GetVariable("name");
		if (name == NULL || name->ToString() != "root")
			m_gxemul.GetRootComponent()->AddChild(component);
		else
			m_gxemul.SetRootComponent(component);

		m_gxemul.SetEmulationFilename(m_filename);
		m_gxemul.GetActionStack().Clear();
	} else {
		whereToAddIt->AddChild(component);
		
		m_addedComponent = component;
		m_whereItWasAdded = whereToAddIt;
	}
}


void LoadEmulationAction::Undo()
{
	if (!m_whereItWasAdded.IsNULL()) {
		m_whereItWasAdded->RemoveChild(m_addedComponent);
	} else {
		assert(false);
	}
}


/*****************************************************************************/


#ifndef WITHOUTUNITTESTS

static void Test_LoadEmulationAction_ReplaceRoot()
{
	GXemul gxemul(false);

	refcount_ptr<Component> dummyComponentA = new DummyComponent;
	refcount_ptr<Component> dummyComponentB = new DummyComponent;
	dummyComponentA->SetVariableValue("x", "123");
	dummyComponentB->SetVariableValue("x", "456");
	gxemul.GetRootComponent()->AddChild(dummyComponentA);
	gxemul.GetRootComponent()->AddChild(dummyComponentB);
	UnitTest::Assert("the root component should have children",
	    gxemul.GetRootComponent()->GetChildren().size(), 2);
	UnitTest::Assert("the dummy1 component should have no children",
	    dummyComponentB->GetChildren().size(), 0);

	UnitTest::Assert("the gxemul instance should have no filename",
	    gxemul.GetEmulationFilename(), "");

	ActionStack& actionStack = gxemul.GetActionStack();

	// Generate temporary file:
	string tmpFilename = "/tmp/hello.gxemul";
	std::ofstream file(tmpFilename.c_str());
	file << "component dummy {\n"
		" string name \"root\"\n"
		" component dummy {\n"
		"  string name \"apa\"\n"
		"  string x \"y\"\n"
		"} }\n";
	file.close();

	// Load with no component target path.

	refcount_ptr<Action> loadAction = new LoadEmulationAction(gxemul,
	    tmpFilename, "");
	actionStack.PushActionAndExecute(loadAction);

	UnitTest::Assert("the root component should have 1 child",
	    gxemul.GetRootComponent()->GetChildren().size(), 1);
	UnitTest::Assert("the apa component should have 0 children",
	    gxemul.GetRootComponent()->GetChildren()[0]->GetChildren().size(),
	    0);

	UnitTest::Assert("the dummy1 component (disconnected) should have"
		" no children",
	    dummyComponentB->GetChildren().size(), 0);
	UnitTest::Assert("gxemul instance should have a filename",
	    gxemul.GetEmulationFilename(), tmpFilename);
	UnitTest::Assert("undo should not be possible",
	    actionStack.IsUndoPossible() == false);
}

static void Test_LoadEmulationAction_ReplaceRootNoRootInFile()
{
	GXemul gxemul(false);

	refcount_ptr<Component> dummyComponentA = new DummyComponent;
	refcount_ptr<Component> dummyComponentB = new DummyComponent;
	dummyComponentA->SetVariableValue("x", "123");
	dummyComponentB->SetVariableValue("x", "456");
	gxemul.GetRootComponent()->AddChild(dummyComponentA);
	gxemul.GetRootComponent()->AddChild(dummyComponentB);
	UnitTest::Assert("the root component should have children",
	    gxemul.GetRootComponent()->GetChildren().size(), 2);
	UnitTest::Assert("the dummy1 component should have no children",
	    dummyComponentB->GetChildren().size(), 0);

	UnitTest::Assert("the gxemul instance should have no filename",
	    gxemul.GetEmulationFilename(), "");

	ActionStack& actionStack = gxemul.GetActionStack();

	// Generate temporary file:
	string tmpFilename = "/tmp/hello.gxemul";
	std::ofstream file(tmpFilename.c_str());
	file << "component dummy {\n"
		"  string name \"apa\"\n"
		"  string x \"y\"\n"
		"}\n";
	file.close();

	// Load with no component target path.
	// (The component in the file will be the new root, except that
	// since the name was not root, a new "root" will be created first,
	// and then the stuff in the file will be added to that.)

	refcount_ptr<Action> loadAction = new LoadEmulationAction(gxemul,
	    tmpFilename, "");
	actionStack.PushActionAndExecute(loadAction);

	UnitTest::Assert("the root component should have 1 child",
	    gxemul.GetRootComponent()->GetChildren().size(), 1);
	UnitTest::Assert("the apa component should have 0 children",
	    gxemul.GetRootComponent()->GetChildren()[0]->GetChildren().size(),
	    0);

	UnitTest::Assert("the dummy1 component (disconnected) should have"
		" no children",
	    dummyComponentB->GetChildren().size(), 0);
	UnitTest::Assert("gxemul instance should have a filename",
	    gxemul.GetEmulationFilename(), tmpFilename);
	UnitTest::Assert("undo should not be possible",
	    actionStack.IsUndoPossible() == false);
}

static void Test_LoadEmulationAction_WithComponentTargetPath()
{
	GXemul gxemul(false);

	refcount_ptr<Component> dummyComponentA = new DummyComponent;
	refcount_ptr<Component> dummyComponentB = new DummyComponent;
	dummyComponentA->SetVariableValue("x", "123");
	dummyComponentB->SetVariableValue("x", "456");
	gxemul.GetRootComponent()->AddChild(dummyComponentA);
	gxemul.GetRootComponent()->AddChild(dummyComponentB);
	UnitTest::Assert("the root component should have children",
	    gxemul.GetRootComponent()->GetChildren().size(), 2);
	UnitTest::Assert("the dummy1 component should have no children",
	    dummyComponentB->GetChildren().size(), 0);

	UnitTest::Assert("the gxemul instance should have no filename",
	    gxemul.GetEmulationFilename(), "");

	ActionStack& actionStack = gxemul.GetActionStack();

	// Generate temporary file:
	string tmpFilename = "/tmp/hello.gxemul";
	std::ofstream file(tmpFilename.c_str());
	file << "component dummy {\n"
		"string name \"root\"\n"
		"component dummy {\n"
		"  string name \"apa\"\n"
		"  string x \"y\"\n"
		"} }\n";
	file.close();

	// Load with component target path:

	refcount_ptr<Action> loadAction = new LoadEmulationAction(gxemul,
	    tmpFilename, "root.dummy1");
	actionStack.PushActionAndExecute(loadAction);

	UnitTest::Assert("the root component should still have 2 children",
	    gxemul.GetRootComponent()->GetChildren().size(), 2);
	UnitTest::Assert("the dummy1 component should have a child",
	    dummyComponentB->GetChildren().size(), 1);
	UnitTest::Assert("gxemul instance should still have no filename",
	    gxemul.GetEmulationFilename(), "");
	UnitTest::Assert("undo should be possible",
	    actionStack.IsUndoPossible() == true);

	actionStack.Undo();

	UnitTest::Assert("the root component should still have children",
	    gxemul.GetRootComponent()->GetChildren().size(), 2);
	UnitTest::Assert("the dummy1 component should again have no children",
	    dummyComponentB->GetChildren().size(), 0);
}

UNITTESTS(LoadEmulationAction)
{
	UNITTEST(Test_LoadEmulationAction_ReplaceRoot);
	UNITTEST(Test_LoadEmulationAction_ReplaceRootNoRootInFile);
	UNITTEST(Test_LoadEmulationAction_WithComponentTargetPath);
}

#endif
