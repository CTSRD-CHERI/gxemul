#ifndef GXEMUL_H
#define	GXEMUL_H

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

#include "misc.h"

#include "ActionStack.h"
#include "CommandInterpreter.h"
#include "Component.h"
#include "UI.h"


/**
 * \brief The main emulator class.
 *
 * Its main purpose is to run the GUI main loop, or the text terminal
 * main loop.
 *
 * A %GXemul instance basically has the following member variables:
 *
 * <ol>
 *	<li>a tree of components, which make up the full
 *		state of the current emulation setup
 *	<li>an ActionStack, for undo/redo functionality
 *	<li>a UI
 *	<li>a CommandInterpreter
 *	<li>a RunState
 * </ol>
 *
 * The implementation file, GXemul.cc, contains both the class implementation
 * and the main() function of the application. The %GXemul class is also
 * responsible for starting execution of unit tests.
 */
class GXemul
{
public:
	enum RunState
	{
		NotRunning,
		Paused,
		Running,
		Quitting
	};

public:
	/**
	 * \brief Creates a GXemul instance.
	 *
	 * @param bWithGUI      true if the GUI is to be used, false otherwise
	 */
	GXemul(bool bWithGUI);

	/**
	 * \brief Parses command line arguments.
	 *
	 * @param argc for parsing command line options
	 * @param argv for parsing command line options
	 * @return true if options were parsable, false if there was
	 *		some error.
	 */
	bool ParseOptions(int argc, char *argv[]);

	/**
	 * \brief Discards the current emulation, and starts anew with just
	 *	an empty root component.
	 */
	void ClearEmulation();

	/**
	 * \brief Runs GXemul's main loop.
	 *
	 * This can be either a GUI main loop, or
	 * a text terminal based main loop.
	 *
	 * @return Zero on success, non-zero on error.
	 */
	int Run();

	/**
	 * \brief Gets the current emulation setup's filename.
	 *
	 * @return The name of the file that is used for the current emulation
	 *	setup. If no filename is defined yet, this is an empty string.
	 */
	const string& GetEmulationFilename() const;

	/**
	 * \brief Sets the current emulation setup's filename.
	 *
	 * @param filename This is the name of the file that is used
	 *	for the current emulation setup.
	 */
	void SetEmulationFilename(const string& filename);

	/**
	 * \brief Gets a reference to the CommandInterpreter.
	 *
	 * @return A reference to the %GXemul instance' CommandInterpreter.
	 */
	CommandInterpreter& GetCommandInterpreter();

	/**
	 * \brief Gets a reference to the ActionStack.
	 *
	 * @return A reference to the %GXemul instance' ActionStack.
	 */
	ActionStack& GetActionStack();

	/**
	 * \brief Gets a pointer to the %GXemul instance' active UI.
	 *
	 * Note: May return NULL if no UI has been initialized.
	 *
	 * @return A pointer to the UI in use.
	 */
	UI* GetUI();

	/**
	 * \brief Gets a pointer to the root configuration component.
	 *
	 * @return A pointer to the root component. If no configuration tree
	 *	is loaded, then this is at least an empty dummy component.
	 *	(The return value is never NULL.)
	 */
	refcount_ptr<Component> GetRootComponent();

	/**
	 * \brief Sets the root component, discarding the previous one.
	 *
	 * This function should not be used to set the root component
	 * to NULL. Use ClearEmulation() instead.
	 *
	 * @param newRootComponent A reference counted pointer to the new
	 *	root component. It may not be a NULL pointer.
	 */
	void SetRootComponent(refcount_ptr<Component> newRootComponent);

	/**
	 * \brief Sets the RunState.
	 *
	 * @param newState The new RunState.
	 */
	void SetRunState(RunState newState);

	/**
	 * \brief Gets the current RunState.
	 *
	 * @return The current RunState.
	 */
	RunState GetRunState() const;

	/**
	 * \brief Gets the current RunState as a string.
	 *
	 * @return The current RunState, formatted as a string.
	 */
	string GetRunStateAsString() const;

	/**
	 * \brief Gets the current global time of the emulation.
	 *
	 * Note: This is not necessarily equal to real-world time.
	 *
	 * @return The time, in seconds, that the emulation has been executing.
	 */
	double GetGlobalTime() const;

	/**
	 * \brief Sets the current global time of the emulation.
	 *
	 * Note: This is not necessarily equal to real-world time.
	 *
	 * @param globalTime The time, in seconds.
	 */
	void SetGlobalTime(double globalTime);

	/**
	 * \brief Gets the current quiet mode setting.
	 *
	 * @return True if running in quiet mode, false for normal operation.
	 */
	bool GetQuietMode() const;

	/**
	 * \brief Sets whether or not to run in quiet mode.
	 *
	 * @param quietMode true to run in quiet mode, false otherwise.
	 */
	void SetQuietMode(bool quietMode);

	/**
	 * \brief Run the emulation for a while.
	 */
	void ExecuteCycles(double timeslice = 0.01);

private:
	/**
	 * \brief Creates an emulation setup from a template machine name.
	 *
	 * @param templateName The name of the template machine.
	 * @return True if the emulation was created, false otherwise.
	 */
	bool CreateEmulationFromTemplateMachine(const string& templateName);

	/**
	 * \brief Prints help message to std::cout.
	 *
	 * @param longUsage True if the long help message should be printed,
	 *		false to only print a short message.
	 */
	void PrintUsage(bool longUsage) const;

private:
	string			m_emulationFileName;
	RunState		m_runState;
	bool			m_bWithGUI;
	bool			m_bRunUnitTests;
	bool			m_quietMode;
	double			m_globalTime;
	refcount_ptr<UI>	m_ui;
	refcount_ptr<Component>	m_rootComponent;
	ActionStack		m_actionStack;
	CommandInterpreter	m_commandInterpreter;
};

#endif	// GXEMUL_H
