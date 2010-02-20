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

#include "PluginDescriptor.h"
#include "StringHelper.h"


PluginDescriptor::PluginDescriptor(const string& str)
	: m_str(str)
	, m_valid(false)
{
	Parse();
}


void PluginDescriptor::Parse()
{
	// First split by colon, but ignore any colons within the plugin
	// arguments, or if escaped by quotes.
	vector<string> vec = SplitDescriptorIntoVector(m_str);

	// No string at all, then we failed.
	if (vec.size() == 0)
		return;

	// A single string? Then this is a file name. (A weird case is if e.g.
	// m_str is "hello()", then this is interpreted as a filename, i.e.
	// to use a plugin with no filename, a component name _must_ currently
	// be supplied.)
	if (vec.size() == 1) {
		m_name = vec[0];

		// The name may not begin or end with space:
		if (m_name[0] == ' ' || m_name[m_name.length()-1] == ' ')
			return;

		m_valid = true;
		return;
	}

	// Too many parts? Then it is not a valid descriptor.
	if (vec.size() > 3)
		return;

	// We have either 2 or 3 parts now.

	// TODO: Refactor out common parts of the code below.

	// TODO 2: Return meaningful error message regarding what went wrong
	//         during parsing, instead of just failing silently.

	if (vec.size() == 3) {
		// If we have 3 parts, they must be:
		// component:plugin(args):name

		m_component = vec[0];

		// The component name may not be empty.
		if (m_component == "")
			return;

		// The component name may not begin or end with space:
		if (m_component[0] == ' ' || m_component[m_component.length()-1] == ' ')
			return;

		// The component may not have () on it.
		if (m_component.find('(') != string::npos)
			return;

		vector<string> pluginParts = GetPluginAndArguments(vec[1]);
		if (pluginParts.size() != 2)
			return;

		if (pluginParts[0].length() == 0)
			return;

		m_plugin = pluginParts[0];
		m_pluginArguments = pluginParts[1];

		// The plugin name may not begin or end with space:
		if (m_plugin[0] == ' ' || m_plugin[m_plugin.length()-1] == ' ')
			return;

		m_name = vec[2];

		// The name may not be empty.
		if (m_name == "")
			return;

		// The name may not begin or end with space:
		if (m_name[0] == ' ' || m_name[m_name.length()-1] == ' ')
			return;

		// The name may not have () on it. (For now.)
		if (m_name.find('(') != string::npos)
			return;

		m_valid = true;
		return;
	}

	// If we are here, we have 2 parts. They must be one of these:

	//  a) component:plugin(args)
	if (vec[1].find('(') != string::npos) {
		m_component = vec[0];

		// The component name may not be empty.
		if (m_component == "")
			return;

		// The component name may not begin or end with space:
		if (m_component[0] == ' ' || m_component[m_component.length()-1] == ' ')
			return;

		// The component may not have () on it.
		if (m_component.find('(') != string::npos)
			return;

		vector<string> pluginParts = GetPluginAndArguments(vec[1]);
		if (pluginParts.size() != 2)
			return;

		if (pluginParts[0].length() == 0)
			return;

		m_plugin = pluginParts[0];
		m_pluginArguments = pluginParts[1];

		// The plugin name may not begin or end with space:
		if (m_plugin[0] == ' ' || m_plugin[m_plugin.length()-1] == ' ')
			return;

		m_valid = true;
		return;
	}

	//  b) plugin(args):name
	if (vec[0].find('(') != string::npos) {
		vector<string> pluginParts = GetPluginAndArguments(vec[0]);
		if (pluginParts.size() != 2)
			return;

		if (pluginParts[0].length() == 0)
			return;

		m_plugin = pluginParts[0];
		m_pluginArguments = pluginParts[1];

		// The plugin name may not begin or end with space:
		if (m_plugin[0] == ' ' || m_plugin[m_plugin.length()-1] == ' ')
			return;

		m_name = vec[1];

		// The name may not be empty.
		if (m_name == "")
			return;

		// The name may not begin or end with space:
		if (m_name[0] == ' ' || m_name[m_name.length()-1] == ' ')
			return;

		m_valid = true;
		return;
	}

	//  c) component:name
	m_component = vec[0];
	m_name = vec[1];

	// The component name may not be empty.
	if (m_component == "")
		return;

	// The name may not be empty.
	if (m_name == "")
		return;

	// The component name may not begin or end with space:
	if (m_component[0] == ' ' || m_component[m_component.length()-1] == ' ')
		return;

	// The name may not begin or end with space:
	if (m_name[0] == ' ' || m_name[m_name.length()-1] == ' ')
		return;

	// The name may not have () on it. (For now.)
	if (m_name.find('(') != string::npos)
		return;

	m_valid = true;
}


