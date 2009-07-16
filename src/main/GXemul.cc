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
 *  This file contains two things:
 *
 *	1. Doxygen documentation for the general design concepts of the
 *	   emulator. The mainpage documentation written here ends up on
 *	   the "main page" in the generated HTML documentation.
 *
 *	2. The GXemul class implementation.
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
 * <a href="http://gxemul.sourceforge.net/">http://gxemul.sourceforge.net/</a>
 *
 * The main program creates a GXemul instance, and does one of two things:
 * <ul>
 *	<li>Starts without any template %machine. (<tt>-V</tt>)
 *	<li>Starts with a template %machine, and a list of filenames to load
 *		(usually a kernel binary to boot the emulated %machine).
 * </ul>
 * After letting the %GXemul instance load the files, GXemul::Run() is called.
 * This is the main loop. It doesn't really do much, it simply calls the UI's
 * main loop, i.e. ConsoleUI::MainLoop().
 *
 * Most of the source code in %GXemul centers around a few core concepts.
 * An overview of these concepts are given below. Anyone who wishes to
 * delve into the source code should be familiar with them.
 *
 *
 * \section concepts_sec Core concepts
 *
 * \subsection components_subsec Components
 *
 * The most important core concept in %GXemul is the Component. Examples of
 * components are processors, networks interface cards, video displays, RAM
 * %memory, busses, %interrupt controllers, and all other kinds of devices.
 *
 * Each component has a parent, so the full set of components in an emulation
 * are in fact a tree. A GXemul instance has one such tree. The root
 * component is a dummy container, which contains zero or more sub-components,
 * but it doesn't actually do anything else.
 *
 * <center><img src="../../model.png"></center>
 *
 * Starting from the root node, each component has a <i>path</i>, e.g.
 * <tt>root.machine1.mainbus0.ram0</tt> for the RAM component in machine1
 * in the example above.
 *
 * The state of each component is stored within that component. The state
 * consists of a number of variables (see StateVariable) such as strings,
 * integers, bools, and other more high-level types such as zero-filled %memory
 * arrays. Such %memory arrays are used e.g. by the RAMComponent to emulate
 * RAM, and can also be used to emulate video framebuffer %memory.
 *
 * Individual components are implemented in <tt>src/components/</tt>, with
 * header files in <tt>src/include/components/</tt>. The <tt>configure</tt>
 * script looks for the string <tt>COMPONENT(name)</tt> in the header files,
 * and automagically adds those to the list of components that will be
 * available at runtime. In addition, <tt>make documentation</tt> also builds
 * HTML pages with lists of available
 * <a href="../../components.html">components</a>, and as a special case,
 * a list of available
 * <a href="../../machines.html">template machines</a> (because they have
 * special meaning to the end-user).
 *
 * \subsection commandinterpreter_subsec Command interpreter
 *
 * A GXemul instance has a CommandInterpreter, which is the part that parses a
 * command line, figures out which Command is to be executed, and executes it.
 * The %CommandInterpreter can be given a complete command line as a string, or
 * it can be given one character (or keypress) at a time. In the later case,
 * the TAB key either completes the word currently being written, or writes
 * out a list of possible completions.
 *
 * When running <tt>gxemul</tt>, the %CommandInterpreter is the UI as seen
 * by the user. When running <tt>gxemul-gui</tt>, the %CommandInterpreter is
 * located in a window. The functionality should be the same in both cases.
 *
 * \subsection unittest_subsec Unit tests
 *
 * Wherever it makes sense, unit tests should be written to make sure
 * that the code is correct, and stays correct. The UnitTest class contains
 * static helper functions for writing unit tests, such as UnitTest::Assert.
 * To add unit tests to a class, the class should be UnitTestable, and in
 * particular, it should implement UnitTestable::RunUnitTests by using the
 * UNITTESTS(className) macro. Individual test cases are then called, as
 * static non-member functions, using the UNITTEST(testname) macro.
 *
 * Since test cases are non-member functions, they need to create instances
 * of the class they wish to test, and they can only call public member
 * functions on those objects, not private ones. Thus, the unit tests only
 * test the "public API" of all classes. (If the internal API needs to be
 * tested, then a workaround can be to add a ConsistencyCheck member function
 * which is public, but mentioning in the documentation for that function
 * that it is only meant for internal use and debugging.)
 *
 * Unit tests are normally executed by <tt>make test</tt>. This is implicitly
 * done when doing <tt>make install</tt> as well, for non-release builds.
 * It is recommended to run the <tt>configure</tt> script with the
 * <tt>--debug</tt> option during development; this enables Wu Yongwei's
 * new/debug %memory leak detector.
 */


