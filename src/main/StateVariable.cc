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
 *
 *
 *  $Id: StateVariable.cc,v 1.4 2008/03/12 11:45:41 debug Exp $
 */

#include <assert.h>

#include "EscapedString.h"
#include "StateVariable.h"


StateVariable::StateVariable()
	: m_type(String)
{
	m_value.pstr = NULL;
}


StateVariable::StateVariable(const string& name, string* ptrToString)
	: m_name(name)
	, m_type(String)
{
	m_value.pstr = ptrToString;
}


StateVariable::StateVariable(const string& name, bool* ptrToVar)
	: m_name(name)
	, m_type(Bool)
{
	m_value.pbool = ptrToVar;
}


StateVariable::StateVariable(const string& name, uint8_t* ptrToVar,
		size_t arrayLength)
	: m_name(name)
	, m_type(Array)
	, m_arrayLength(arrayLength)
{
	m_value.parray = ptrToVar;
}


StateVariable::StateVariable(const string& name, uint8_t* ptrToVar)
	: m_name(name)
	, m_type(UInt8)
{
	m_value.puint8 = ptrToVar;
}


StateVariable::StateVariable(const string& name, uint16_t* ptrToVar)
	: m_name(name)
	, m_type(UInt16)
{
	m_value.puint16 = ptrToVar;
}


StateVariable::StateVariable(const string& name, uint32_t* ptrToVar)
	: m_name(name)
	, m_type(UInt32)
{
	m_value.puint32 = ptrToVar;
}


StateVariable::StateVariable(const string& name, uint64_t* ptrToVar)
	: m_name(name)
	, m_type(UInt64)
{
	m_value.puint64 = ptrToVar;
}


StateVariable::StateVariable(const string& name, int8_t* ptrToVar)
	: m_name(name)
	, m_type(SInt8)
{
	m_value.psint8 = ptrToVar;
}


StateVariable::StateVariable(const string& name, int16_t* ptrToVar)
	: m_name(name)
	, m_type(SInt16)
{
	m_value.psint16 = ptrToVar;
}


StateVariable::StateVariable(const string& name, int32_t* ptrToVar)
	: m_name(name)
	, m_type(SInt32)
{
	m_value.psint32 = ptrToVar;
}


StateVariable::StateVariable(const string& name, int64_t* ptrToVar)
	: m_name(name)
	, m_type(SInt64)
{
	m_value.psint64 = ptrToVar;
}


enum StateVariable::Type StateVariable::GetType() const
{
	return m_type;
}


const string& StateVariable::GetName() const
{
	return m_name;
}


string StateVariable::GetTypeString() const
{
	switch (m_type) {
	case String:
		return "string";
	case Bool:
		return "bool";
	case Array:
		return "array";
	case UInt8:
		return "uint8";
	case UInt16:
		return "uint16";
	case UInt32:
		return "uint32";
	case UInt64:
		return "uint64";
	case SInt8:
		return "sint8";
	case SInt16:
		return "sint16";
	case SInt32:
		return "sint32";
	case SInt64:
		return "sint64";
	default:
		return "unknown";
	}
}


bool StateVariable::CopyValueFrom(const StateVariable& otherVariable)
{
	if (m_type != otherVariable.m_type)
		return false;

	switch (m_type) {
	case String:
		*m_value.pstr = *otherVariable.m_value.pstr;
		break;
	case Bool:
		*m_value.pbool = *otherVariable.m_value.pbool;
		break;
	case Array:
		if (m_arrayLength != otherVariable.m_arrayLength) {
			std::cerr << "INTERNAL ERROR in StateVariable::"
			    "CopyValueFrom: lengths differ. " <<
			    m_arrayLength << " != " <<
			    otherVariable.m_arrayLength << ".\n";
			assert(false);
			return false;
		}
		memcpy(m_value.parray, otherVariable.m_value.parray,
		    m_arrayLength);
		break;
	case UInt8:
		*m_value.puint8 = *otherVariable.m_value.puint8;
		break;
	case UInt16:
		*m_value.puint16 = *otherVariable.m_value.puint16;
		break;
	case UInt32:
		*m_value.puint32 = *otherVariable.m_value.puint32;
		break;
	case UInt64:
		*m_value.puint64 = *otherVariable.m_value.puint64;
		break;
	case SInt8:
		*m_value.psint8 = *otherVariable.m_value.psint8;
		break;
	case SInt16:
		*m_value.psint16 = *otherVariable.m_value.psint16;
		break;
	case SInt32:
		*m_value.psint32 = *otherVariable.m_value.psint32;
		break;
	case SInt64:
		*m_value.psint64 = *otherVariable.m_value.psint64;
		break;
	default:
		// Unknown type?
		assert(false);
		return false;
	}

	return true;
}


