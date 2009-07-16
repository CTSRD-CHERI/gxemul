/*
 *  Copyright (C) 2008-2009  Anders Gavare.  All rights reserved.
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
#include <string.h>
#include <fstream>

using std::ifstream;

#include "AddressDataBus.h"
#include "Component.h"
#include "FileLoader.h"
#include "FileLoader_aout.h"
#include "FileLoader_ELF.h"


FileLoader::FileLoader(const string& filename)
	: m_filename(filename)
{
	m_fileLoaders.push_back(new FileLoader_aout(filename));
	m_fileLoaders.push_back(new FileLoader_ELF(filename));
}


const string& FileLoader::GetFilename() const
{
	return m_filename;
}


string FileLoader::DetectFileFormat(refcount_ptr<const FileLoaderImpl>& loader) const
{
	loader = NULL;

	ifstream file(m_filename.c_str());

	// TODO: Disk images?
	// TODO: Raw binaries?
	// Should these be detected by prefixes, as in GXemul 0.4 and below?
	// Like   loadaddr:skiplen:initialpc:filename
	// but how about disk images?

	if (!file.is_open())
		return "Not accessible";

	unsigned char buf[512];

	memset(buf, 0, sizeof(buf));
	file.read((char *)buf, sizeof(buf));
	size_t amountRead = file.gcount();

	// Ask all file loaders about how well they handle the format. Return
	// the format string from the loader that had the highest score.
	float bestMatch = 0.0;
	string bestFormat = "Unknown";
	FileLoaderImplVector::const_iterator it = m_fileLoaders.begin();
	for (; it != m_fileLoaders.end(); ++it) {
		float match;
		string format = (*it)->DetectFileType(buf, amountRead, match);
		if (match > bestMatch) {
			bestMatch = match;
			bestFormat = format;
			loader = *it;
		}
	}

	return bestFormat;
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

	refcount_ptr<const FileLoaderImpl> loaderImpl;
	string fileFormat = DetectFileFormat(loaderImpl);

	if (!loaderImpl.IsNULL())
		return loaderImpl->LoadIntoComponent(component);

#ifndef NDEBUG
	std::cerr << "\n*** FileLoader::Load: File format '" <<
	    fileFormat << "' not yet implemented. TODO\n";
#endif

	return false;
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

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
	refcount_ptr<const FileLoaderImpl> loaderImpl;
	UnitTest::Assert("file format detection failure?",
	    fileLoader.DetectFileFormat(loaderImpl), "Not accessible");
}

static void Test_FileLoader_DetectFileFormat_NonsenseFile()
{
	FileLoader fileLoader("test/FileLoader_NonsenseFile");
	refcount_ptr<const FileLoaderImpl> loaderImpl;
	UnitTest::Assert("file format detection failure?",
	    fileLoader.DetectFileFormat(loaderImpl), "Unknown");
}

static void Test_FileLoader_DetectFileFormat_ELF32()
{
	FileLoader fileLoader("test/FileLoader_ELF_MIPS");
	refcount_ptr<const FileLoaderImpl> loaderImpl;
	UnitTest::Assert("file format detection failure?",
	    fileLoader.DetectFileFormat(loaderImpl), "ELF32");
}

static void Test_FileLoader_DetectFileFormat_ELF64()
{
	FileLoader fileLoader("test/FileLoader_ELF_SH5");
	refcount_ptr<const FileLoaderImpl> loaderImpl;
	UnitTest::Assert("file format detection failure?",
	    fileLoader.DetectFileFormat(loaderImpl), "ELF64");
}

static void Test_FileLoader_DetectFileFormat_aout_88K()
{
	FileLoader fileLoader("test/FileLoader_A.OUT_M88K");
	refcount_ptr<const FileLoaderImpl> loaderImpl;
	UnitTest::Assert("file format detection failure?",
	    fileLoader.DetectFileFormat(loaderImpl), "a.out_M88K_fromBeginning");
}

static refcount_ptr<Component> SetupTestMachineAndLoad(
	string machineName, string fileName)
{
	FileLoader fileLoader(fileName);
	refcount_ptr<Component> machine =
	    ComponentFactory::CreateComponent(machineName);

	machine->SetVariableValue("name", "\"machine\"");
	refcount_ptr<Component> component =
	    machine->LookupPath("machine.mainbus0.cpu0");
	UnitTest::Assert("could not look up CPU to load into?",
	    !component.IsNULL());

	UnitTest::Assert("could not load the file " + fileName + " for"
	    " machine " + machineName, fileLoader.Load(component));

	return machine;
}

static void Test_FileLoader_Load_ELF32()
{
	refcount_ptr<Component> machine =
	    SetupTestMachineAndLoad("testmips", "test/FileLoader_ELF_MIPS");

	// Read from CPU, to make sure the file was loaded:
	refcount_ptr<Component> cpu =
	    machine->LookupPath("machine.mainbus0.cpu0");
	AddressDataBus * bus = cpu->AsAddressDataBus();
	bus->AddressSelect((int32_t)0x80010000);
	uint32_t word = 0x12345678;
	bus->ReadData(word, BigEndian);
	UnitTest::Assert("memory (CPU) wasn't filled with data from the file?",
	    word, 0x8f8b8008);

	// Read directly from RAM too, to make sure the file was loaded:
	refcount_ptr<Component> ram =
	    machine->LookupPath("machine.mainbus0.ram0");
	AddressDataBus * ramBus = ram->AsAddressDataBus();
	ramBus->AddressSelect(0x1000c);
	uint32_t word2 = 0x12345678;
	ramBus->ReadData(word2, BigEndian);
	UnitTest::Assert("memory (RAM) wasn't filled with data from the file?",
	    word2, 0x006b7021);
}

static void Test_FileLoader_Load_aout()
{
	refcount_ptr<Component> machine =
	    SetupTestMachineAndLoad("testm88k", "test/FileLoader_A.OUT_M88K");

	// Read from CPU, to make sure the file was loaded:
	refcount_ptr<Component> cpu =
	    machine->LookupPath("machine.mainbus0.cpu0");
	AddressDataBus * bus = cpu->AsAddressDataBus();
	bus->AddressSelect((int32_t)0x12b8);
	uint32_t word = 0x12345678;
	bus->ReadData(word, BigEndian);
	UnitTest::Assert("memory (CPU) wasn't filled with data from the file?",
	    word, 0x67ff0020);

	// Read directly from RAM too, to make sure the file was loaded:
	refcount_ptr<Component> ram =
	    machine->LookupPath("machine.mainbus0.ram0");
	AddressDataBus * ramBus = ram->AsAddressDataBus();
	ramBus->AddressSelect(0x12e0);
	uint32_t word2 = 0xfdecba98;
	ramBus->ReadData(word2, BigEndian);
	UnitTest::Assert("memory (RAM) wasn't filled with data from the file?",
	    word2, 0xf6c05802);
}

UNITTESTS(FileLoader)
{
	UNITTEST(Test_FileLoader_Constructor);
	UNITTEST(Test_FileLoader_Constructor_NonExistingFile);
	
	UNITTEST(Test_FileLoader_DetectFileFormat_NonexistingFile);
	UNITTEST(Test_FileLoader_DetectFileFormat_NonsenseFile);
	UNITTEST(Test_FileLoader_DetectFileFormat_ELF32);
	UNITTEST(Test_FileLoader_DetectFileFormat_ELF64);
	UNITTEST(Test_FileLoader_DetectFileFormat_aout_88K);

	UNITTEST(Test_FileLoader_Load_ELF32);
	UNITTEST(Test_FileLoader_Load_aout);
}

#endif