/*****************************************************************************/

#include "ConsoleUI.h"
#include "NullUI.h"

#include "FileLoader.h"
#include "GXemul.h"
#include "components/DummyComponent.h"
#include "ComponentFactory.h"
#include "UnitTest.h"

#include <assert.h>
#include <string.h>
#include <unistd.h>

#include <fstream>
#include <iostream>


GXemul::GXemul()
	: m_quietMode(false)
	, m_ui(new NullUI(this))
	, m_commandInterpreter(this)
	, m_runState(NotRunning)
	, m_rootComponent(new DummyComponent)
{
	ClearEmulation();
}


void GXemul::ClearEmulation()
{
	if (GetRunState() == Running)
		SetRunState(NotRunning);

	m_step = 0;
	m_globalTime = 0.0;
	m_rootComponent = new DummyComponent;
	m_rootComponent->SetVariableValue("name", "\"root\"");
	m_emulationFileName = "";
	m_modelIsDirty = false;

	m_ui->UpdateUI();
}


bool GXemul::IsTemplateMachine(const string& templateName)
{
	if (!ComponentFactory::HasAttribute(templateName, "template"))
		return false;

	if (!ComponentFactory::HasAttribute(templateName, "machine"))
		return false;

	return true;
}


bool GXemul::CreateEmulationFromTemplateMachine(const string& templateName)
{
	if (!IsTemplateMachine(templateName)) {
		std::cerr << templateName << " is not a known template machine name.\n"
		    "Use gxemul -H to get a list of valid machine templates.\n";
		return false;
	}

	refcount_ptr<Component> machine =
	    ComponentFactory::CreateComponent(templateName);
	if (machine.IsNULL())
		return false;

	GetRootComponent()->AddChild(machine);
	return true;
}


void GXemul::ListTemplates()
{
	std::cout << "Available template machines:\n\n";

	vector<string> names = ComponentFactory::GetAllComponentNames(true);

	size_t maxNameLen = 0;
	for (size_t i=0; i<names.size(); ++i)
		if (names[i].length() > maxNameLen)
			maxNameLen = names[i].length();

	for (size_t i=0; i<names.size(); ++i) {
		string name = names[i];
		
		std::cout << "  " << name;
		for (size_t j=0; j<maxNameLen - name.length() + 6; ++j)
			std::cout << " ";

		std::cout << ComponentFactory::GetAttribute(
		    name, "description");
		std::cout << "\n";
	}

	std::cout << "\n";
}


void GXemul::DumpMachineAsHTML(const string& machineName)
{
	refcount_ptr<Component> component =
	    ComponentFactory::CreateComponent(machineName);

	if (!component.IsNULL() &&
	    component->GetChildren().size() != 0)
		std::cout << "<pre>" <<
		    component->GenerateTreeDump("", true, "../")
		    << "</pre>";
}


