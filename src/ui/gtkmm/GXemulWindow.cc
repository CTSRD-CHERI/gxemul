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
 *  $Id: GXemulWindow.cc,v 1.3 2008/01/02 10:56:41 debug Exp $
 */

#ifdef WITH_GTKMM

#include <iostream>

#include "misc.h"
#include "ui/gtkmm/GXemulWindow.h"


GXemulWindow::GXemulWindow(GXemul* gxemul)
	: m_gxemul(gxemul)
{
	set_title("GXemul");

	add(m_Box);

	m_refActionGroup = Gtk::ActionGroup::create();

	// File menu:
	m_refActionGroup->add(Gtk::Action::create("FileMenu", "_File"));
	m_refActionGroup->add(Gtk::Action::create("FileNew", Gtk::Stock::NEW),
	    sigc::mem_fun(*this, &GXemulWindow::on_menu_new));
	m_refActionGroup->add(Gtk::Action::create("FileQuit", Gtk::Stock::QUIT),
	    sigc::mem_fun(*this, &GXemulWindow::on_menu_quit));

	// Help menu:
	m_refActionGroup->add( Gtk::Action::create("HelpMenu", "_Help") );
	m_refActionGroup->add( Gtk::Action::create("HelpAbout",
	    Gtk::Stock::HELP), sigc::mem_fun(*this,
	    &GXemulWindow::on_menu_about) );

	m_refUIManager = Gtk::UIManager::create();
	m_refUIManager->insert_action_group(m_refActionGroup);

	add_accel_group(m_refUIManager->get_accel_group());

	Glib::ustring ui_info = 
	    "<ui>"
	    "  <menubar name='MenuBar'>"
	    "    <menu action='FileMenu'>"
	    "      <menuitem action='FileNew'/>"
	    "      <separator/>"
	    "      <menuitem action='FileQuit'/>"
	    "    </menu>"
	    "    <menu action='HelpMenu'>"
	    "      <menuitem action='HelpAbout'/>"
	    "    </menu>"
	    "  </menubar>"
	    "</ui>";

#ifdef GLIBMM_EXCEPTIONS_ENABLED
	try {
		m_refUIManager->add_ui_from_string(ui_info);
	} catch(const Glib::Error& ex) {
		std::cerr << "building menus failed: " <<  ex.what();
	}
#else
	auto_ptr<Glib::Error> ex;
	m_refUIManager->add_ui_from_string(ui_info, ex);
	if(ex.get()) {
		std::cerr << "building menus failed: " <<  ex->what();
	}
#endif //GLIBMM_EXCEPTIONS_ENABLED

	Gtk::Widget* pMenubar = m_refUIManager->get_widget("/MenuBar");
	if (pMenubar != NULL)
		m_Box.pack_start(*pMenubar, Gtk::PACK_SHRINK);

	m_Box.add(m_VPaned);

	m_VPaned.pack1(m_EmulationDesignArea);
	m_VPaned.pack2(m_DebugConsoleWidget);

	show_all_children();
}

GXemulWindow::~GXemulWindow()
{
}

void GXemulWindow::on_menu_about()
{
	Gtk::MessageDialog dialog(*this, "GXemul "VERSION,
	    false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK);
	dialog.set_secondary_text(_(COPYRIGHT_MSG"\n"SECONDARY_MSG"\n"
	    "If you have questions or feedback, don't "
	    "hesitate to mail me.\nanders@gavare.se"));
	dialog.run();
}

void GXemulWindow::on_menu_new()
{
	// TODO
}

void GXemulWindow::on_menu_quit()
{
	hide();
}

#endif	// WITH_GTKMM