string StateVariable::ToString() const
{
	stringstream sstr;
	switch (m_type) {
	case String:
		return m_value.pstr == NULL? "" : *m_value.pstr;
	case Bool:
		sstr << (*m_value.pbool? "true" : "false");
		return sstr.str();
	case Array:
		// TODO
		assert(false);
		return "TODO";
	case UInt8:
		sstr << (int) *m_value.puint8;
		return sstr.str();
	case UInt16:
		sstr << *m_value.puint16;
		return sstr.str();
	case UInt32:
		sstr << *m_value.puint32;
		return sstr.str();
	case UInt64:
		sstr << *m_value.puint64;
		return sstr.str();
	case SInt8:
		sstr << (int) *m_value.psint8;
		return sstr.str();
	case SInt16:
		sstr << *m_value.psint16;
		return sstr.str();
	case SInt32:
		sstr << *m_value.psint32;
		return sstr.str();
	case SInt64:
		sstr << *m_value.psint64;
		return sstr.str();
	default:
		return "unknown";
	}
}

string StateVariable::Serialize(SerializationContext& context) const
{
	return
	    context.Tabs() + GetTypeString() + " " + m_name + " " +
	    EscapedString(ToString()).Generate() + "\n";
}


bool StateVariable::SetValue(const string& escapedStringValue)
{
	stringstream sstr;

	if (m_value.pstr == NULL)
		return false;

	if (m_type == String) {
		*m_value.pstr = EscapedString(escapedStringValue).Decode();
	} if (m_type == Bool) {
		string str = EscapedString(escapedStringValue).Decode();
		if (str == "true")
			*m_value.pbool = true;
		else if (str == "false")
			*m_value.pbool = false;
		else
			return false;
	} else if (m_type == UInt8) {
		string str = EscapedString(escapedStringValue).Decode();
		uint8_t tmp = strtoull(str.c_str(), NULL, 0);
		sstr << (int) tmp;
		if (sstr.str() == str)
			*m_value.puint8 = tmp;
		else
			return false;
	} else if (m_type == UInt16) {
		string str = EscapedString(escapedStringValue).Decode();
		uint16_t tmp = strtoull(str.c_str(), NULL, 0);
		sstr << tmp;
		if (sstr.str() == str)
			*m_value.puint16 = tmp;
		else
			return false;
	} else if (m_type == UInt32) {
		string str = EscapedString(escapedStringValue).Decode();
		uint32_t tmp = strtoull(str.c_str(), NULL, 0);
		sstr << tmp;
		if (sstr.str() == str)
			*m_value.puint32 = tmp;
		else
			return false;
	} else if (m_type == UInt64) {
		string str = EscapedString(escapedStringValue).Decode();
		uint64_t tmp = strtoull(str.c_str(), NULL, 0);
		sstr << tmp;
		if (sstr.str() == str)
			*m_value.puint64 = tmp;
		else
			return false;
	} else if (m_type == SInt8) {
		string str = EscapedString(escapedStringValue).Decode();
		int8_t tmp = strtoull(str.c_str(), NULL, 0);
		sstr << (int) tmp;
		if (sstr.str() == str)
			*m_value.psint8 = tmp;
		else
			return false;
	} else if (m_type == SInt16) {
		string str = EscapedString(escapedStringValue).Decode();
		int16_t tmp = strtoull(str.c_str(), NULL, 0);
		sstr << tmp;
		if (sstr.str() == str)
			*m_value.psint16 = tmp;
		else
			return false;
	} else if (m_type == SInt32) {
		string str = EscapedString(escapedStringValue).Decode();
		int32_t tmp = strtoull(str.c_str(), NULL, 0);
		sstr << tmp;
		if (sstr.str() == str)
			*m_value.psint32 = tmp;
		else
			return false;
	} else if (m_type == SInt64) {
		string str = EscapedString(escapedStringValue).Decode();
		int64_t tmp = strtoull(str.c_str(), NULL, 0);
		sstr << tmp;
		if (sstr.str() == str)
			*m_value.psint64 = tmp;
		else
			return false;
	} else {
		return false;
	}
	
	return true;
}