void GXemul::GenerateHTMLListOfComponents(bool machines)
{
	std::cout <<
		"Available " <<
		(machines? "template machines" : "components") << ":\n"
		"<p><table border=0>\n"
		"<tr>\n"
		" <td><b><u>" <<
		(machines? "Machine&nbsp;name" : "Component&nbsp;name") << ":"
		"</u></b>&nbsp;&nbsp;</td>\n"
#ifdef UNSTABLE_DEVEL
		" <td><b><u>Status:</u></b>&nbsp;&nbsp;</td>\n"
#endif
		" <td><b><u>Description:</u></b>&nbsp;&nbsp;</td>\n"
		" <td><b><u>Comments:</u></b>&nbsp;&nbsp;</td>\n"
		" <td><b><u>Contributors:</u></b>&nbsp;&nbsp;</td>\n"
		"</tr>\n";

	bool everyOther = false;
	vector<string> names = ComponentFactory::GetAllComponentNames(false);
	for (size_t i=0; i<names.size(); ++i) {
		const string& componentName = names[i];
		string treeDump;

		refcount_ptr<Component> creatable =
		    ComponentFactory::CreateComponent(componentName);
		if (creatable.IsNULL())
			continue;

		bool isTemplateMachine = !ComponentFactory::GetAttribute(
		    componentName, "machine").empty() &&
		    !ComponentFactory::GetAttribute(
		    componentName, "template").empty();

		if (machines) {
			if (!isTemplateMachine)
				continue;
		} else {
			// Other components:  Don't include template machines.
			if (isTemplateMachine)
			    	continue;
		}

		// Include an ASCII tree dump for template components that
		// have children:
		if (!ComponentFactory::GetAttribute(
		    componentName, "template").empty()) {
			refcount_ptr<Component> component =
			    ComponentFactory::CreateComponent(componentName);

			if (!component.IsNULL() &&
			    component->GetChildren().size() != 0)
				treeDump = "<pre>" +
				    component->GenerateTreeDump("", true)
				    + "</pre>";
		}

		// Some distance between table entries:
		std::cout <<
			"<tr>\n"
			" <td></td>"
			"</tr>\n";

		std::cout <<
			"<tr bgcolor=" <<
			(everyOther? "#f0f0f0" : "#e0e0e0") << ">\n";

		// Include a href link to a "full html page" for a component,
		// if it exists:
		std::ifstream documentationComponentFile((
		    "doc/components/component_"
		    + componentName + ".html").c_str());
		std::ifstream documentationMachineFile((
		    "doc/machines/machine_"
		    + componentName + ".html").c_str());

		if (documentationComponentFile.is_open())
			std::cout <<
				" <td valign=top>"
				"<a href=\"components/component_" <<
				componentName
				<< ".html\"><tt>" << componentName <<
				"</tt></a></td>\n";
		else if (documentationMachineFile.is_open())
			std::cout <<
				" <td valign=top>"
				"<a href=\"machines/machine_" <<
				componentName
				<< ".html\"><tt>" << componentName <<
				"</tt></a></td>\n";
		else
			std::cout <<
				" <td valign=top><tt>" << componentName
				<< "</tt></td>\n";

		std::cout <<
#ifdef UNSTABLE_DEVEL
			" <td valign=top>" << (ComponentFactory::HasAttribute(
				componentName, "stable")? "stable&nbsp;&nbsp;" :
				"experimental&nbsp;&nbsp;") << "</td>\n"
#endif
			" <td valign=top>" << ComponentFactory::GetAttribute(
				componentName, "description") <<
				treeDump << "</td>\n"
			" <td valign=top>" << ComponentFactory::GetAttribute(
				componentName, "comments") << "</td>\n"
			" <td valign=top>" << ComponentFactory::GetAttribute(
				componentName, "contributors") << "</td>\n"
			"</tr>\n";

		everyOther = !everyOther;
	}

	std::cout << "</table><p>\n";
}


