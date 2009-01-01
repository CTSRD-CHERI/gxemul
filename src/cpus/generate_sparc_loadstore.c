/*
 *  Copyright (C) 2006-2009  Anders Gavare.  All rights reserved.
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

#include <stdio.h>
#include <string.h>


void print_function_name(int use_imm, int store, int size, int signedness)
{
	if (use_imm)
		printf("i");
	if (store)
		printf("s");
	else {
		printf("l");
		if (!signedness)
			printf("u");
	}
	printf("%i", 1 << size);
}


void loadstore(int mode32, int use_imm, int store, int size, int signedness)
{
	if (store && signedness)
		return;

	printf("#if%sdef MODE32\n", mode32? "" : "n");

	if (use_imm)
		printf("#define LS_USE_IMM\n");

	if (store)
		printf("#define LS_STORE\n");
	else
		printf("#define LS_LOAD\n");

	printf("#define LS_N sparc%s_instr_", mode32? "32" : "");
	print_function_name(use_imm, store, size, signedness);
	printf("\n");

	printf("#define LS_GENERIC_N sparc%s_generic_", mode32? "32" : "");
	print_function_name(use_imm, store, size, signedness);
	printf("\n");

	printf("#define LS_%i\n", 1 << size);
	printf("#define LS_SIZE %i\n", 1 << size);

	if (signedness && !store)
		printf("#define LS_SIGNED\n");

	printf("#include \"cpu_sparc_instr_loadstore.c\"\n");

	if (signedness && !store)
		printf("#undef LS_SIGNED\n");

	printf("#undef LS_SIZE\n");
	printf("#undef LS_%i\n", 1 << size);

	printf("#undef LS_GENERIC_N\n");
	printf("#undef LS_N\n");

	if (store)
		printf("#undef LS_STORE\n");
	else
		printf("#undef LS_LOAD\n");

	if (use_imm)
		printf("#undef LS_USE_IMM\n");

	printf("#endif\n");
}


int main(int argc, char *argv[])
{
	int store, mode32, size, signedness, use_imm;

	printf("\n/*  AUTOMATICALLY GENERATED! Do not edit.  */\n\n");

	for (mode32=0; mode32<=1; mode32++)
	   for (use_imm=0; use_imm<=1; use_imm++)
	      for (store=0; store<=1; store++)
		for (size=0; size<=3; size++)
		    for (signedness=0; signedness<=1; signedness++)
			loadstore(mode32, use_imm, store, size, signedness);

	/*  Array of pointers to fast load/store functions:  */
	for (mode32=0; mode32<=1; mode32++) {
		printf("#if%sdef MODE32\n", mode32? "" : "n");
		printf("\n\nvoid (*sparc%s_loadstore[32])(struct cpu *, struct "
		    "sparc_instr_call *) = {\n", mode32? "32" : "");
		for (use_imm=0; use_imm<=1; use_imm++)
		   for (store=0; store<=1; store++)
		     for (size=0; size<=3; size++)
			for (signedness=0; signedness<=1; signedness++) {
				if (store || size || signedness || use_imm)
					printf(",\n");

				if (store && signedness) {
					printf("\tsparc%s_instr_invalid",
					    mode32? "32" : "");
					continue;
				}

				printf("\tsparc%s_instr_", mode32? "32" : "");
				print_function_name(use_imm, store,
				    size, signedness);
			}
		printf(" };\n");
		printf("#endif\n");
	}

	return 0;
}

