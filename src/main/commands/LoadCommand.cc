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
 *
 *
 *  $Id: LoadCommand.cc,v 1.1 2008/01/12 08:29:56 debug Exp $
 */

#include "actions/LoadEmulationAction.h"
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

	refcount_ptr<Action> loadAction = new LoadEmulationAction(gxemul,
	    filename, path);
	gxemul.GetActionStack().PushActionAndExecute(loadAction);
}


string LoadCommand::GetShortDescription() const
{
	return _("Loads an emulation from a file.");
}


string LoadCommand::GetLongDescription() const
{
	return _("Loads an entire emulation setup, or a part of it, from a "
	    "file in the filesystem.\n"
	    "The filename may be omitted, if it is known from an"
	    " earlier save or load\n"
	    "command. If the component path is omitted, the loaded "
	    "emulation replaces any\n"
	    "currently loaded emulation. If a component path is specified, "
	    "the new\ncomponent is added as a child to that path.\n"
	    "\n"
	    "See also:  save    (to save an emulation to a file)\n");
}