bool GXemul::ParseFilenames(string templateMachine, int filenameCount, char *filenames[])
{
	bool optionsEnoughToStartRunning = false;

	if (templateMachine != "") {
		if (CreateEmulationFromTemplateMachine(templateMachine)) {
			// A template is now being used.
		} else {
			std::cerr << "Failed to create configuration from "
			    "template: " << templateMachine << "\n" <<
			    "Aborting." << "\n";
			return false;
		}
	}

	//  1. If a machine template has been selected, then treat the following
	//     arguments as files to load. (Legacy compatibility with previous
	//     versions of GXemul.)
	//  2. Otherwise, treat the argument as a configuration file.

	if (filenameCount > 0) {
		if (templateMachine != "") {
			// Machine template.

			// Load binaries into the main cpu.
			refcount_ptr<Component> main_cpu =
			    GetRootComponent()->
			    LookupPath("root.machine0.mainbus0.cpu0");
			// TODO: Don't hardcode the CPU path!

			while (filenameCount > 0) {
				FileLoader loader(filenames[0]);
				if (!loader.Load(main_cpu)) {
					std::cerr << "Failed to load "
					    "binary: " <<
					    filenames[0] << "\n" <<
					    "Aborting." << "\n";
					return false;
				}

				filenameCount --;
				filenames ++;
			}

			optionsEnoughToStartRunning = true;
		} else {
			// Config file.
			if (filenameCount == 1) {
				string configfileName = filenames[0];
				optionsEnoughToStartRunning = true;

				string cmd = "load " + configfileName;
				GetCommandInterpreter().RunCommand(cmd);

				if (GetRootComponent()->GetChildren().size()
				    == 0) {
					std::cerr << "Failed to load "
					    "configuration: " <<
					    configfileName << "\n" <<
					    "Aborting." << "\n";
					return false;
				}
			} else {
				std::cerr << "More than one configfile name "
				    "supplied on the command line?" << "\n" <<
				    "Aborting." << "\n";
				return false;
			}
		}
	}

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
			if (GetRunState() == Paused)
				return true;

			std::cerr << 
			    "No binary specified. Usually when starting up an emulation based on a template\n"
			    "machine, you need to supply one or more binaries. This could be an operating\n"
			    "system kernel, a ROM image, or something similar.\n"
			    "\n"
			    "You can also use the -V option to start in paused mode, and load binaries\n"
			    "interactively.\n"
			    "\n"
			    "(Run  gxemul -h  for more help on command line options.)\n";
			return false;
		}
		
		PrintUsage(false);
		return false;
	}
}


string GXemul::Version()
{
	stringstream ss;
	ss << "GXemul "
#ifdef VERSION
	    << VERSION
#else
	    << "(unknown version)"
#endif
	    << "      "COPYRIGHT_MSG"\n"SECONDARY_MSG;

	return ss.str();
}


void GXemul::PrintUsage(bool longUsage) const
{
	std::cout << Version() << "\n";

	if (!longUsage) {
		std::cout << "Insufficient command line arguments given to"
		    " start an emulation. You have\n"
		    "the following alternatives:\n" <<
		    "\n" <<
		    "  1. Run  gxemul  with the machine selection option "
		    "(-e), which creates\n"
		    "     a default emulation from a template machine.\n\n"
		    "  2. Run  gxemul  with a configuration file (.gxemul).\n"
		    "     This is useful for more complicated setups.\n\n"
		    "  3. Run  gxemul -V  with no other options, which causes"
		    " gxemul to be started\n"
		    "     with no emulation loaded at all.\n\n" <<
		    "\n" <<
		    "Run  gxemul -h  for help on command line options.\n\n";
		return;
	}

	std::cout << "Usage:" << "\n"
	    << "  gxemul [general options] [configfile]\n"
	    << "  gxemul [general options] [machine selection options] binary"
	    << " [...]\n\n";

	// When changing command line options, REMEMBER to keep the following
	// things in synch:
	//
	//	1. The help message.
	//	2. The option parsing in ParseOptions.
	//	3. The man page.

	std::cout <<
		"Machine selection options:\n"
		"  -e t     Starts an emulation based on template t."
			" (Use -H to get a list.)\n"
		// -E is deprecated.
		"\n"
		"General options:\n"
		"  -H       Displays a list of all machine"
			" templates (for use with -e).\n"
		"  -h       Displays this help message.\n"
		"  -q       Quiet mode. (Supresses startup banner and "
			"some debug output.)\n"
		"  -V       Starts in the Paused state. (Can also be used"
			" to start gxemul\n"
		"           without loading an emulation at all.)\n"
		// -W is undocumented. It is only used internally.
		"\n"
		"An emulation setup created by either supplying machine "
			"selection options\n"
		"directly on the command line, or by supplying a configuration"
			" file (with\n"
		"the .gxemul extension).\n"
		"\n";
}


