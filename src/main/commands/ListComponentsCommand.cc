/*
 *  Copyright (C) 2008-2010  Anders Gavare.  All rights reserved.
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

#include "commands/ListComponentsCommand.h"
#include "ComponentFactory.h"
#include "GXemul.h"


ListComponentsCommand::ListComponentsCommand()
	: Command("list-components", "")
{
}


ListComponentsCommand::~ListComponentsCommand()
{
}


bool ListComponentsCommand::Execute(GXemul& gxemul, const vector<string>& arguments)
{
	vector<string> allComponents =
	    ComponentFactory::GetAllComponentNames(false);

	size_t maxLen = 0;
	size_t i;
	for (i=0; i<allComponents.size(); i++)
		if (allComponents[i].length() > maxLen)
			maxLen = allComponents[i].length();

	for (i=0; i<allComponents.size(); i++) {
		const string& name = allComponents[i];
		refcount_ptr<Component> creatable =
		    ComponentFactory::CreateComponent(name);
		if (creatable.IsNULL())
			continue;

		stringstream msg;
		msg << "  " + name;
		for (size_t j = 0; j < 3 + maxLen - name.length(); j++)
			msg << " ";
		msg << ComponentFactory::GetAttribute(name, "description");
		msg << "\n";

		gxemul.GetUI()->ShowDebugMessage(msg.str());
	}

	return true;
}


string ListComponentsCommand::GetShortDescription() const
{
	return "Displays all available components.";
}


string ListComponentsCommand::GetLongDescription() const
{
	return "Displays a list of all available components.";
}

