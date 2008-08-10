#ifndef	MISC_H
#define	MISC_H

/*
 *  Copyright (C) 2003-2008  Anders Gavare.  All rights reserved.
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
 *  Misc. definitions for GXemul.
 */


#include <sys/types.h>
#include <inttypes.h>


// config.h contains #defines set by the configure script.
#include "../../config.h"

#define	COPYRIGHT_MSG	"Copyright (C) 2003-2008  Anders Gavare"

// The recommended way to add a specific message to the startup banner or
// about box is to use the SECONDARY_MSG. This should end with a newline
// character, unless it is completely empty.
//
// Example:  "Modified by XYZ to include support for machine type UVW.\n"
//
#define	SECONDARY_MSG	""


// Use Glib::ustring if available, otherwise std::string. Define
// stringchar to be the type of a character.
#ifdef WITH_GTKMM
#include <glibmm/ustring.h>
typedef Glib::ustring string;
typedef gunichar stringchar;
#else   // !WITH_GTKMM
#include <string>
typedef std::string string;
typedef char stringchar;
#endif

// Use Glib's I18N support if available
#ifdef WITH_GTKMM
#include <glibmm/i18n.h>
#else   // !WITH_GTKMM
#define	_(x)	(x)
#endif

#include <iostream>
using std::ostream;

#include <memory>
using std::auto_ptr;

#include <list>
using std::list;

#include <map>
using std::map;
using std::multimap;

#include <set>
using std::set;

#include <sstream>
using std::stringstream;

#include <vector>
using std::vector;

using std::min;
using std::max;
using std::pair;


// Generic and vector-specific foreach, which work the way I want it.
// (STL's for_each and mem_fun behave differently.)
#define foreach_item(containertype,container,func) {	\
	containertype::iterator _it; \
	for (_it=container.begin(); _it!=container.end(); ++_it) (func)(*_it); }
#define foreach_vec(container,func) { for (size_t _i=0, _n=(container).size(); \
	_i<_n; ++_i) (func)((container)[_i]); }


#ifndef NDEBUG
#include "thirdparty/debug_new.h"
#endif


// Reference counting is needed in lots of places, so it is best to
// include it from this file.
#include "refcount_ptr.h"  


#ifdef NO_C99_PRINTF_DEFINES
//
// This is a SUPER-UGLY HACK which happens to work on some machines.
// The correct solution is to upgrade your compiler to C99.
//
#ifdef NO_C99_64BIT_LONGLONG
#define	PRIi8		"i"
#define	PRIi16		"i"
#define	PRIi32		"i"
#define	PRIi64		"lli"
#define	PRIx8		"x"
#define	PRIx16		"x"
#define	PRIx32		"x"
#define	PRIx64		"llx"
#else
#define	PRIi8		"i"
#define	PRIi16		"i"
#define	PRIi32		"i"
#define	PRIi64		"li"
#define	PRIx8		"x"
#define	PRIx16		"x"
#define	PRIx32		"x"
#define	PRIx64		"lx"
#endif
#endif


#ifdef NO_MAP_ANON
#error mmap for systems without MAP_ANON has not yet been implemented. \
	The old implementation was too ugly. Please let me know about this \
	if you see this error message. (Most likely IRIX systems.)
#endif


enum Endianness
{
	BigEndian = 0,
	LittleEndian
};


#ifdef HOST_LITTLE_ENDIAN
#define	LE16_TO_HOST(x)	    (x)
#define	BE16_TO_HOST(x)	    ((((x) & 0xff00) >> 8) | (((x)&0xff) << 8))
#else
#define	LE16_TO_HOST(x)	    ((((x) & 0xff00) >> 8) | (((x)&0xff) << 8))
#define	BE16_TO_HOST(x)	    (x)
#endif

#ifdef HOST_LITTLE_ENDIAN
#define	LE32_TO_HOST(x)	    (x)
#define	BE32_TO_HOST(x)	    ((((x) & 0xff000000) >> 24) | (((x)&0xff) << 24) | \
			     (((x) & 0xff0000) >> 8) | (((x) & 0xff00) << 8))
#else
#define	LE32_TO_HOST(x)	    ((((x) & 0xff000000) >> 24) | (((x)&0xff) << 24) | \
			     (((x) & 0xff0000) >> 8) | (((x) & 0xff00) << 8))
#define	BE32_TO_HOST(x)	    (x)
#endif

#ifdef HOST_LITTLE_ENDIAN
#define	LE64_TO_HOST(x)	    (x)
#define BE64_TO_HOST(x)	    (	(((x) >> 56) & 0xff) +			\
				((((x) >> 48) & 0xff) << 8) +		\
				((((x) >> 40) & 0xff) << 16) +		\
				((((x) >> 32) & 0xff) << 24) +		\
				((((x) >> 24) & 0xff) << 32) +		\
				((((x) >> 16) & 0xff) << 40) +		\
				((((x) >> 8) & 0xff) << 48) +		\
				(((x) & 0xff) << 56)  )
#else
#define	BE64_TO_HOST(x)	    (x)
#define LE64_TO_HOST(x)	    (	(((x) >> 56) & 0xff) +			\
				((((x) >> 48) & 0xff) << 8) +		\
				((((x) >> 40) & 0xff) << 16) +		\
				((((x) >> 32) & 0xff) << 24) +		\
				((((x) >> 24) & 0xff) << 32) +		\
				((((x) >> 16) & 0xff) << 40) +		\
				((((x) >> 8) & 0xff) << 48) +		\
				(((x) & 0xff) << 56)  )
#endif


#ifdef HAVE___FUNCTION__

#define	FAILURE(error_msg)					{	\
		char where_msg[400];					\
		snprintf(where_msg, sizeof(where_msg),			\
		    "%s, line %i, function %s().\n",			\
		    __FILE__, __LINE__, __FUNCTION__);			\
        	fprintf(stderr, "\n%s, in %s\n", error_msg, where_msg);	\
		exit(1);						\
	}

#else

#define	FAILURE(error_msg)					{	\
		char where_msg[400];					\
		snprintf(where_msg, sizeof(where_msg),			\
		    "%s, line %i\n", __FILE__, __LINE__);		\
        	fprintf(stderr, "\n%s, in %s.\n", error_msg, where_msg);\
		exit(1);						\
	}

#endif	/*  !HAVE___FUNCTION__  */



#endif	/*  MISC_H  */
