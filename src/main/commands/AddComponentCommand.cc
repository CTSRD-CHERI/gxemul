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

#include "actions/AddComponentAction.h"
#include "commands/AddComponentCommand.h"
#include "ComponentFactory.h"
#include "GXemul.h"


AddComponentCommand::AddComponentCommand()
	: Command("add", "component-name [path]")
{
}


AddComponentCommand::~AddComponentCommand()
{
}


static void ShowMsg(GXemul& gxemul, const string& msg)
{
	gxemul.GetUI()->ShowDebugMessage(msg);
}


void AddComponentCommand::Execute(GXemul& gxemul,
    const vector<string>& arguments)
{
	if (arguments.size() < 1) {
		ShowMsg(gxemul, "No component-name given.\n");
		return;
	}

	if (arguments.size() > 2) {
		ShowMsg(gxemul, "Too many arguments.\n");
		return;
	}

	string componentName = arguments[0];

	refcount_ptr<Component> componentToAdd =
	    ComponentFactory::CreateComponent(componentName);

	if (componentToAdd.IsNULL()) {
		ShowMsg(gxemul, componentName + ": unknown component.\n");
		return;
	}

	string path = "root";
	if (arguments.size() == 2)
		path = arguments[1];

	vector<string> matches = gxemul.GetRootComponent()->
	    FindPathByPartialMatch(path);
	if (matches.size() == 0) {
		ShowMsg(gxemul, path+" is not a path to a known component.\n");
		return;
	}
	if (matches.size() > 1) {
		ShowMsg(gxemul, path+" matches multiple components:\n");
		for (size_t i=0; i<matches.size(); i++)
			ShowMsg(gxemul, "  " + matches[i] + "\n");
		return;
	}

	refcount_ptr<Component> whereToAddIt =
	    gxemul.GetRootComponent()->LookupPath(matches[0]);
	if (whereToAddIt.IsNULL()) {
		ShowMsg(gxemul, "Lookup of " + path + " failed.\n");
		return;
	}

	refcount_ptr<Action> addComponentAction =
	    new AddComponentAction(componentToAdd, whereToAddIt);

	gxemul.GetActionStack().PushActionAndExecute(addComponentAction);
}


string AddComponentCommand::GetShortDescription() const
{
	return _("Adds a component to the emulation.");
}


string AddComponentCommand::GetLongDescription() const
{
	return _("Adds a component (given by the component-name) to the "
	    "current emulation setup.\n"
	    "If path is omitted, the component is added at the root of"
	    " the component tree.\n"
	    "\n"
	    "See also:  remove  (to remove a component from the emulation)\n"
	    "           tree    (to inspect the current emulation setup)\n");
}

