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
 */

#include <assert.h>
#include <fstream>

using std::ifstream;


#include "AddressDataBus.h"
#include "Component.h"
#include "FileLoader.h"
#include "FileLoader_ELF.h"
#include "thirdparty/exec_elf.h"


FileLoader::FileLoader(const string& filename)
	: m_filename(filename)
{
}


const string& FileLoader::GetFilename() const
{
	return m_filename;
}


string FileLoader::DetectFileFormat() const
{
	ifstream file(m_filename.c_str());
	if (!file.is_open())
		return "Not accessible";

	char buf[256];

	// buf must be large enough for the largest possible header we wish
	// to examine to fit.
	assert(sizeof(buf) >= sizeof(Elf32_Ehdr));
	assert(sizeof(buf) >= sizeof(Elf64_Ehdr));

	memset(buf, 0, sizeof(buf));
	file.read(buf, sizeof(buf));

	// Try ELF first.
	// Note: The e_ident part of the 32-bit and the 64-bit variants have
	// the same layout, so it is safe to only check the 32-bit variant here.
	Elf32_Ehdr* elf32_ehdr = (Elf32_Ehdr*) buf;
	if (elf32_ehdr->e_ident[EI_MAG0] == ELFMAG0 &&
	    elf32_ehdr->e_ident[EI_MAG1] == ELFMAG1 &&
	    elf32_ehdr->e_ident[EI_MAG2] == ELFMAG2 &&
	    elf32_ehdr->e_ident[EI_MAG3] == ELFMAG3) {
		// We are here if this is either an ELF32 or ELF64.
		int elfClass = elf32_ehdr->e_ident[EI_CLASS];

		if (elfClass == ELFCLASS32)
			return "ELF32";
		if (elfClass == ELFCLASS64)
			return "ELF64";

		stringstream ss;
		ss << "ELF Unknown class " << elfClass;
		return ss.str();
	}

	// TODO: Various a.out formats. Compare with GXemul 0.4.x.
	
	// TODO: Raw binaries?

	return "Unknown";
}


bool FileLoader::Load(refcount_ptr<Component> component) const
{
	AddressDataBus * bus = component->AsAddressDataBus();
	if (bus == NULL) {
		// We cannot load into something that isn't an AddressDataBus.
		// TODO: Better error reporting!
#ifndef NDEBUG
		std::cerr << "\n*** FileLoader::Load: " <<
		    component->GeneratePath() << " is not an "
		    "AddressDataBus. TODO: Better error reporting.\n";
#endif
		return false;
	}

	string fileFormat = DetectFileFormat();

	if (fileFormat == "ELF32" || fileFormat == "ELF64") {
		FileLoader_ELF elfLoader(m_filename);
		return elfLoader.LoadIntoBus(bus);
	}

	// TODO: Other formats!

#ifndef NDEBUG
	std::cerr << "\n*** FileLoader::Load: File format '" <<
	    fileFormat << "' not yet implemented. TODO\n";
#endif

	return false;
}


/*****************************************************************************/


#ifndef WITHOUTUNITTESTS

#include "ComponentFactory.h"

static void Test_FileLoader_Constructor()
{
	FileLoader fileLoader("test/FileLoader_ELF_MIPS");
	UnitTest::Assert("filename mismatch?",
	    fileLoader.GetFilename(), "test/FileLoader_ELF_MIPS");
}

static void Test_FileLoader_Constructor_NonExistingFile()
{
	FileLoader fileLoader("test/Nonexisting");
	UnitTest::Assert("filename mismatch?",
	    fileLoader.GetFilename(), "test/Nonexisting");
}

static void Test_FileLoader_DetectFileFormat_NonexistingFile()
{
	FileLoader fileLoader("test/Nonexisting");
	UnitTest::Assert("file format detection failure?",
	    fileLoader.DetectFileFormat(), "Not accessible");
}

static void Test_FileLoader_DetectFileFormat_NonsenseFile()
{
	FileLoader fileLoader("test/FileLoader_NonsenseFile");
	UnitTest::Assert("file format detection failure?",
	    fileLoader.DetectFileFormat(), "Unknown");
}

static void Test_FileLoader_DetectFileFormat_ELF32()
{
	FileLoader fileLoader("test/FileLoader_ELF_MIPS");
	UnitTest::Assert("file format detection failure?",
	    fileLoader.DetectFileFormat(), "ELF32");
}

static void Test_FileLoader_DetectFileFormat_ELF64()
{
	FileLoader fileLoader("test/FileLoader_ELF_SH5");
	UnitTest::Assert("file format detection failure?",
	    fileLoader.DetectFileFormat(), "ELF64");
}

static void Test_FileLoader_Load_ELF32()
{
	FileLoader fileLoader("test/FileLoader_ELF_MIPS");
	refcount_ptr<Component> machine =
	    ComponentFactory::CreateComponent("testmips");

	machine->SetVariableValue("name", "machine");
	refcount_ptr<Component> component =
	    machine->LookupPath("machine.mainbus0.mips_cpu0");
	UnitTest::Assert("could not look up CPU to load into?",
	    !component.IsNULL());

	UnitTest::Assert("could not load the file?",
	    fileLoader.Load(component));

	// Read from CPU, to make sure the file was loaded:
	AddressDataBus * bus = component->AsAddressDataBus();
	bus->AddressSelect((int32_t)0x80010000);
	uint32_t word = 0x12345678;
	bus->ReadData(word, BigEndian);
	UnitTest::Assert("memory (CPU) wasn't filled with data from the file?",
	    word, 0x8f8b8008);

	// Read directly from RAM too, to make sure the file was loaded:
	refcount_ptr<Component> ram =
	    machine->LookupPath("machine.mainbus0.ram0");
	AddressDataBus * ramBus = component->AsAddressDataBus();
	ramBus->AddressSelect(0x1000c);
	uint32_t word2 = 0x12345678;
	ramBus->ReadData(word2, BigEndian);
	UnitTest::Assert("memory (RAM) wasn't filled with data from the file?",
	    word2, 0x006b7021);
}

UNITTESTS(FileLoader)
{
	UNITTEST(Test_FileLoader_Constructor);
	UNITTEST(Test_FileLoader_Constructor_NonExistingFile);
	
	UNITTEST(Test_FileLoader_DetectFileFormat_NonexistingFile);
	UNITTEST(Test_FileLoader_DetectFileFormat_NonsenseFile);
	UNITTEST(Test_FileLoader_DetectFileFormat_ELF32);
	UNITTEST(Test_FileLoader_DetectFileFormat_ELF64);

	UNITTEST(Test_FileLoader_Load_ELF32);
}

#endif