/*****************************************************************************/


#ifndef WITHOUTUNITTESTS

static void Test_StateVariable_String_Construct()
{
	string myString = "hi";

	StateVariable var("hello", &myString);

	UnitTest::Assert("name should be hello",
	    var.GetName(), "hello");
	UnitTest::Assert("type should be String",
	    var.GetType() == StateVariable::String);
	UnitTest::Assert("value should be hi",
	    var.ToString(), "hi");
}

static void Test_StateVariable_String_SetValue()
{
	string myString = "hi";

	StateVariable var("hello", &myString);

	var.SetValue("value2");

	UnitTest::Assert("type should still be String",
	    var.GetType() == StateVariable::String);
	UnitTest::Assert("value should now be value2",
	    var.ToString(), "value2");
	UnitTest::Assert("myString should have been updated",
	    myString, "value2");
}

static void Test_StateVariable_String_CopyValueFrom()
{
	string myString1 = "hi";
	string myString2 = "something";

	StateVariable var1("hello", &myString1);
	StateVariable var2("world", &myString2);

	UnitTest::Assert("value should initially be hi",
	    var1.ToString(), "hi");

	var1.CopyValueFrom(var2);

	UnitTest::Assert("name should still be hello",
	    var1.GetName(), "hello");
	UnitTest::Assert("type should still be String",
	    var1.GetType() == StateVariable::String);
	UnitTest::Assert("value should be changed to something",
	    var1.ToString(), "something");
	UnitTest::Assert("myString1 should have been updated",
	    myString1, "something");
}

static void Test_StateVariable_String_Serialize()
{
	string hi = "value world";
	StateVariable var("hello", &hi);

	SerializationContext dummyContext;

	UnitTest::Assert("variable serialization mismatch?",
	    var.Serialize(dummyContext), "string hello \"value world\"\n");
}

static void Test_StateVariable_String_Serialize_WithEscapes()
{
	string s = "a\\b\tc\nd\re\bf\"g'h";
	StateVariable var("hello", &s);

	SerializationContext dummyContext;

	UnitTest::Assert("variable serialization mismatch?",
	    var.Serialize(dummyContext) ==
	    "string hello " + EscapedString(s).Generate() + "\n");
}

static void Test_StateVariable_Bool_Construct()
{
	bool myBool = true;

	StateVariable var("hello", &myBool);

	UnitTest::Assert("name should be hello",
	    var.GetName(), "hello");
	UnitTest::Assert("type should be Bool",
	    var.GetType() == StateVariable::Bool);
	UnitTest::Assert("value should be true",
	    var.ToString(), "true");
}

