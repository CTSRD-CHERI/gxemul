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
 *  $Id: EmulationDesignArea.cc,v 1.1 2007/12/31 11:50:19 debug Exp $
 */

#ifdef WITH_GTKMM

#include "ui/gtkmm/EmulationDesignArea.h"


EmulationDesignArea::EmulationDesignArea()
{
        set_size_request(800, 550);
}

EmulationDesignArea::~EmulationDesignArea()
{
}

bool EmulationDesignArea::on_expose_event(GdkEventExpose* event)
{
  // This is where we draw on the window
  Glib::RefPtr<Gdk::Window> window = get_window();
  if(window)
  {
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();

    double x0=0.1, y0=0.5, // start point
           x1=0.4, y1=0.9,  // control point #1
           x2=0.6, y2=0.1,  // control point #2
           x3=0.9, y3=0.5;  // end point

    Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
    // clip to the area indicated by the expose event so that we only redraw
    // the portion of the window that needs to be redrawn
    cr->rectangle(event->area.x, event->area.y,
            event->area.width, event->area.height);
    cr->clip();

    // scale to unit square (0 to 1 with and height)
    cr->scale(width, height);

    cr->set_line_width(0.05);
    // draw curve
    cr->move_to(x0, y0);
    cr->curve_to(x1, y1, x2, y2, x3, y3);
    cr->stroke();
    // show control points
    cr->set_source_rgba(1, 0.2, 0.2, 0.6);
    cr->move_to(x0, y0);
    cr->line_to (x1, y1);
    cr->move_to(x2, y2);
    cr->line_to (x3, y3);
    cr->stroke();
  }
  return true;
}

#endif	// WITH_GTKMM