vector<string> PluginDescriptor::SplitDescriptorIntoVector(const string &str)
{
	// This is slow and hackish, but works.
	vector<string> strings;
	string word;
	bool lastWasSplitter = false;
	int parenthesisLevel = 0;
	bool withinQuotes = false;

	for (size_t i=0, n=str.length(); i<n; i++) {
		char ch = str[i];

		if (!withinQuotes) {
			if (ch == '(')
				parenthesisLevel ++;
			if (ch == ')' && parenthesisLevel > 0)
				parenthesisLevel --;
		}

		if (ch == '"')
			withinQuotes = !withinQuotes;

		if (ch == ':' && !withinQuotes && parenthesisLevel == 0) {
			strings.push_back(word);
			word = "";
			lastWasSplitter = true;
		} else {
			word += ch;
			lastWasSplitter = false;
		}
	}

	if (word != "" || lastWasSplitter)
		strings.push_back(word);

	return strings;
}


vector<string> PluginDescriptor::GetPluginAndArguments(const string &str)
{
	vector<string> strings;

	// Valid examples:   a(b)  a(b(c))  a(")")
	// Invalid examples: a     ()       a(b)(c)    a(b   a(b")

	if (str.length() == 0 || str[0] == '(')
		return strings;

	// Find first '(':
	size_t pos = str.find('(');
	strings.push_back(str.substr(0, pos));

	string word;
	int parenthesisLevel = 0;
	bool withinQuotes = false;
	bool closed = false;

	for (size_t i=pos, n=str.length(); i<n; i++) {
		char ch = str[i];

		if (!withinQuotes) {
			if (ch == '(') {
				if (closed)
					return strings;
					
				parenthesisLevel ++;
			}
			
			if (ch == ')' && parenthesisLevel > 0) {
				parenthesisLevel --;
				if (parenthesisLevel == 0)
					closed = true;
			}
		}

		if (ch == '"')
			withinQuotes = !withinQuotes;

		word += ch;
	}

	if (!withinQuotes && closed && word.length() >= 2
	    && word[0] == '(' && word[word.length()-1] == ')')
		strings.push_back(word.substr(1, word.length() - 2));

	return strings;
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

static void Test_PluginDescriptor_Simple()
{
	PluginDescriptor pd1("a:b(c):d");

	UnitTest::Assert("pd1 validness", pd1.IsValid());
	UnitTest::Assert("component part", pd1.ComponentName(), "a");
	UnitTest::Assert("plugin name part", pd1.PluginName(), "b");
	UnitTest::Assert("plugin arguments part", pd1.PluginArguments(), "c");
	UnitTest::Assert("name part", pd1.Name(), "d");
}

static void Test_PluginDescriptor_MissingComponent()
{
	PluginDescriptor pd1("b(c):d");

	UnitTest::Assert("pd1 validness", pd1.IsValid());
	UnitTest::Assert("component part", pd1.ComponentName(), "");
	UnitTest::Assert("plugin name part", pd1.PluginName(), "b");
	UnitTest::Assert("plugin arguments part", pd1.PluginArguments(), "c");
	UnitTest::Assert("name part", pd1.Name(), "d");
}

static void Test_PluginDescriptor_MissingPluginArguments()
{
	PluginDescriptor pd1("a:b():d");

	UnitTest::Assert("pd1 validness", pd1.IsValid());
	UnitTest::Assert("component part", pd1.ComponentName(), "a");
	UnitTest::Assert("plugin name part", pd1.PluginName(), "b");
	UnitTest::Assert("plugin arguments part", pd1.PluginArguments(), "");
	UnitTest::Assert("name part", pd1.Name(), "d");
}

static void Test_PluginDescriptor_MissingPlugin()
{
	PluginDescriptor pd1("a:d");

	UnitTest::Assert("pd1 validness", pd1.IsValid());
	UnitTest::Assert("component part", pd1.ComponentName(), "a");
	UnitTest::Assert("plugin name part", pd1.PluginName(), "");
	UnitTest::Assert("plugin arguments part", pd1.PluginArguments(), "");
	UnitTest::Assert("name part", pd1.Name(), "d");
}

static void Test_PluginDescriptor_MissingName()
{
	PluginDescriptor pd1("a:b(c)");

	UnitTest::Assert("pd1 validness", pd1.IsValid());
	UnitTest::Assert("component part", pd1.ComponentName(), "a");
	UnitTest::Assert("plugin name part", pd1.PluginName(), "b");
	UnitTest::Assert("plugin arguments part", pd1.PluginArguments(), "c");
	UnitTest::Assert("name part", pd1.Name(), "");
}

static void Test_PluginDescriptor_SinglePartMeansFilename1()
{
	PluginDescriptor pd1("hello");

	UnitTest::Assert("pd1 validness", pd1.IsValid());
	UnitTest::Assert("component part", pd1.ComponentName(), "");
	UnitTest::Assert("plugin name part", pd1.PluginName(), "");
	UnitTest::Assert("plugin arguments part", pd1.PluginArguments(), "");
	UnitTest::Assert("name part", pd1.Name(), "hello");
}

static void Test_PluginDescriptor_SinglePartMeansFilename2()
{
	// Yes, this is a filename! If the behavior is to be changed in the
	// future, make sure to update these tests as well.
	PluginDescriptor pd1("hello()");

	UnitTest::Assert("pd1 validness", pd1.IsValid());
	UnitTest::Assert("component part", pd1.ComponentName(), "");
	UnitTest::Assert("plugin name part", pd1.PluginName(), "");
	UnitTest::Assert("plugin arguments part", pd1.PluginArguments(), "");
	UnitTest::Assert("name part", pd1.Name(), "hello()");
}

static void Test_PluginDescriptor_Examples()
{
	// Some examples, that are supposed to be parsable:

	// netbsd-GENERIC.gz			(load a basic file into cpu0)
	PluginDescriptor pd1("netbsd-GENERIC.gz");
	UnitTest::Assert("pd1 validness", pd1.IsValid());
	UnitTest::Assert("component part", pd1.ComponentName(), "");
	UnitTest::Assert("plugin name part", pd1.PluginName(), "");
	UnitTest::Assert("plugin arguments part", pd1.PluginArguments(), "");
	UnitTest::Assert("name part", pd1.Name(), "netbsd-GENERIC.gz");

	// cpu1:netbsd-GENERIC.gz		(load into a specific cpu)
	PluginDescriptor pd2("cpu1:netbsd-GENERIC.gz");
	UnitTest::Assert("pd2 validness", pd2.IsValid());
	UnitTest::Assert("component part", pd2.ComponentName(), "cpu1");
	UnitTest::Assert("plugin name part", pd2.PluginName(), "");
	UnitTest::Assert("plugin arguments part", pd2.PluginArguments(), "");
	UnitTest::Assert("name part", pd2.Name(), "netbsd-GENERIC.gz");

	// wd0:netbsd.img			(disk image file mapper)
	PluginDescriptor pd3("wd0:netbsd.img");
	UnitTest::Assert("pd3 validness", pd3.IsValid());
	UnitTest::Assert("component part", pd3.ComponentName(), "wd0");
	UnitTest::Assert("plugin name part", pd3.PluginName(), "");
	UnitTest::Assert("plugin arguments part", pd3.PluginArguments(), "");
	UnitTest::Assert("name part", pd3.Name(), "netbsd.img");

	// wd0:image(readonly):netbsd.img	(mapper with arguments! non-writable image)
	PluginDescriptor pd4("wd0:image(readonly):netbsd.img");
	UnitTest::Assert("pd4 validness", pd4.IsValid());
	UnitTest::Assert("component part", pd4.ComponentName(), "wd0");
	UnitTest::Assert("plugin name part", pd4.PluginName(), "image");
	UnitTest::Assert("plugin arguments part", pd4.PluginArguments(), "readonly");
	UnitTest::Assert("name part", pd4.Name(), "netbsd.img");

	// cpu0.dcache:cachestatisticsviewer()	(cache statistics viewer)
	PluginDescriptor pd5("cpu0.dcache:cachestatisticsviewer()");
	UnitTest::Assert("pd5 validness", pd5.IsValid());
	UnitTest::Assert("component part", pd5.ComponentName(), "cpu0.dcache");
	UnitTest::Assert("plugin name part", pd5.PluginName(), "cachestatisticsviewer");
	UnitTest::Assert("plugin arguments part", pd5.PluginArguments(), "");
	UnitTest::Assert("name part", pd5.Name(), "");

	// vga0:sdl()				(for displaying graphical controllers, which can
	// 					be interrogated about memory format, resolution etc)
	PluginDescriptor pd6("vga0:sdl()");
	UnitTest::Assert("pd6 validness", pd6.IsValid());
	UnitTest::Assert("component part", pd6.ComponentName(), "vga0");
	UnitTest::Assert("plugin name part", pd6.PluginName(), "sdl");
	UnitTest::Assert("plugin arguments part", pd6.PluginArguments(), "");
	UnitTest::Assert("name part", pd6.Name(), "");

	// vga0:sdl(DISPLAY=othermachine:1.0):VGA	(sdl on remote display)
	PluginDescriptor pd7("vga0:sdl(DISPLAY=othermachine:1.0):VGA");
	UnitTest::Assert("pd7 validness", pd7.IsValid());
	UnitTest::Assert("component part", pd7.ComponentName(), "vga0");
	UnitTest::Assert("plugin name part", pd7.PluginName(), "sdl");
	UnitTest::Assert("plugin arguments part", pd7.PluginArguments(), "DISPLAY=othermachine:1.0");
	UnitTest::Assert("name part", pd7.Name(), "VGA");

	// fb_videoram0:sdl(x=800,y=600,data=rgb24) (for displaying raw memory ranges)
	PluginDescriptor pd8("fb_videoram0:sdl(x=800,y=600,data=rgb24)");
	UnitTest::Assert("pd8 validness", pd8.IsValid());
	UnitTest::Assert("component part", pd8.ComponentName(), "fb_videoram0");
	UnitTest::Assert("plugin name part", pd8.PluginName(), "sdl");
	UnitTest::Assert("plugin arguments part", pd8.PluginArguments(), "x=800,y=600,data=rgb24");
	UnitTest::Assert("name part", pd8.Name(), "");

	// com0:stdio()				(default stdio)
	PluginDescriptor pd9("com0:stdio()");
	UnitTest::Assert("pd9 validness", pd9.IsValid());
	UnitTest::Assert("component part", pd9.ComponentName(), "com0");
	UnitTest::Assert("plugin name part", pd9.PluginName(), "stdio");
	UnitTest::Assert("plugin arguments part", pd9.PluginArguments(), "");
	UnitTest::Assert("name part", pd9.Name(), "");

	// pccom1:xterm(gnome-terminal):COM2		(terminal window)
	PluginDescriptor pd10("pccom1:xterm(gnome-terminal):COM2");
	UnitTest::Assert("pd10 validness", pd10.IsValid());
	UnitTest::Assert("component part", pd10.ComponentName(), "pccom1");
	UnitTest::Assert("plugin name part", pd10.PluginName(), "xterm");
	UnitTest::Assert("plugin arguments part", pd10.PluginArguments(), "gnome-terminal");
	UnitTest::Assert("name part", pd10.Name(), "COM2");
}

static void Test_PluginDescriptor_AllowPeriodInComponentName()
{
	PluginDescriptor pd1("cpu0.dcache:b(c):d");

	UnitTest::Assert("pd1 validness", pd1.IsValid());
	UnitTest::Assert("component part", pd1.ComponentName(), "cpu0.dcache");
	UnitTest::Assert("plugin name part", pd1.PluginName(), "b");
	UnitTest::Assert("plugin arguments part", pd1.PluginArguments(), "c");
	UnitTest::Assert("name part", pd1.Name(), "d");
}

static void Test_PluginDescriptor_AllowPeriodInFileName()
{
	PluginDescriptor pd1("wd0:image():netbsd.img");

	UnitTest::Assert("pd1 validness", pd1.IsValid());
	UnitTest::Assert("component part", pd1.ComponentName(), "wd0");
	UnitTest::Assert("plugin name part", pd1.PluginName(), "image");
	UnitTest::Assert("plugin arguments part", pd1.PluginArguments(), "");
	UnitTest::Assert("name part", pd1.Name(), "netbsd.img");
}

static void Test_PluginDescriptor_AllowUnderscoreInComponentName()
{
	PluginDescriptor pd1("fb_videoram0:b(c):d");

	UnitTest::Assert("pd1 validness", pd1.IsValid());
	UnitTest::Assert("component part", pd1.ComponentName(), "fb_videoram0");
	UnitTest::Assert("plugin name part", pd1.PluginName(), "b");
	UnitTest::Assert("plugin arguments part", pd1.PluginArguments(), "c");
	UnitTest::Assert("name part", pd1.Name(), "d");
}

static void Test_PluginDescriptor_AllowSpaceAsPluginArguments()
{
	PluginDescriptor pd1("cpu0:b(  ):d");

	UnitTest::Assert("pd1 validness", pd1.IsValid());
	UnitTest::Assert("component part", pd1.ComponentName(), "cpu0");
	UnitTest::Assert("plugin name part", pd1.PluginName(), "b");
	UnitTest::Assert("plugin arguments part", pd1.PluginArguments(), "  ");
	UnitTest::Assert("name part", pd1.Name(), "d");
}

static void Test_PluginDescriptor_AllowSpaceInPluginArguments()
{
	PluginDescriptor pd1("cpu0:b(c ):d");

	UnitTest::Assert("pd1 validness", pd1.IsValid());
	UnitTest::Assert("component part", pd1.ComponentName(), "cpu0");
	UnitTest::Assert("plugin name part", pd1.PluginName(), "b");
	UnitTest::Assert("plugin arguments part", pd1.PluginArguments(), "c ");
	UnitTest::Assert("name part", pd1.Name(), "d");
}

static void Test_PluginDescriptor_AllowCommaInPluginArguments()
{
	PluginDescriptor pd1("cpu0:b(c,q,-):d");

	UnitTest::Assert("pd1 validness", pd1.IsValid());
	UnitTest::Assert("component part", pd1.ComponentName(), "cpu0");
	UnitTest::Assert("plugin name part", pd1.PluginName(), "b");
	UnitTest::Assert("plugin arguments part", pd1.PluginArguments(), "c,q,-");
	UnitTest::Assert("name part", pd1.Name(), "d");
}

static void Test_PluginDescriptor_AllowColonInPluginArguments()
{
	PluginDescriptor pd1("a:sdl(DISPLAY=othermachine:1.0):d");

	UnitTest::Assert("pd1 validness", pd1.IsValid());
	UnitTest::Assert("component part", pd1.ComponentName(), "a");
	UnitTest::Assert("plugin name part", pd1.PluginName(), "sdl");
	UnitTest::Assert("plugin arguments part", pd1.PluginArguments(), "DISPLAY=othermachine:1.0");
	UnitTest::Assert("name part", pd1.Name(), "d");
}

static void Test_PluginDescriptor_AllowParenthesisInPluginArguments()
{
	PluginDescriptor pd1("a:b(c() hello())");

	UnitTest::Assert("pd1 validness", pd1.IsValid());
	UnitTest::Assert("component part", pd1.ComponentName(), "a");
	UnitTest::Assert("plugin name part", pd1.PluginName(), "b");
	UnitTest::Assert("plugin arguments part", pd1.PluginArguments(), "c() hello()");
	UnitTest::Assert("name part", pd1.Name(), "");
}

static void Test_PluginDescriptor_AllowLeftParenthesisInPluginArguments()
{
	PluginDescriptor pd1("a:b(\"c(\"):d");

	UnitTest::Assert("pd1 validness", pd1.IsValid());
	UnitTest::Assert("component part", pd1.ComponentName(), "a");
	UnitTest::Assert("plugin name part", pd1.PluginName(), "b");
	UnitTest::Assert("plugin arguments part", pd1.PluginArguments(), "\"c(\"");
	UnitTest::Assert("name part", pd1.Name(), "d");
}

static void Test_PluginDescriptor_AllowRightParenthesisInPluginArguments()
{
	PluginDescriptor pd1("a:B(\":)\"):d");

	UnitTest::Assert("pd1 validness", pd1.IsValid());
	UnitTest::Assert("component part", pd1.ComponentName(), "a");
	UnitTest::Assert("plugin name part", pd1.PluginName(), "B");
	UnitTest::Assert("plugin arguments part", pd1.PluginArguments(), "\":)\"");
	UnitTest::Assert("name part", pd1.Name(), "d");
}

static void Test_PluginDescriptor_EmptyString()
{
	PluginDescriptor pd1("");
	UnitTest::Assert("should have been invalid", pd1.IsValid() == false);
}

static void Test_PluginDescriptor_BadTwoPlugins1()
{
	PluginDescriptor pd1("a(b):c(d):e");
	UnitTest::Assert("should have been invalid", pd1.IsValid() == false);
}

static void Test_PluginDescriptor_BadTwoPlugins2()
{
	PluginDescriptor pd1("a:b(c):d(e)");
	UnitTest::Assert("should have been invalid", pd1.IsValid() == false);
}

static void Test_PluginDescriptor_BadPluginWithoutArgs()
{
	PluginDescriptor pd1("component:plugin:name");
	UnitTest::Assert("should have been invalid, plugin must "
	    "be followed by ()", pd1.IsValid() == false);
}

static void Test_PluginDescriptor_BadPluginWithSpaceBefore()
{
	PluginDescriptor pd1("component: plugin(ok):name");
	UnitTest::Assert("should have been invalid, plugin must "
	    "be followed by ()", pd1.IsValid() == false);
}

static void Test_PluginDescriptor_BadPluginWithSpaceAfterwards1()
{
	PluginDescriptor pd1("component:plugin (hi):name");
	UnitTest::Assert("should have been invalid, plugin must "
	    "be followed by ()", pd1.IsValid() == false);
}

static void Test_PluginDescriptor_BadPluginWithSpaceAfterwards2()
{
	PluginDescriptor pd1("component:plugin(hello) :name");
	UnitTest::Assert("should have been invalid, plugin must "
	    "be followed by ()", pd1.IsValid() == false);
}

static void Test_PluginDescriptor_BadPluginWithoutCompleteArgs1()
{
	PluginDescriptor pd1("component:plugin(arg:name");
	UnitTest::Assert("should have been invalid, plugin must "
	    "be followed by ()", pd1.IsValid() == false);
}

static void Test_PluginDescriptor_BadPluginWithoutCompleteArgs2()
{
	PluginDescriptor pd1("component:plugin()hello:name");
	UnitTest::Assert("should have been invalid, plugin must "
	    "be followed by () and nothing after that", pd1.IsValid() == false);
}

static void Test_PluginDescriptor_BadPluginWithoutCompleteArgs3()
{
	PluginDescriptor pd1("component:plugin(arg)x:name");
	UnitTest::Assert("should have been invalid, plugin must "
	    "be followed by () and nothing after that", pd1.IsValid() == false);
}

static void Test_PluginDescriptor_BadPluginWithMultipleArgs()
{
	PluginDescriptor pd1("component:plugin(arg1)(arg2):name");
	UnitTest::Assert("should have been invalid, plugin must "
	    "be followed by one set of () (with optional args) and nothing"
	    " after that", pd1.IsValid() == false);
}

static void Test_PluginDescriptor_BadPluginArgsButNoName1()
{
	PluginDescriptor pd1("a:():d");
	UnitTest::Assert("should have been invalid", pd1.IsValid() == false);
}

static void Test_PluginDescriptor_BadPluginArgsButNoName2()
{
	PluginDescriptor pd1("a:(something):d");
	UnitTest::Assert("should have been invalid", pd1.IsValid() == false);
}

static void Test_PluginDescriptor_BadWeirdSpace1()
{
	PluginDescriptor pd1("a :b(c):d");
	UnitTest::Assert("should have been invalid", pd1.IsValid() == false);
}

static void Test_PluginDescriptor_BadWeirdSpace2()
{
	PluginDescriptor pd1("a: b(c):d");
	UnitTest::Assert("should have been invalid", pd1.IsValid() == false);
}

static void Test_PluginDescriptor_BadWeirdSpace3()
{
	PluginDescriptor pd1("a:b(c) :d");
	UnitTest::Assert("should have been invalid", pd1.IsValid() == false);
}

static void Test_PluginDescriptor_BadWeirdSpace4()
{
	PluginDescriptor pd1("a:b(c): d");
	UnitTest::Assert("should have been invalid", pd1.IsValid() == false);
}

static void Test_PluginDescriptor_BadEmptyButSpecifiedComponent()
{
	PluginDescriptor pd1(":b(c):d");
	UnitTest::Assert("should have been invalid", pd1.IsValid() == false);
}

static void Test_PluginDescriptor_BadEmptyButSpecifiedName()
{
	PluginDescriptor pd1("a:b(c):");
	UnitTest::Assert("should have been invalid", pd1.IsValid() == false);
}

static void Test_PluginDescriptor_BadEmptyButSpecifiedPlugin()
{
	PluginDescriptor pd1("a::d");
	UnitTest::Assert("should have been invalid", pd1.IsValid() == false);
}

static void Test_PluginDescriptor_TooManyParts()
{
	PluginDescriptor pd1("a:b(c):c:e");
	UnitTest::Assert("should have been invalid", pd1.IsValid() == false);
}

UNITTESTS(PluginDescriptor)
{
	// Simple tests:
	UNITTEST(Test_PluginDescriptor_Simple);
	UNITTEST(Test_PluginDescriptor_MissingComponent);
	UNITTEST(Test_PluginDescriptor_MissingPluginArguments);
	UNITTEST(Test_PluginDescriptor_MissingPlugin);
	UNITTEST(Test_PluginDescriptor_MissingName);
	UNITTEST(Test_PluginDescriptor_SinglePartMeansFilename1);
	UNITTEST(Test_PluginDescriptor_SinglePartMeansFilename2);
	UNITTEST(Test_PluginDescriptor_Examples);

	// Specific chars that should work:
	UNITTEST(Test_PluginDescriptor_AllowPeriodInComponentName);
	UNITTEST(Test_PluginDescriptor_AllowPeriodInFileName);
	UNITTEST(Test_PluginDescriptor_AllowUnderscoreInComponentName);
	UNITTEST(Test_PluginDescriptor_AllowSpaceAsPluginArguments);
	UNITTEST(Test_PluginDescriptor_AllowSpaceInPluginArguments);
	UNITTEST(Test_PluginDescriptor_AllowCommaInPluginArguments);
	UNITTEST(Test_PluginDescriptor_AllowColonInPluginArguments);
	UNITTEST(Test_PluginDescriptor_AllowParenthesisInPluginArguments);
	UNITTEST(Test_PluginDescriptor_AllowLeftParenthesisInPluginArguments);
	UNITTEST(Test_PluginDescriptor_AllowRightParenthesisInPluginArguments);

	// Various kinds of errors:
	UNITTEST(Test_PluginDescriptor_EmptyString);
	UNITTEST(Test_PluginDescriptor_BadTwoPlugins1);
	UNITTEST(Test_PluginDescriptor_BadTwoPlugins2);
	UNITTEST(Test_PluginDescriptor_BadPluginWithoutArgs);
	UNITTEST(Test_PluginDescriptor_BadPluginWithSpaceBefore);
	UNITTEST(Test_PluginDescriptor_BadPluginWithSpaceAfterwards1);
	UNITTEST(Test_PluginDescriptor_BadPluginWithSpaceAfterwards2);
	UNITTEST(Test_PluginDescriptor_BadPluginWithoutCompleteArgs1);
	UNITTEST(Test_PluginDescriptor_BadPluginWithoutCompleteArgs2);
	UNITTEST(Test_PluginDescriptor_BadPluginWithoutCompleteArgs3);
	UNITTEST(Test_PluginDescriptor_BadPluginWithMultipleArgs);
	UNITTEST(Test_PluginDescriptor_BadPluginArgsButNoName1);
	UNITTEST(Test_PluginDescriptor_BadPluginArgsButNoName2);
	UNITTEST(Test_PluginDescriptor_BadWeirdSpace1);
	UNITTEST(Test_PluginDescriptor_BadWeirdSpace2);
	UNITTEST(Test_PluginDescriptor_BadWeirdSpace3);
	UNITTEST(Test_PluginDescriptor_BadWeirdSpace4);
	UNITTEST(Test_PluginDescriptor_BadEmptyButSpecifiedComponent);
	UNITTEST(Test_PluginDescriptor_BadEmptyButSpecifiedName);
	UNITTEST(Test_PluginDescriptor_BadEmptyButSpecifiedPlugin);
	UNITTEST(Test_PluginDescriptor_TooManyParts);
}

#endif

