/*
 *  Copyright (C) 2003-2009  Anders Gavare.  All rights reserved.
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
 *  This file contains three things:
 *
 *	1. Doxygen documentation for the general design concepts of the
 *	   emulator. The mainpage documentation written here ends up on
 *	   the "main page" in the generated HTML documentation.
 *
 *	2. The main() entry point.
 */


/*! \mainpage Source code documentation
 *
 * \section intro_sec Introduction
 *
 * This is the automatically generated Doxygen documentation for %GXemul,
 * built from comments throughout the source code.
 *
 * See the <a href="../../index.html">main documentation</a> for more
 * information about this version of %GXemul.
 *
 * See GXemul's home page for more information about %GXemul in general:
 * <a href="http://gavare.se/gxemul/">http://gavare.se/gxemul/</a>
 *
 * <b>TODO</b>
 */


/*****************************************************************************/

#include <stdio.h>
#include "../../config.h"


/*  main.c:  */
extern "C" int old_main(int argc, char *argv[]);


#ifdef I18N
#include <libintl.h>
#define I18N_PACKAGE_NAME	"gxemul"
#endif


/**
 * \brief Program entry point.
 */
int main(int argc, char *argv[])
{
	const char *progname = argv[0];

#ifdef I18N
	// Initialize gettext, for Internationalization support:
	bindtextdomain(I18N_PACKAGE_NAME, NULL);
	bind_textdomain_codeset(I18N_PACKAGE_NAME, "UTF-8");
	textdomain(I18N_PACKAGE_NAME);
#endif

	return old_main(argc, argv);
}

