/*
 *  Copyright (C) 2003-2009  Anders Gavare.  All rights reserved.
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
 *  This file contains three things:
 *
 *	1. Doxygen documentation for the general design concepts of the
 *	   emulator. The mainpage documentation written here ends up on
 *	   the "main page" in the generated HTML documentation.
 *
 *	2. The GXemul class implementation.
 *
 *	3. The main() entry point.
 */


/*! \mainpage Source code documentation
 *
 * \section intro_sec Introduction
 *
 * This is the automatically generated Doxygen documentation for %GXemul,
 * built from comments throughout the source code.
 *
 * See the <a href="../../index.html">main documentation</a> for more
 * information about this version of %GXemul.
 *
 * See GXemul's home page for more information about %GXemul in general:
 * <a href="http://gavare.se/gxemul/">http://gavare.se/gxemul/</a>
 *
 * <b>TODO</b>
 */


/*****************************************************************************/

#include "GXemul.h"
#include "UnitTest.h"

#include <iostream>


/*  main.c:  */
extern "C" int old_main(int argc, char *argv[]);


#ifdef I18N
#include <libintl.h>
#define I18N_PACKAGE_NAME	"gxemul"
#endif


/// For command line parsing using getopt().
extern char *optarg;

/// For command line parsing using getopt().
extern int optind;


GXemul::GXemul()
	: m_runState(NotRunning)
{
}


void GXemul::SetRunState(RunState newState)
{
	m_runState = newState;
}


GXemul::RunState GXemul::GetRunState() const
{
	return m_runState;
}


bool GXemul::ParseOptions(int argc, char *argv[])
{
	string templateMachine = "";
	bool optionsEnoughToStartRunning = false;
	int ch;

	// REMEMBER to keep the following things in synch:
	//	1. The help message.
	//	2. The option parsing in ParseOptions.
	//	3. The man page.

	const char *opts = "Ee:hVW:";

	while ((ch = getopt(argc, argv, opts)) != -1) {
		switch (ch) {

		case 'E':
			std::cerr << _("The -E option is deprecated. "
			    "Use -e instead.\n");
			return false;

		case 'e':
			templateMachine = optarg;
			break;

		case 'h':
			PrintUsage(true);
			return false;

		case 'V':
			SetRunState(Paused);

			// Note: -V allows the user to start the console version
			// of gxemul without an initial emulation setup.
			optionsEnoughToStartRunning = true;
			break;

		case 'W':
			if (string(optarg) == "unittest") {
				exit(UnitTest::RunTests());
			} else {
				PrintUsage(false);
				return false;
			}
			break;

		default:
			std::cout << _("\n"
				"It is possible that you are attempting to use"
				    " an option which was available\n"
				"in older versions of GXemul, but has not been"
				    " reimplemented in GXemul 0.5.x.\n"
				"Please see the man page or the  gxemul -h "
				    " help message for available options.\n\n");
			PrintUsage(false);
			return false;
		}
	}

#if 0
	if (templateMachine != "") {
		if (CreateEmulationFromTemplateMachine(templateMachine)) {
			// A template is now being used.
		} else {
			std::cerr << _("Failed to create configuration from "
			    "template: ") << templateMachine << "\n" <<
			    _("Aborting.") << "\n";
			return false;
		}
	}

	// Any remaining arguments?
	//  1. If a machine template has been selected, then treat the following
	//     arguments as files to load. (Legacy compatibility with previous
	//     versions of GXemul.)
	//  2. Otherwise, treat the argument as a configuration file.
	argc -= optind;
	argv += optind;

	if (argc > 0) {
		if (templateMachine != "") {
			// Machine template.

			// Load binaries into the main cpu.
			refcount_ptr<Component> main_cpu =
			    GetRootComponent()->
			    LookupPath("root.machine0.mainbus0.cpu0");
			// TODO: Don't hardcode the CPU path!

			while (argc > 0) {
				FileLoader loader(argv[0]);
				if (!loader.Load(main_cpu)) {
					std::cerr << _("Failed to load "
					    "binary: ") <<
					    argv[0] << "\n" <<
					    _("Aborting.") << "\n";
					return false;
				}

				argc --;
				argv ++;
			}

			optionsEnoughToStartRunning = true;
		} else {
			// Config file.
			if (argc == 1) {
				string configfileName = argv[0];
				optionsEnoughToStartRunning = true;

				refcount_ptr<Action> loadAction =
				    new LoadEmulationAction(*this,
				    configfileName, "");
				GetActionStack().PushActionAndExecute(
				    loadAction);

				if (GetRootComponent()->GetChildren().size()
				    == 0) {
					std::cerr << _("Failed to load "
					    "configuration: ") <<
					    configfileName << "\n" <<
					    _("Aborting.") << "\n";
					return false;
				}
			} else {
				std::cerr << _("More than one configfile name "
				    "supplied on the command line?") << "\n" <<
				    _("Aborting.") << "\n";
				return false;
			}
		}
	}
#endif

	if (optionsEnoughToStartRunning) {
		// Automatically start running, if the -V option was not
		// supplied. In the future, if a GUI is implemented, starting
		// the GUI without any configuration should work like the
		// -V option.
		if (GetRunState() == NotRunning)
			SetRunState(Running);

		return true;
	} else {
		if (templateMachine != "") {
			std::cerr << _("No binary specified. Aborting.\n");
			return false;
		}
		
		PrintUsage(false);
		return false;
	}
}


