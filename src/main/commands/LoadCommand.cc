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

#include <fstream>
#include "commands/LoadCommand.h"
#include "GXemul.h"


LoadCommand::LoadCommand()
	: Command("load", "[filename [component-path]]")
{
}


LoadCommand::~LoadCommand()
{
}


static void ShowMsg(GXemul& gxemul, const string& msg)
{
	gxemul.GetUI()->ShowDebugMessage(msg);
}


void LoadCommand::Execute(GXemul& gxemul, const vector<string>& arguments)
{
	string filename = gxemul.GetEmulationFilename();
	string path = "";

	if (arguments.size() > 2) {
		ShowMsg(gxemul, "Too many arguments.\n");
		return;
	}

	if (arguments.size() > 0)
		filename = arguments[0];

	if (filename == "") {
		ShowMsg(gxemul, "No filename given.\n");
		return;
	}

	if (arguments.size() > 1)
		path = arguments[1];


	const string extension = ".gxemul";
	if (filename.length() < extension.length() || filename.substr(
	    filename.length() - extension.length()) != extension)
		ShowMsg(gxemul, "Warning: the name " + filename +
		    " does not have a .gxemul extension. Continuing anyway.\n");

	refcount_ptr<Component> whereToAddIt;
	if (path != "") {
		vector<string> matches = gxemul.GetRootComponent()->
		    FindPathByPartialMatch(path);
		if (matches.size() == 0) {
			ShowMsg(gxemul, path +
			    " is not a path to a known component.\n");
			return;
		}
		if (matches.size() > 1) {
			ShowMsg(gxemul, path +
			    " matches multiple components:\n");
			for (size_t i=0; i<matches.size(); i++)
				ShowMsg(gxemul, "  " + matches[i] + "\n");
			return;
		}

		whereToAddIt =
		    gxemul.GetRootComponent()->LookupPath(matches[0]);
		if (whereToAddIt.IsNULL()) {
			ShowMsg(gxemul, "Lookup of " + path + " failed.\n");
			return;
		}
	}
	
	if (whereToAddIt.IsNULL())
		gxemul.ClearEmulation();

	refcount_ptr<Component> component;

	// Load from the file
	std::ifstream file(filename.c_str());
	if (file.fail()) {
		ShowMsg(gxemul, "Unable to open " + filename +
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
		ShowMsg(gxemul, "Loading from " + filename + " failed; "
		    "no component found?\n");
		return;
	}

	if (whereToAddIt.IsNULL()) {
		const StateVariable* name = component->GetVariable("name");
		if (name == NULL || name->ToString() != "root")
			gxemul.GetRootComponent()->AddChild(component);
		else
			gxemul.SetRootComponent(component);

		gxemul.SetEmulationFilename(filename);
		gxemul.SetDirtyFlag(false);
	} else {
		whereToAddIt->AddChild(component);
		gxemul.SetDirtyFlag(true);
	}
}


string LoadCommand::GetShortDescription() const
{
	return "Loads an emulation from a file.";
}


string LoadCommand::GetLongDescription() const
{
	return "Loads an entire emulation setup, or a part of it, from a "
	    "file in the filesystem.\n"
	    "The filename may be omitted, if it is known from an"
	    " earlier save or load\n"
	    "command. If the component path is omitted, the loaded "
	    "emulation replaces any\n"
	    "currently loaded emulation. If a component path is specified, "
	    "the new\ncomponent is added as a child to that path.\n"
	    "\n"
	    "See also:  save    (to save an emulation to a file)\n";
}

