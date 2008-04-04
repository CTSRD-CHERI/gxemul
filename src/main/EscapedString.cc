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

#include "EscapedString.h"


EscapedString::EscapedString(const string& str)
	: m_str(str)
{
}


string EscapedString::Generate() const
{
	string result = "\"";

	for (size_t i=0; i<m_str.length(); i++) {
		char ch = m_str[i];

		switch (ch) {
		case '\n':
			result += "\\n";
			break;
		case '\r':
			result += "\\r";
			break;
		case '\t':
			result += "\\t";
			break;
		case '"':
			result += "\\\"";
			break;
		case '\\':
			result += "\\\\";
			break;
		default:
			result += ch;
		}
	}

	result += "\"";

	return result;
}


string EscapedString::Decode() const
{
	string result = "";
	size_t offset = 0;
	
	if (m_str[0] == '"')
		offset = 1;

	for (size_t i=offset; i<m_str.length()-offset; i++) {
		char ch = m_str[i];

		switch (ch) {
		case '\\':
			{
				char ch2 = m_str[++i];
				switch (ch2) {
				case 'n':
					result += '\n';
					break;
				case 'r':
					result += '\r';
					break;
				case 't':
					result += '\t';
					break;
				default:
					result += ch2;
				}
				break;
			}
		default:
			result += ch;
		}
	}

	return result;
}


/*****************************************************************************/


#ifndef WITHOUTUNITTESTS

static void Test_EscapedString_Generate()
{
	UnitTest::Assert("trivial escape: normal text",
	    EscapedString("hello world 123").Generate() ==
	    "\"hello world 123\"");

	UnitTest::Assert("escape tab",
	    EscapedString("hi\tworld").Generate() ==
	    "\"hi\\tworld\"");

	UnitTest::Assert("escape newline and carriage return",
	    EscapedString("hi\nworld\ragain").Generate() ==
	    "\"hi\\nworld\\ragain\"");

	UnitTest::Assert("escape quotes",
	    EscapedString("hi'123\"456\"789").Generate() ==
	    "\"hi'123\\\"456\\\"789\"");

	UnitTest::Assert("escaped escape char",
	    EscapedString("Hello\\world").Generate() ==
	    "\"Hello\\\\world\"");
}

static void Test_EscapedString_Decode()
{
	UnitTest::Assert("trivial escape: normal text",
	    EscapedString("\"hello world 123\"").Decode() ==
	    "hello world 123");

	UnitTest::Assert("escape tab",
	    EscapedString("\"hi\\tworld\"").Decode() ==
	    "hi\tworld");

	UnitTest::Assert("escape newline and carriage return",
	    EscapedString("\"hi\\nworld\\ragain\"").Decode() ==
	    "hi\nworld\ragain");

	UnitTest::Assert("escape quotes",
	    EscapedString("\"hi'123\\\"456\\\"789\"").Decode() ==
	    "hi'123\"456\"789");

	UnitTest::Assert("escaped escape char",
	    EscapedString("\"Hello\\\\world\"").Decode() ==
	    "Hello\\world");
}

static void Test_EscapedString_Decode_WithoutQuotes()
{
	UnitTest::Assert("trivial escape: normal text",
	    EscapedString("hello world 123").Decode() ==
	    "hello world 123");

	UnitTest::Assert("escape tab",
	    EscapedString("hi\\tworld").Decode() ==
	    "hi\tworld");

	UnitTest::Assert("escape newline and carriage return",
	    EscapedString("hi\\nworld\\ragain").Decode() ==
	    "hi\nworld\ragain");

	UnitTest::Assert("escape quotes",
	    EscapedString("hi'123\\\"456\\\"789").Decode() ==
	    "hi'123\"456\"789");

	UnitTest::Assert("escaped escape char",
	    EscapedString("Hello\\\\world").Decode() ==
	    "Hello\\world");
}

UNITTESTS(EscapedString)
{
	UNITTEST(Test_EscapedString_Generate);
	UNITTEST(Test_EscapedString_Decode);
	UNITTEST(Test_EscapedString_Decode_WithoutQuotes);
}

#endif
