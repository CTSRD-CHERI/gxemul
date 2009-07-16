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
#include "FileLoader_ELF.h"
#include "thirdparty/exec_elf.h"


FileLoader_ELF::FileLoader_ELF(const string& filename)
	: FileLoaderImpl(filename)
{
}


string FileLoader_ELF::DetectFileType(unsigned char *buf, size_t buflen, float& matchness) const
{
	if (buflen < sizeof(Elf32_Ehdr))
		return "";

	// Note: The e_ident part of the 32-bit and the 64-bit variants have
	// the same layout, so it is safe to only check the 32-bit variant here.
	Elf32_Ehdr* elf32_ehdr = (Elf32_Ehdr*) buf;
	if (elf32_ehdr->e_ident[EI_MAG0] == ELFMAG0 &&
	    elf32_ehdr->e_ident[EI_MAG1] == ELFMAG1 &&
	    elf32_ehdr->e_ident[EI_MAG2] == ELFMAG2 &&
	    elf32_ehdr->e_ident[EI_MAG3] == ELFMAG3) {
		// We are here if this is either an ELF32 or ELF64.
		int elfClass = elf32_ehdr->e_ident[EI_CLASS];

		matchness = 1.0;
		if (elfClass == ELFCLASS32)
			return "ELF32";
		if (elfClass == ELFCLASS64)
			return "ELF64";

		matchness = 0.0;
		stringstream ss;
		ss << "ELF Unknown class " << elfClass;
		return ss.str();
	}

	return "";
}


