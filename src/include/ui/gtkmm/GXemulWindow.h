#ifndef GXEMULWINDOW_H
#define	GXEMULWINDOW_H

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
 *  $Id: GXemulWindow.h,v 1.1 2007/12/31 11:50:18 debug Exp $
 */

#include <gtkmm.h>

#include "GXemul.h"
#include "ui/gtkmm/DebugConsoleWidget.h"
#include "ui/gtkmm/EmulationDesignArea.h"


/**
 * \brief The main GUI window.
 */
class GXemulWindow
	: public Gtk::Window
{
public:
	GXemulWindow(GXemul* gxemul);
	virtual ~GXemulWindow();

private:
	virtual void on_menu_about();
	virtual void on_menu_new();
	virtual void on_menu_quit();

private:
	GXemul*		m_gxemul;

	Gtk::VBox	m_Box;
	Gtk::VPaned	m_VPaned;

	EmulationDesignArea m_EmulationDesignArea;
	DebugConsoleWidget m_DebugConsoleWidget;

	Glib::RefPtr<Gtk::UIManager> m_refUIManager;
	Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;
};

#endif	// GXEMULWINDOW_H
