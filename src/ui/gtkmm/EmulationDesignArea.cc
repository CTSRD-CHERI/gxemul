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

#ifdef WITH_GTKMM

#include "ui/gtkmm/EmulationDesignArea.h"
#include "GXemul.h"


EmulationDesignArea::EmulationDesignArea(GXemul* gxemul)
	: m_gxemul(gxemul)
{
}


EmulationDesignArea::~EmulationDesignArea()
{
}


void EmulationDesignArea::DrawComponentAndChildren(
	Cairo::RefPtr<Cairo::Context> cr,
	int x1, int y1, int x2, int y2,
	refcount_ptr<Component> component)
{
	Components components = component->GetChildren();

	// TODO: Draw something meaningful!

	int lesser = MIN(x2-x1, y2-y1);
	int xc = (x2 + x1) / 2;
	int yc = (y2 + y1) / 2;
	if (components.size() > 0)
		yc = y1 + (y2-y1) / 6;

	// Draw the component:
	cr->save();
	cr->set_line_width(3);
	cr->arc(xc, yc, lesser/6.5, 0.0, 2.0 * M_PI);
	cr->set_source_rgba(0.0, 0.0, 1.0, 0.5);
	cr->fill_preserve();
	cr->restore();
	cr->stroke();

	// Now, draw all children:
	for (size_t i=0; i<components.size(); ++i) {
		int cx1 = x1 + i * (x2-x1) / components.size();
		int cx2 = x1 + (i+1) * (x2-x1) / components.size() - 1;

		int cy1 = y1 + (y2-y1) / 3;
		int cy2 = y2;

		DrawComponentAndChildren(cr, cx1, cy1, cx2, cy2, components[i]);
	}
}


bool EmulationDesignArea::on_expose_event(GdkEventExpose* event)
{
	Glib::RefPtr<Gdk::Window> window = get_window();
	if (!window)
		return false;

	Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();

	// Clip to the expose event area:
	cr->rectangle(event->area.x, event->area.y,
	    event->area.width, event->area.height);
	cr->clip();

	Gtk::Allocation allocation = get_allocation();
	const int width = allocation.get_width();
	const int height = allocation.get_height();

	// Draw all components:
	Components components = m_gxemul->GetRootComponent()->GetChildren();
	for (size_t i=0; i<components.size(); ++i) {
		int x1 = i * width / components.size();
		int x2 = (i+1) * width / components.size() - 1;
		int y1 = 0, y2 = height - 1;

		DrawComponentAndChildren(cr, x1, y1, x2, y2, components[i]);
	}

	return true;
}


#endif	// WITH_GTKMM