void GXemul::PrintUsage(bool longUsage) const
{
	std::cout << "GXemul "VERSION"     "COPYRIGHT_MSG"\n"SECONDARY_MSG"\n";

	if (!longUsage) {
		std::cout << _("Insufficient command line arguments given to"
		    " start an emulation. You have\n"
		    "the following alternatives:\n") <<
		    "\n" <<
		    _("  1. Run  gxemul  with machine selection options "
		    "(-e), which creates\n"
		    "     a default emulation from a template machine.\n\n"
		    "  2. Run  gxemul  with a configuration file (.gxemul).\n"
		    "     This is useful for more complicated setups.\n\n"
		    "  3. Run  gxemul -V  with no other options, which causes"
		    " gxemul to be started\n"
		    "     with no emulation loaded at all.\n\n") <<
		    "\n" <<
		    _("Run  gxemul -h  for help on command line options.\n\n");
		return;
	}

	std::cout <<
		_("Usage: gxemul [general options] [configfile]\n"
		"       gxemul [general options] [machine selection"
			" options] binary [...]\n");

	// When changing command line options, REMEMBER to keep the following
	// things in synch:
	//
	//	1. The help message.
	//	2. The option parsing in ParseOptions.
	//	3. The man page.

	std::cout <<
		_("Machine selection options:\n"
		"  -e t       Starts an emulation based on template t."
			" (Use -H to get a list.)\n"
		// -E is deprecated.
		"\n"
		"General options:\n"
		"  -H         Displays a list of all machine"
			" templates (for use with -e).\n"
		"  -h         Displays this help message.\n"
		"  -q         Quiet mode. (Supresses startup banner and "
			"some debug output.)\n"
		"  -V         Starts in the Paused state. (Can also be used"
			" to start gxemul\n"
		"             without loading an emulation at all.)\n"
		// -W is undocumented. It is only used internally.
		"\n"
		"An emulation setup is either created by supplying machine "
			"selection options\n"
		"directly on the command line, or by supplying a configuration"
			" file (with\n"
		"the .gxemul extension).\n"
		"\n");
}


int GXemul::Run()
{
	// TODO
	return 0;
}


/**
 * \brief Program entry point.
 */
int main(int argc, char *argv[])
{
	const char *progname = argv[0];

#ifdef I18N
	// Initialize gettext, for Internationalization support:
	bindtextdomain(I18N_PACKAGE_NAME, NULL);
	bind_textdomain_codeset(I18N_PACKAGE_NAME, "UTF-8");
	textdomain(I18N_PACKAGE_NAME);
#endif

	GXemul gxemul;

	if (!gxemul.ParseOptions(argc, argv))
		return 0;

	return gxemul.Run();
}