bool FileLoader_ELF::LoadIntoComponent(refcount_ptr<Component> component) const
{
	AddressDataBus* bus = component->AsAddressDataBus();
	if (bus == NULL)
		return false;

	ifstream file(Filename().c_str());
	if (!file.is_open())
		return false;

	char buf[64];

	// buf must be large enough for the largest possible header we wish
	// to examine to fit.
	assert(sizeof(buf) >= sizeof(Elf32_Ehdr));
	assert(sizeof(buf) >= sizeof(Elf64_Ehdr));

	memset(buf, 0, sizeof(buf));
	file.read(buf, sizeof(buf));

	// Note: The e_ident part of the 32-bit and the 64-bit variants have
	// the same layout, so it is safe to only use the 32-bit variant here.
	Elf32_Ehdr* ehdr32 = (Elf32_Ehdr*) buf;
	Elf64_Ehdr* ehdr64 = (Elf64_Ehdr*) buf;
	if (ehdr32->e_ident[EI_MAG0] != ELFMAG0 ||
	    ehdr32->e_ident[EI_MAG1] != ELFMAG1 ||
	    ehdr32->e_ident[EI_MAG2] != ELFMAG2 ||
	    ehdr32->e_ident[EI_MAG3] != ELFMAG3) {
		// Not an ELF.
		return false;
	}

	int elfClass = ehdr32->e_ident[EI_CLASS];
	int elfDataEncoding = ehdr32->e_ident[EI_DATA];
	int elfVersion = ehdr32->e_ident[EI_VERSION];

	if (elfClass != ELFCLASS32 && elfClass != ELFCLASS64) {
		// Unknown ELF class.
		return false;
	}
	if (elfDataEncoding != ELFDATA2LSB && elfDataEncoding != ELFDATA2MSB) {
		// Unknown ELF data encoding.
		return false;
	}
	if (elfVersion != EV_CURRENT) {
		// Unknown ELF version.
		return false;
	}

	bool elf32 = elfClass == ELFCLASS32;

#define ELF_HEADER_VAR(hdr32,hdr64,type,name) type name = elf32? hdr32->name  \
							       : hdr64->name; \
	if (elfDataEncoding == ELFDATA2LSB) {				      \
		int size = elf32? sizeof(hdr32->name) : sizeof(hdr64->name);  \
		switch (size) {						      \
		case 2:	name = LE16_TO_HOST(name); break;		      \
		case 4:	name = LE32_TO_HOST(name); break;		      \
		case 8:	name = LE64_TO_HOST(name); break;		      \
		}							      \
	} else {							      \
		int size = elf32? sizeof(hdr32->name) : sizeof(hdr64->name);  \
		switch (size) {						      \
		case 2:	name = BE16_TO_HOST(name); break;		      \
		case 4:	name = BE32_TO_HOST(name); break;		      \
		case 8:	name = BE64_TO_HOST(name); break;		      \
		}							      \
	}

	ELF_HEADER_VAR(ehdr32, ehdr64, uint64_t, e_type);

	if (e_type != ET_EXEC) {
		// Only Executables are of interest to us here.
		return false;
	}

	ELF_HEADER_VAR(ehdr32, ehdr64, uint64_t, e_entry);
	ELF_HEADER_VAR(ehdr32, ehdr64, uint64_t, e_machine);
	ELF_HEADER_VAR(ehdr32, ehdr64, uint64_t, e_phoff);
	ELF_HEADER_VAR(ehdr32, ehdr64, uint64_t, e_phentsize);
	ELF_HEADER_VAR(ehdr32, ehdr64, uint64_t, e_phnum);

	for (size_t i=0; i<e_phnum; ++i) {
		// Load Phdr number i:
		file.seekg(e_phoff + i * e_phentsize, std::ios::beg);

		char phdr_buf[64];
		assert(sizeof(phdr_buf) >= sizeof(Elf32_Phdr));
		assert(sizeof(phdr_buf) >= sizeof(Elf64_Phdr));
		Elf32_Phdr* phdr32 = (Elf32_Phdr*) phdr_buf;
		Elf64_Phdr* phdr64 = (Elf64_Phdr*) phdr_buf;

		memset(phdr_buf, 0, sizeof(phdr_buf));
		file.read(phdr_buf, sizeof(phdr_buf));
		
		ELF_HEADER_VAR(phdr32, phdr64, uint64_t, p_type);
		ELF_HEADER_VAR(phdr32, phdr64, uint64_t, p_offset);
		ELF_HEADER_VAR(phdr32, phdr64, uint64_t, p_vaddr);
		ELF_HEADER_VAR(phdr32, phdr64, uint64_t, p_filesz);
		ELF_HEADER_VAR(phdr32, phdr64, uint64_t, p_memsz);

		// Skip non-loadable program segments:		
		if (p_type != PT_LOAD)
			continue;

		file.seekg(p_offset, std::ios::beg);
		char databuf[65536];
		uint64_t bytesRead = 0;
		uint64_t vaddrToWriteTo = p_vaddr;
		while (bytesRead < p_filesz) {
			int sizeToRead = sizeof(databuf);
			if (sizeToRead + bytesRead > p_filesz)
				sizeToRead = p_filesz - bytesRead;

			assert(sizeToRead != 0);
			memset(databuf, 0, sizeToRead);

			file.read(databuf, sizeToRead);
			bytesRead += sizeToRead;

			// Write to the bus, one byte at a time.
			for (int k=0; k<sizeToRead; ++k) {
				bus->AddressSelect(vaddrToWriteTo ++);
				if (!bus->WriteData(databuf[k])) {
					// Failed to write data.
					return false;
				}
			}
		}
	}

	// TODO: Symbols

	// Set the CPU's entry point.
	// Special handling for some architectures: 32-bit MIPS uses
	// sign-extension.
	if (e_machine == EM_MIPS && elf32)
		e_entry = (int32_t) e_entry;

	stringstream ss;
	ss << e_entry;
	component->SetVariableValue("pc", ss.str());
	// TODO: Error handling if there was no "pc"?

	return true;
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

#include "ComponentFactory.h"

static void Test_FileLoader_ELF_Constructor()
{
	FileLoader_ELF elfLoader("test/FileLoader_ELF_MIPS");
}

UNITTESTS(FileLoader_ELF)
{
	UNITTEST(Test_FileLoader_ELF_Constructor);

	// TODO
}

#endif