static void Test_StateVariable_Bool_SetValue()
{
	bool myBool = true;

	StateVariable var("hello", &myBool);

	UnitTest::Assert("changing to false should be possible",
	    var.SetValue("false") == true);

	UnitTest::Assert("type should still be Bool",
	    var.GetType() == StateVariable::Bool);
	UnitTest::Assert("value should now be changed",
	    var.ToString(), "false");
	UnitTest::Assert("myBool should have been updated",
	    myBool == false);

	UnitTest::Assert("changing to true should be possible",
	    var.SetValue("true") == true);

	UnitTest::Assert("value should now be changed again",
	    var.ToString(), "true");
	UnitTest::Assert("myBool should have been updated again",
	    myBool == true);

	UnitTest::Assert("changing to non-bool value should not be possible",
	    var.SetValue("hello") == false);

	UnitTest::Assert("value should not be changed",
	    var.ToString(), "true");
}

static void Test_StateVariable_Bool_CopyValueFrom()
{
	bool myBool1 = false;
	bool myBool2 = true;

	StateVariable var1("hello", &myBool1);
	StateVariable var2("world", &myBool2);

	UnitTest::Assert("copying from bool to bool should be possible",
	    var1.CopyValueFrom(var2) == true);

	UnitTest::Assert("name should still be hello",
	    var1.GetName(), "hello");
	UnitTest::Assert("type should still be Bool",
	    var1.GetType() == StateVariable::Bool);
	UnitTest::Assert("value should be changed to true",
	    var1.ToString(), "true");
	UnitTest::Assert("myBool1 should have been updated",
	    myBool1 == true);

	string myString = "hm";
	StateVariable var3("test", &myString);

	UnitTest::Assert("copying from string to bool should not be possible",
	    var1.CopyValueFrom(var3) == false);
}

static void Test_StateVariable_Bool_Serialize()
{
	bool myBool = true;
	StateVariable var("hello", &myBool);

	SerializationContext dummyContext;

	UnitTest::Assert("variable serialization mismatch (1)",
	    var.Serialize(dummyContext), "bool hello \"true\"\n");
	
	myBool = false;

	UnitTest::Assert("variable serialization mismatch (2)",
	    var.Serialize(dummyContext), "bool hello \"false\"\n");
}

static void Test_StateVariable_Numeric_Construct()
{
	uint8_t  varUInt8  = 223;
	uint16_t varUInt16 = 55000;
	uint32_t varUInt32 = 3000000001;
	uint64_t varUInt64 = 0xfedc010203040506ULL;
	int8_t   varSInt8  = -120;
	int16_t  varSInt16 = -22000;
	int32_t  varSInt32 = -1000000001;
	int64_t  varSInt64 = 0xfedc010203040506ULL;

	StateVariable vuint8 ("vuint8",  &varUInt8);
	StateVariable vuint16("vuint16", &varUInt16);
	StateVariable vuint32("vuint32", &varUInt32);
	StateVariable vuint64("vuint64", &varUInt64);
	StateVariable vsint8 ("vsint8",  &varSInt8);
	StateVariable vsint16("vsint16", &varSInt16);
	StateVariable vsint32("vsint32", &varSInt32);
	StateVariable vsint64("vsint64", &varSInt64);

	// Types
	UnitTest::Assert("UInt8",  vuint8.GetType()  == StateVariable::UInt8);
	UnitTest::Assert("UInt16", vuint16.GetType() == StateVariable::UInt16);
	UnitTest::Assert("UInt32", vuint32.GetType() == StateVariable::UInt32);
	UnitTest::Assert("UInt64", vuint64.GetType() == StateVariable::UInt64);
	UnitTest::Assert("SInt8",  vsint8.GetType()  == StateVariable::SInt8);
	UnitTest::Assert("SInt16", vsint16.GetType() == StateVariable::SInt16);
	UnitTest::Assert("SInt32", vsint32.GetType() == StateVariable::SInt32);
	UnitTest::Assert("SInt64", vsint64.GetType() == StateVariable::SInt64);

	// Values
	UnitTest::Assert("value UInt8",  vuint8.ToString(),  "223");
	UnitTest::Assert("value UInt16", vuint16.ToString(), "55000");
	UnitTest::Assert("value UInt32", vuint32.ToString(), "3000000001");
	UnitTest::Assert("value UInt64", vuint64.ToString(),
	    "18364554488662197510");
	UnitTest::Assert("value SInt8",  vsint8.ToString(),  "-120");
	UnitTest::Assert("value SInt16", vsint16.ToString(), "-22000");
	UnitTest::Assert("value SInt32", vsint32.ToString(), "-1000000001");
	UnitTest::Assert("value SInt64", vsint64.ToString(),
	    "-82189585047354106");
}

