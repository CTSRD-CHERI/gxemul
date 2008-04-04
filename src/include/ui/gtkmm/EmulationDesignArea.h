#ifndef EMULATIONDESIGNAREA_H
#define	EMULATIONDESIGNAREA_H

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

#include <gtkmm.h>

#include "misc.h"
#include "Component.h"

class GXemul;

/**
 * \brief The main emulation design area.
 *
 * TODO: Longer description.
 */
class EmulationDesignArea : public Gtk::DrawingArea
{
public:
	/**
	 * \brief Constructs an %EmulationDesignArea.
	 *
	 * \param gxemul The owner GXemul instance.
	 */
	EmulationDesignArea(GXemul* gxemul);

	virtual ~EmulationDesignArea();

protected:
	virtual bool on_expose_event(GdkEventExpose* event);

private:
	void DrawComponentAndChildren(
	    Cairo::RefPtr<Cairo::Context> cr,
	    int x1, int y1, int x2, int y2,
	    refcount_ptr<Component> component);

private:
	GXemul *	m_gxemul;
};

#endif	// EMULATIONDESIGNAREA_H
