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

	/**
	 * \brief Update volatile UI components.
	 *
	 * Components updated include Undo/Redo button sensibility,
	 * the main window's title, etc.
	 *
	 * Should be called from the GtkmmUI instance.
	 */
	void UpdateUI();

	/**
	 * \brief Add a message to the debug window text output
	 * area.
	 *
	 * \param msg The message to add.
	 */
	void InsertText(const string& msg);

private:
	virtual void on_menu_about();
	virtual void on_menu_copy();
	virtual void on_menu_cut();
	virtual void on_menu_delete();
	virtual void on_menu_go();
	virtual void on_menu_new_blank();
	virtual void on_menu_new_from_template();
	virtual void on_menu_open();
	virtual void on_menu_paste();
	virtual void on_menu_pause();
	virtual void on_menu_preferences();
	virtual void on_menu_quit();
	virtual void on_menu_redo();
	virtual void on_menu_reset();
	virtual void on_menu_save();
	virtual void on_menu_save_as();
	virtual void on_menu_undo();

private:
	GXemul*		m_gxemul;

	Gtk::VBox	m_Box;

	EmulationDesignArea m_EmulationDesignArea;
	DebugConsoleWidget m_DebugConsoleWidget;

	Glib::RefPtr<Gtk::UIManager> m_refUIManager;
	Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;
};

#endif	// GXEMULWINDOW_H