static void Test_StateVariable_Numeric_SetValue()
{
	uint8_t  varUInt8  = 223;
	uint16_t varUInt16 = 55000;
	uint32_t varUInt32 = 3000000001;
	uint64_t varUInt64 = 0xfedc010203040506ULL;
	int8_t   varSInt8  = -120;
	int16_t  varSInt16 = -22000;
	int32_t  varSInt32 = -1000000001;
	int64_t  varSInt64 = 0xfedc010203040506ULL;

	StateVariable vuint8 ("vuint8",  &varUInt8);
	StateVariable vuint16("vuint16", &varUInt16);
	StateVariable vuint32("vuint32", &varUInt32);
	StateVariable vuint64("vuint64", &varUInt64);
	StateVariable vsint8 ("vsint8",  &varSInt8);
	StateVariable vsint16("vsint16", &varSInt16);
	StateVariable vsint32("vsint32", &varSInt32);
	StateVariable vsint64("vsint64", &varSInt64);

	UnitTest::Assert("changing to 'hello' should not be possible",
	    vuint8.SetValue("hello") == false);

	// UInt8
	UnitTest::Assert("changing to 100 should be possible",
	    vuint8.SetValue("100") == true);
	UnitTest::Assert("varUInt8 should have been updated",
	    varUInt8, 100);
	UnitTest::Assert("changing to 300 should not be possible",
	    vuint8.SetValue("300") == false);
	UnitTest::Assert("varUInt8 should not have been updated",
	    varUInt8, 100);
	UnitTest::Assert("changing to -110 should not be possible",
	    vuint8.SetValue("-110") == false);
	UnitTest::Assert("varUInt8 should not have been updated",
	    varUInt8, 100);

	// SInt8
	UnitTest::Assert("changing to 100 should be possible",
	    vsint8.SetValue("100") == true);
	UnitTest::Assert("varSInt8 should have been updated",
	    varSInt8, 100);
	UnitTest::Assert("changing to 200 should not be possible",
	    vsint8.SetValue("200") == false);
	UnitTest::Assert("varSInt8 should not have been updated",
	    varSInt8, 100);
	UnitTest::Assert("changing to -210 should not be possible",
	    vsint8.SetValue("-210") == false);
	UnitTest::Assert("varSInt8 should not have been updated",
	    varSInt8, 100);
	UnitTest::Assert("changing to -110 should be possible",
	    vsint8.SetValue("-110") == true);
	UnitTest::Assert("varSInt8 should have been updated",
	    varSInt8, -110);

	// Tests for other numeric types: TODO
}

UNITTESTS(StateVariable)
{
	// String tests
	UNITTEST(Test_StateVariable_String_Construct);
	UNITTEST(Test_StateVariable_String_SetValue);
	UNITTEST(Test_StateVariable_String_CopyValueFrom);
	UNITTEST(Test_StateVariable_String_Serialize);
	UNITTEST(Test_StateVariable_String_Serialize_WithEscapes);

	// Bool tests
	UNITTEST(Test_StateVariable_Bool_Construct);
	UNITTEST(Test_StateVariable_Bool_SetValue);
	UNITTEST(Test_StateVariable_Bool_CopyValueFrom);
	UNITTEST(Test_StateVariable_Bool_Serialize);

	// Numeric tests
	UNITTEST(Test_StateVariable_Numeric_Construct);
	UNITTEST(Test_StateVariable_Numeric_SetValue);
	//UNITTEST(Test_StateVariable_Numeric_CopyValueFrom);
	//UNITTEST(Test_StateVariable_Numeric_Serialize);

	// TODO: array tests
}

#endif