int GXemul::Run()
{
	// Default to the console UI:
	m_ui = new ConsoleUI(this);

	// Once a GUI has been implemented, this is the
	// place to call its constructor. TODO

	m_ui->Initialize();
	
	if (!GetQuietMode())
		m_ui->ShowStartupBanner();

	try {
		m_ui->MainLoop();
	} catch (std::exception& ex) {
		stringstream ss;
		ss << "\n### FATAL ERROR ###\n\n" << ex.what() << "\n\n" <<
		    "If you are able to reproduce this crash, "
		    "please send detailed repro-steps to\n"
		    "the author, to the gxemul-devel mailing list, or"
		    " ask in #GXemul on the\n"
		    "FreeNode IRC network.\n";

		m_ui->FatalError(ss.str());

		// Release the UI:
		m_ui = NULL;

		return 1;
	}

	// Release the UI:
	m_ui = NULL;

	return 0;
}


bool GXemul::GetDirtyFlag() const
{
	return m_modelIsDirty;
}


void GXemul::SetDirtyFlag(bool dirtyFlag)
{
	m_modelIsDirty = dirtyFlag;
	m_ui->UpdateUI();
}


const string& GXemul::GetEmulationFilename() const
{
	return m_emulationFileName;
}


void GXemul::SetEmulationFilename(const string& filename)
{
	m_emulationFileName = filename;

	m_ui->UpdateUI();
}


CommandInterpreter& GXemul::GetCommandInterpreter()
{
	return m_commandInterpreter;
}


double GXemul::GetGlobalTime() const
{
	return m_globalTime;
}


uint64_t GXemul::GetStep() const
{
	return m_step;
}


void GXemul::SetGlobalTime(double globalTime)
{
	m_globalTime = globalTime;
}


UI* GXemul::GetUI()
{
	return m_ui;
}


refcount_ptr<Component> GXemul::GetRootComponent()
{
	return m_rootComponent;
}


void GXemul::SetRootComponent(refcount_ptr<Component> newRootComponent)
{
	assert(!newRootComponent.IsNULL());
	m_rootComponent = newRootComponent;

	m_ui->UpdateUI();
}


void GXemul::SetRunState(RunState newState)
{
	m_runState = newState;

	m_ui->UpdateUI();
}


GXemul::RunState GXemul::GetRunState() const
{
	return m_runState;
}


string GXemul::GetRunStateAsString() const
{
	switch (m_runState) {
	case NotRunning:
		return "Not running";
	case Paused:
		return "Paused";
	case Running:
		return "Running";
	case Quitting:
		return "Quitting";
	}

	return "Unknown RunState";
}


bool GXemul::GetQuietMode() const
{
	return m_quietMode;
}


void GXemul::SetQuietMode(bool quietMode)
{
	m_quietMode = quietMode;
}


void GXemul::ExecuteSteps(int nrOfSteps)
{
	// TODO
}


/*****************************************************************************/

#ifdef WITHUNITTESTS

static void Test_Construction()
{
	GXemul gxemul;
}

UNITTESTS(GXemul)
{
	UNITTEST(Test_Construction);
}


#endif

