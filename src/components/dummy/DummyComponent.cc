/*
 *  Copyright (C) 2007-2009  Anders Gavare.  All rights reserved.
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

#include "components/DummyComponent.h"
#include "Checksum.h"
#include "ComponentFactory.h"


DummyComponent::DummyComponent(string className)
	: Component(className, className)
{
	// The dummy component should have no additional state.
}


refcount_ptr<Component> DummyComponent::Create()
{
	return new DummyComponent();
}


string DummyComponent::GetAttribute(const string& attributeName)
{
	if (attributeName == "stable")
		return "yes";

	if (attributeName == "description")
		return "A dummy component, which does nothing.";

	return Component::GetAttribute(attributeName);
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

static void Test_DummyComponent_CreateComponent()
{
	refcount_ptr<Component> component;

	component = ComponentFactory::CreateComponent("dummy");
	UnitTest::Assert("creating a dummy component should be possible",
	    component.IsNULL() == false);

	UnitTest::Assert("the name should be 'dummy'",
	    component->GetClassName() == "dummy");
}

static void Test_DummyComponent_GetSetParent()
{
	refcount_ptr<Component> dummyA = new DummyComponent;
	refcount_ptr<Component> dummyB = new DummyComponent;

	UnitTest::Assert("parent should initially be NULL",
	    dummyA->GetParent() == NULL);

	dummyA->SetParent(dummyB);

	UnitTest::Assert("parent should be dummyB",
	    dummyA->GetParent() == dummyB);

	dummyA->SetParent(NULL);

	UnitTest::Assert("parent should now be NULL",
	    dummyA->GetParent() == NULL);
}

static void Test_DummyComponent_AddChild_Sets_Parent()
{
	refcount_ptr<Component> dummy = new DummyComponent;
	refcount_ptr<Component> dummyChildA = new DummyComponent;

	UnitTest::Assert("child A's parent should initially be NULL",
	    dummyChildA->GetParent() == NULL);

	dummy->AddChild(dummyChildA);

	UnitTest::Assert("child A's parent should now be dummy",
	    dummyChildA->GetParent() == dummy);
}

static void Test_DummyComponent_AddChildren_Count()
{
	refcount_ptr<Component> dummy = new DummyComponent;
	refcount_ptr<Component> dummyChildA = new DummyComponent;
	refcount_ptr<Component> dummyChildB = new DummyComponent;
	refcount_ptr<Component> dummyChildC = new DummyComponent;

	UnitTest::Assert("there should initially be no child components",
	    dummy->GetChildren().size() == 0);

	dummy->AddChild(dummyChildA);
	dummy->AddChild(dummyChildB);
	dummy->AddChild(dummyChildC);

	UnitTest::Assert("there should be 3 child components",
	    dummy->GetChildren().size() == 3);
}

static void Test_DummyComponent_Add_Tree_Of_Children()
{
	refcount_ptr<Component> dummy = new DummyComponent;
	refcount_ptr<Component> dummyChildA = new DummyComponent;
	refcount_ptr<Component> dummyChildB = new DummyComponent;
	refcount_ptr<Component> dummyChildC = new DummyComponent;

	dummyChildA->AddChild(dummyChildB);
	dummyChildA->AddChild(dummyChildC);
	dummy->AddChild(dummyChildA);

	UnitTest::Assert("there should be 1 child component",
	    dummy->GetChildren().size() == 1);
	UnitTest::Assert("there should be 2 child components in dummyChildA",
	    dummyChildA->GetChildren().size() == 2);
}

static void Test_DummyComponent_RemoveChild()
{
	refcount_ptr<Component> dummy = new DummyComponent;
	refcount_ptr<Component> dummyChildA = new DummyComponent;
	refcount_ptr<Component> dummyChildB = new DummyComponent;
	refcount_ptr<Component> dummyChildC = new DummyComponent;

	dummyChildA->AddChild(dummyChildB);
	dummyChildA->AddChild(dummyChildC);
	dummy->AddChild(dummyChildA);

	UnitTest::Assert("there should be 1 child component",
	    dummy->GetChildren().size() == 1);
	UnitTest::Assert("child A's parent should be dummy",
	    dummyChildA->GetParent() == dummy);
	UnitTest::Assert("there should be 2 child components in dummyChildA",
	    dummyChildA->GetChildren().size() == 2);

	dummy->RemoveChild(dummyChildA);

	UnitTest::Assert("there should now be 0 child components",
	    dummy->GetChildren().size() == 0);

	UnitTest::Assert(
	    "there should still be 2 child components in dummyChildA",
	    dummyChildA->GetChildren().size() == 2);
	UnitTest::Assert("child A should have no parent",
	    dummyChildA->GetParent() == NULL);
}

static void Test_DummyComponent_AddChild_UniqueName()
{
	refcount_ptr<Component> dummy = new DummyComponent;
	refcount_ptr<Component> dummyChildA = new DummyComponent;
	refcount_ptr<Component> dummyChildB = new DummyComponent;
	refcount_ptr<Component> dummyChildC = new DummyComponent;

	UnitTest::Assert("dummyChildA should have no initial name",
	    dummyChildA->GetVariable("name")->ToString(), "");

	dummy->AddChild(dummyChildA);

	const StateVariable* name = dummyChildA->GetVariable("name");
	UnitTest::Assert("dummyChildA's name mismatch",
	    name->ToString(), "dummy0");

	dummy->AddChild(dummyChildB);

	name = dummyChildA->GetVariable("name");
	UnitTest::Assert("dummyChildA should still have a name",
	    name != NULL);
	UnitTest::Assert("dummyChildA's name changed unexpectedly?",
	    name->ToString(), "dummy0");

	name = dummyChildB->GetVariable("name");
	UnitTest::Assert("dummyChildB should have a name now",
	    name != NULL);
	UnitTest::Assert("dummyChildB's name mismatch",
	    name->ToString(), "dummy1");

	dummy->AddChild(dummyChildC);

	name = dummyChildC->GetVariable("name");
	UnitTest::Assert("dummyChildC should have a name now",
	    name != NULL);
	UnitTest::Assert("dummyChildC's name mismatch",
	    name->ToString(), "dummy2");

	dummy->RemoveChild(dummyChildA);

	name = dummyChildB->GetVariable("name");
	UnitTest::Assert("dummyChildB should still have a name, after"
		" removing child A",
	    name != NULL);
	UnitTest::Assert("dummyChildB's should not have changed",
	    name->ToString(), "dummy1");
}

static void Test_DummyComponent_GeneratePath()
{
	refcount_ptr<Component> dummyA = new DummyComponent;
	refcount_ptr<Component> dummyB = new DummyComponent;
	refcount_ptr<Component> dummyC = new DummyComponent;

	UnitTest::Assert("generated path with no name",
	    dummyA->GeneratePath(), "(dummy)");

	dummyB->AddChild(dummyA);

	UnitTest::Assert("generated path with default name fallback",
	    dummyA->GeneratePath(), "(dummy).dummy0");

	dummyC->AddChild(dummyB);

	UnitTest::Assert("generated path with two parent levels",
	    dummyA->GeneratePath(), "(dummy).dummy0.dummy0");

	dummyC->RemoveChild(dummyB);

	UnitTest::Assert("generated path with one parent level again",
	    dummyA->GeneratePath(), "dummy0.dummy0");
}

static void Test_DummyComponent_LookupPath()
{
	refcount_ptr<Component> dummyA = new DummyComponent;

	dummyA->SetVariableValue("name", "\"hello\"");

	refcount_ptr<Component> component1 = dummyA->LookupPath("nonsense");
	UnitTest::Assert("nonsense lookup should fail",
	    component1.IsNULL() == true);

	refcount_ptr<Component> component2 = dummyA->LookupPath("hello");
	UnitTest::Assert("lookup should have found the component itself",
	    component2 == dummyA);

	refcount_ptr<Component> child = new DummyComponent;
	refcount_ptr<Component> childchild = new DummyComponent;
	child->SetVariableValue("name", "\"x\"");
	childchild->SetVariableValue("name", "\"y\"");
	dummyA->AddChild(child);
	child->AddChild(childchild);

	refcount_ptr<Component> component3 = dummyA->LookupPath("hello");
	UnitTest::Assert("lookup should still succeed, when dummyA has"
		" children",
	    component3 == dummyA);

	refcount_ptr<Component> component4 = dummyA->LookupPath("hello.z");
	UnitTest::Assert("lookup of nonsense child of dummyA should fail",
	    component4.IsNULL() == true);

	refcount_ptr<Component> component5 = dummyA->LookupPath("hello.x");
	UnitTest::Assert("lookup of child of dummyA should succeed",
	    component5 == child);

	refcount_ptr<Component> component6 = dummyA->LookupPath("hello.x.y");
	UnitTest::Assert("lookup of grandchild of dummyA should succeed",
	    component6 == childchild);
}

static void Test_DummyComponent_FindPathByPartialMatch()
{
	refcount_ptr<Component> root = new DummyComponent;
	refcount_ptr<Component> machine0 = new DummyComponent;
	refcount_ptr<Component> machine1 = new DummyComponent;
	refcount_ptr<Component> machine2 = new DummyComponent;
	refcount_ptr<Component> machine3 = new DummyComponent;
	refcount_ptr<Component> m0isabus0 = new DummyComponent;
	refcount_ptr<Component> m1pcibus0 = new DummyComponent;
	refcount_ptr<Component> m1pcibus1 = new DummyComponent;
	refcount_ptr<Component> m2pcibus0 = new DummyComponent;
	refcount_ptr<Component> m3otherpci = new DummyComponent;

	root->GetVariable("name")->SetValue("\"root\"");
	machine0->GetVariable("name")->SetValue("\"machine0\"");
	machine1->GetVariable("name")->SetValue("\"machine1\"");
	machine2->GetVariable("name")->SetValue("\"machine2\"");
	machine3->GetVariable("name")->SetValue("\"machine3\"");
	m0isabus0->GetVariable("name")->SetValue("\"isabus0\"");
	m1pcibus0->GetVariable("name")->SetValue("\"pcibus0\"");
	m1pcibus1->GetVariable("name")->SetValue("\"pcibus1\"");
	m2pcibus0->GetVariable("name")->SetValue("\"pcibus0\"");
	m3otherpci->GetVariable("name")->SetValue("\"otherpci\"");

	root->AddChild(machine0);
	root->AddChild(machine1);
	root->AddChild(machine2);
	root->AddChild(machine3);

	machine0->AddChild(m0isabus0);
	machine1->AddChild(m1pcibus0);
	machine1->AddChild(m1pcibus1);
	machine2->AddChild(m2pcibus0);
	machine3->AddChild(m3otherpci);

	vector<string> matches;

	matches = root->FindPathByPartialMatch("nonsense");
	UnitTest::Assert("there should be no nonsense matches",
	    matches.size(), 0);
	
	matches = root->FindPathByPartialMatch("");
	UnitTest::Assert("empty string should return all components",
	    matches.size(), 10);

	matches = root->FindPathByPartialMatch("pci");
	UnitTest::Assert("pci matches mismatch",
	    matches.size(), 3);
	UnitTest::Assert("pci match 0 mismatch",
	    matches[0], "root.machine1.pcibus0");
	UnitTest::Assert("pci match 1 mismatch",
	    matches[1], "root.machine1.pcibus1");
	UnitTest::Assert("pci match 2 mismatch",
	    matches[2], "root.machine2.pcibus0");

	matches = root->FindPathByPartialMatch("machine1");
	UnitTest::Assert("machine1 match mismatch",
	    matches.size(), 1);
	UnitTest::Assert("machine1 match 0 mismatch",
	    matches[0], "root.machine1");

	matches = root->FindPathByPartialMatch("machine2.pcibus");
	UnitTest::Assert("machine2.pcibus match mismatch",
	    matches.size(), 1);
	UnitTest::Assert("machine2.pcibus match 0 mismatch",
	    matches[0], "root.machine2.pcibus0");

	matches = root->FindPathByPartialMatch("machine.pcibus");
	UnitTest::Assert("machine.pcibus should have no matches",
	    matches.size(), 0);
}

static void Test_DummyComponent_GetUnknownVariable()
{
	refcount_ptr<Component> dummy = new DummyComponent;

	UnitTest::Assert("variable variablename should not be set",
	    dummy->GetVariable("variablename") == NULL);
}

static void Test_DummyComponent_Clone_Basic()
{
	refcount_ptr<Component> dummy = new DummyComponent;
	refcount_ptr<Component> dummyChildA = new DummyComponent;
	refcount_ptr<Component> dummyChildA1 = new DummyComponent;
	refcount_ptr<Component> dummyChildA2 = new DummyComponent;

	dummyChildA->AddChild(dummyChildA1);
	dummyChildA->AddChild(dummyChildA2);
	dummy->AddChild(dummyChildA);

	Checksum originalChecksum;
	dummy->AddChecksum(originalChecksum);

	refcount_ptr<Component> clone = dummy->Clone();

	Checksum cloneChecksum;
	clone->AddChecksum(cloneChecksum);

	UnitTest::Assert("clone should have same checksum",
	    originalChecksum == cloneChecksum);

	dummy->SetVariableValue("name", "\"modified\"");

	Checksum originalChecksumAfterModifyingOriginal;
	dummy->AddChecksum(originalChecksumAfterModifyingOriginal);
	Checksum cloneChecksumAfterModifyingOriginal;
	clone->AddChecksum(cloneChecksumAfterModifyingOriginal);

	UnitTest::Assert("original should have changed checksum",
	    originalChecksum != originalChecksumAfterModifyingOriginal);
	UnitTest::Assert("clone should have same checksum",
	    cloneChecksum == cloneChecksumAfterModifyingOriginal);

	clone->SetVariableValue("name", "\"modified\"");

	Checksum originalChecksumAfterModifyingClone;
	dummy->AddChecksum(originalChecksumAfterModifyingClone);
	Checksum cloneChecksumAfterModifyingClone;
	clone->AddChecksum(cloneChecksumAfterModifyingClone);

	UnitTest::Assert("original should have same checksum, after the "
		"clone has been modified",
	    originalChecksumAfterModifyingClone ==
	    originalChecksumAfterModifyingOriginal);
	UnitTest::Assert("modified clone should have same checksum as "
		"modified original",
	    cloneChecksumAfterModifyingClone ==
	    originalChecksumAfterModifyingOriginal);
}

class DummyComponentWithAllVariableTypes
	: public DummyComponent
{
public:
	DummyComponentWithAllVariableTypes()
		: DummyComponent("dummy2")
	{
		AddVariable("m_string", &m_string);
		AddVariable("m_uint8",  &m_uint8);
		AddVariable("m_uint16", &m_uint16);
		AddVariable("m_uint32", &m_uint32);
		AddVariable("m_uint64", &m_uint64);
		AddVariable("m_sint8",  &m_sint8);
		AddVariable("m_sint16", &m_sint16);
		AddVariable("m_sint32", &m_sint32);
		AddVariable("m_sint64", &m_sint64);
	}

	virtual void ResetState()
	{
		DummyComponent::ResetState();

		m_string = "some value";
		m_uint8 = 123;
		m_uint16 = 0xf9a2;
		m_uint32 = 0x98f8aa01;
		m_uint64 = ((uint64_t)0xf8192929 << 32) | 0x30300a0a;
		m_sint8 = -123;
		m_sint16 = -400;
		m_sint32 = -10000;
		m_sint64 = -42;
	}

	static refcount_ptr<Component> Create()
	{
		return new DummyComponentWithAllVariableTypes();
	}

private:
	string		m_string;
	uint8_t		m_uint8;
	uint16_t	m_uint16;
	uint32_t	m_uint32;
	uint64_t	m_uint64;
	int8_t		m_sint8;
	int16_t		m_sint16;
	int32_t		m_sint32;
	int64_t		m_sint64;
};

static void Test_DummyComponent_Clone_AllVariableTypes()
{
	refcount_ptr<Component> dummy = new DummyComponentWithAllVariableTypes;
	refcount_ptr<Component> dA = new DummyComponentWithAllVariableTypes;
	refcount_ptr<Component> dA1 = new DummyComponentWithAllVariableTypes;
	refcount_ptr<Component> dA2 = new DummyComponentWithAllVariableTypes;

	dA->AddChild(dA1);
	dA->AddChild(dA2);
	dummy->AddChild(dA);

	Checksum originalChecksum;
	dummy->AddChecksum(originalChecksum);

	refcount_ptr<Component> clone = dummy->Clone();

	Checksum cloneChecksum;
	clone->AddChecksum(cloneChecksum);

	UnitTest::Assert("clone should have same checksum",
	    originalChecksum == cloneChecksum);

	dummy->SetVariableValue("name", "\"modified\"");

	Checksum originalChecksumAfterModifyingOriginal;
	dummy->AddChecksum(originalChecksumAfterModifyingOriginal);
	Checksum cloneChecksumAfterModifyingOriginal;
	clone->AddChecksum(cloneChecksumAfterModifyingOriginal);

	UnitTest::Assert("original should have changed checksum",
	    originalChecksum != originalChecksumAfterModifyingOriginal);
	UnitTest::Assert("clone should have same checksum",
	    cloneChecksum == cloneChecksumAfterModifyingOriginal);

	clone->SetVariableValue("name", "\"modified\"");

	Checksum originalChecksumAfterModifyingClone;
	dummy->AddChecksum(originalChecksumAfterModifyingClone);
	Checksum cloneChecksumAfterModifyingClone;
	clone->AddChecksum(cloneChecksumAfterModifyingClone);

	UnitTest::Assert("original should have same checksum, after the "
		"clone has been modified",
	    originalChecksumAfterModifyingClone ==
	    originalChecksumAfterModifyingOriginal);
	UnitTest::Assert("modified clone should have same checksum as "
		"modified original",
	    cloneChecksumAfterModifyingClone ==
	    originalChecksumAfterModifyingOriginal);
}

static void Test_DummyComponent_SerializeDeserialize()
{
	refcount_ptr<Component> dummy = new DummyComponent;
	refcount_ptr<Component> dummyChildA = new DummyComponent;
	refcount_ptr<Component> dummyChildA1 = new DummyComponent;
	refcount_ptr<Component> dummyChildA2 = new DummyComponent;

	dummyChildA->AddChild(dummyChildA1);
	dummyChildA->AddChild(dummyChildA2);
	dummy->AddChild(dummyChildA);

	UnitTest::Assert("serialize/deserialize consistency failure",
	    dummy->CheckConsistency() == true);
}

UNITTESTS(DummyComponent)
{
	ComponentFactory::RegisterComponentClass("dummy2",
	    DummyComponentWithAllVariableTypes::Create,
	    Component::GetAttribute);

	// Creation using CreateComponent
	UNITTEST(Test_DummyComponent_CreateComponent);

	// Parent tests
	UNITTEST(Test_DummyComponent_GetSetParent);

	// Add/Remove children
	UNITTEST(Test_DummyComponent_AddChild_Sets_Parent);
	UNITTEST(Test_DummyComponent_AddChildren_Count);
	UNITTEST(Test_DummyComponent_Add_Tree_Of_Children);
	UNITTEST(Test_DummyComponent_RemoveChild);
	UNITTEST(Test_DummyComponent_AddChild_UniqueName);

	// Path tests
	UNITTEST(Test_DummyComponent_GeneratePath);
	UNITTEST(Test_DummyComponent_LookupPath);
	UNITTEST(Test_DummyComponent_FindPathByPartialMatch);

	// Get state variables
	UNITTEST(Test_DummyComponent_GetUnknownVariable);

	// Clone
	UNITTEST(Test_DummyComponent_Clone_Basic);
	UNITTEST(Test_DummyComponent_Clone_AllVariableTypes);

	// Serialization/deserialization
	UNITTEST(Test_DummyComponent_SerializeDeserialize);
}

#endif

