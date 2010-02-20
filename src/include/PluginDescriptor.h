#ifndef PLUGINDESCRIPTOR_H
#define	PLUGINDESCRIPTOR_H

/*
 *  Copyright (C) 2010  Anders Gavare.  All rights reserved.
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

#include "UnitTest.h"


/**
 * \brief A class used to parse plugin descriptor strings.
 *
 * A plugin descriptor is made up of three parts:
 * <ul>
 *	<li>component
 *	<li>plugin with optional arguments
 *	<li>name (usually a filename)
 * </ul>
 *
 * One or more of these must be present for the descriptor to be valid.
 * (The descriptor can be invalid for other reasons.)
 *
 * The component does not have to be an existing component name, the plugin
 * name and args are not checked by this class, and the name argument (usually
 * a filename) does not need to exist. It is up to the caller to make sense
 * of the component name, plugin name, and file name.
 */
class PluginDescriptor
	: public UnitTestable
{
public:
	/**
	 * \brief Constructor a plugin descriptor with a given string.
	 *
	 * @param str The string (e.g. "a:b(c):d").
	 */
	PluginDescriptor(const string& str);

	/**
	 * \brief Returns true is the plugin descriptor string was valid.
	 *
	 * @return true if the string could be parsed, false otherwise.
	 */
	bool IsValid() const
	{
		return m_valid;
	}

	/**
	 * \brief Returns the component name. May be an empty string.
	 *
	 * @return The component name.
	 */
	const string& ComponentName() const
	{
		return m_component;
	}

	/**
	 * \brief Returns the plugin name. May be an empty string.
	 *
	 * Note: This does not include any arguments to the plugin.
	 *
	 * @return The plugin name, without any arguments.
	 */
	const string& PluginName() const
	{
		return m_plugin;
	}

	/**
	 * \brief Returns the plugin arguments. May be an empty string.
	 *
	 * @return The plugin arguments.
	 */
	const string& PluginArguments() const
	{
		return m_pluginArguments;
	}

	/**
	 * \brief Returns the filename/name. May be an empty string.
	 *
	 * @return The filename/name.
	 */
	const string& Name() const
	{
		return m_name;
	}


	/********************************************************************/

	static void RunUnitTests(int& nSucceeded, int& nFailures);

private:
	/**
	 * \brief Parses the plugin descriptor string.
	 *
	 * m_valid is set to true or false, depending on success.
	 */
	void Parse();

	static vector<string> SplitDescriptorIntoVector(const string &str);
	static vector<string> GetPluginAndArguments(const string &str);

private:
	const string		m_str;

	bool			m_valid;
	string			m_component;
	string			m_plugin;
	string			m_pluginArguments;
	string			m_name;
};


#endif	// PLUGINDESCRIPTOR_H
