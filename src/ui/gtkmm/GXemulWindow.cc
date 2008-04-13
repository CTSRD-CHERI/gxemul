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

#include <gtkmm.h>

#include <iostream>

#include "misc.h"
#include "ui/gtkmm/GXemulWindow.h"


GXemulWindow::GXemulWindow(GXemul* gxemul)
	: m_gxemul(gxemul)
	, m_EmulationDesignArea(gxemul)
	, m_DebugConsoleWidget(gxemul)
	, m_updating(false)
{
	set_title("GXemul");

	add(m_Box);

	m_refActionGroup = Gtk::ActionGroup::create();

	// File|New sub menu:
	m_refActionGroup->add(Gtk::Action::create("FileNewBlack",
	    Gtk::Stock::NEW, _("_Blank emulation"),
	    _("Create a new blank emulation")),
	    sigc::mem_fun(*this, &GXemulWindow::on_menu_new_blank));
	m_refActionGroup->add(Gtk::Action::create("FileNewFromTemplate",
	    _("From _template"), _("Create a new emulation from a template")),
	    sigc::mem_fun(*this, &GXemulWindow::on_menu_new_from_template));

	// File menu:
	m_refActionGroup->add(Gtk::Action::create("FileMenu", _("_File")));
	m_refActionGroup->add(Gtk::Action::create("FileNew", _("_New")));
	m_refActionGroup->add(Gtk::Action::create("FileOpen", Gtk::Stock::OPEN,
	     _("_Open"), _("Open an emulation setup")),
	    sigc::mem_fun(*this, &GXemulWindow::on_menu_open));
	m_refActionGroup->add(Gtk::Action::create("FileSave", Gtk::Stock::SAVE,
	    _("_Save"), _("Save the emulation setup")),
	    sigc::mem_fun(*this, &GXemulWindow::on_menu_save));
	m_refActionGroup->add(Gtk::Action::create("FileSaveAs",
	    Gtk::Stock::SAVE_AS, _("Save _as"),
	    _("Save the emulation setup under a different filename")),
	    sigc::mem_fun(*this, &GXemulWindow::on_menu_save_as));
	m_refActionGroup->add(Gtk::Action::create("FileQuit", Gtk::Stock::QUIT,
	    _("_Quit"), _("Quit the application")),
	    sigc::mem_fun(*this, &GXemulWindow::on_menu_quit));

	// Edit menu:
	m_refActionGroup->add(Gtk::Action::create("EditMenu", _("_Edit")));
	m_refActionGroup->add(Gtk::Action::create("EditUndo", Gtk::Stock::UNDO,
	    _("_Undo"), _("Undo the last action")),
	    sigc::mem_fun(*this, &GXemulWindow::on_menu_undo));
	m_refActionGroup->add(Gtk::Action::create("EditRedo", Gtk::Stock::REDO,
	    _("_Redo"), _("Redo the next action, if possible")),
	    sigc::mem_fun(*this, &GXemulWindow::on_menu_redo));
	m_refActionGroup->add(Gtk::Action::create("EditCut", Gtk::Stock::CUT),
	    sigc::mem_fun(*this, &GXemulWindow::on_menu_cut));
	m_refActionGroup->add(Gtk::Action::create("EditCopy", Gtk::Stock::COPY),
	    sigc::mem_fun(*this, &GXemulWindow::on_menu_copy));
	m_refActionGroup->add(Gtk::Action::create("EditPaste",
	    Gtk::Stock::PASTE),
	    sigc::mem_fun(*this, &GXemulWindow::on_menu_paste));
	m_refActionGroup->add(Gtk::Action::create("EditDelete",
	    Gtk::Stock::DELETE),
	    sigc::mem_fun(*this, &GXemulWindow::on_menu_delete));
	m_refActionGroup->add(Gtk::Action::create("EditPreferences",
	    Gtk::Stock::PREFERENCES, _("Pre_ferences")),
	    sigc::mem_fun(*this, &GXemulWindow::on_menu_preferences));

	// Emulation menu:
	m_toggleActionGo = Gtk::ToggleAction::create("EmulationGo",
	    Gtk::Stock::MEDIA_PLAY, _("_Run"), _("Run the emulation"));
	m_toggleActionPause = Gtk::ToggleAction::create("EmulationPause",
	    Gtk::Stock::MEDIA_PAUSE, _("_Pause"), _("Pause the emulation"));

	m_refActionGroup->add(Gtk::Action::create("EmulationMenu",
	    _("E_mulation")));
	m_refActionGroup->add(m_toggleActionGo,
	    sigc::mem_fun(*this, &GXemulWindow::on_menu_go));
	m_refActionGroup->add(m_toggleActionPause,
	    sigc::mem_fun(*this, &GXemulWindow::on_menu_pause));
	m_refActionGroup->add(Gtk::Action::create("EmulationReset",
	    _("R_eset"), _("Reset the emulation to its initial state")),
	    sigc::mem_fun(*this, &GXemulWindow::on_menu_reset));

	// Help menu:
	m_refActionGroup->add( Gtk::Action::create("HelpMenu", _("_Help")) );
	m_refActionGroup->add( Gtk::Action::create("HelpAbout",
	    Gtk::Stock::ABOUT), sigc::mem_fun(*this,
	    &GXemulWindow::on_menu_about) );

	m_refUIManager = Gtk::UIManager::create();
	m_refUIManager->insert_action_group(m_refActionGroup);

	add_accel_group(m_refUIManager->get_accel_group());

	Glib::ustring ui_info = 
	    "<ui>"
	    "  <menubar name='MenuBar'>"
	    "    <menu action='FileMenu'>"
	    "      <menu action='FileNew'>"
	    "        <menuitem action='FileNewBlack'/>"
	    "        <menuitem action='FileNewFromTemplate'/>"
	    "      </menu>"
	    "      <menuitem action='FileOpen'/>"
	    "      <separator/>"
	    "      <menuitem action='FileSave'/>"
	    "      <menuitem action='FileSaveAs'/>"
	    "      <separator/>"
	    "      <menuitem action='FileQuit'/>"
	    "    </menu>"
	    "    <menu action='EditMenu'>"
	    "      <menuitem action='EditUndo'/>"
	    "      <menuitem action='EditRedo'/>"
	    "      <separator/>"
	    "      <menuitem action='EditCut'/>"
	    "      <menuitem action='EditCopy'/>"
	    "      <menuitem action='EditPaste'/>"
	    "      <menuitem action='EditDelete'/>"
	    "      <separator/>"
	    "      <menuitem action='EditPreferences'/>"
	    "    </menu>"
	    "    <menu action='EmulationMenu'>"
	    "      <menuitem action='EmulationGo'/>"
	    "      <menuitem action='EmulationPause'/>"
	    "      <menuitem action='EmulationReset'/>"
	    "      <separator/>"
	    "    </menu>"
	    "    <menu action='HelpMenu'>"
	    "      <menuitem action='HelpAbout'/>"
	    "    </menu>"
	    "  </menubar>"
	    "  <toolbar name='ToolBar'>"
	    "    <toolitem action='FileOpen'/>"
	    "    <toolitem action='FileSave'/>"
	    "    <separator/>"
	    "    <toolitem action='EditUndo'/>"
	    "    <toolitem action='EditRedo'/>"
	    "    <separator/>"
	    "    <toolitem action='EmulationGo'/>"
	    "    <toolitem action='EmulationPause'/>"
	    "  </toolbar>"
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

	Gtk::Widget* pToolbar = m_refUIManager->get_widget("/ToolBar");
	if (pToolbar != NULL) {
		((Gtk::Toolbar*)pToolbar)->
		    set_toolbar_style(Gtk::TOOLBAR_ICONS);
		((Gtk::Toolbar*)pToolbar)->set_tooltips(true);
		m_Box.pack_start(*pToolbar, Gtk::PACK_SHRINK);
	}
	
	Gtk::VBox *const pVBox = new Gtk::VBox();
	m_Box.add(*Gtk::manage(pVBox));

	Gtk::VPaned *const pVPaned = new Gtk::VPaned();
	pVBox->pack_start(*Gtk::manage(pVPaned));
	pVPaned->set_border_width(1);

	Gtk::HPaned *const pHPaned = new Gtk::HPaned();
	pVPaned->add1(*Gtk::manage(pHPaned));

	Gtk::Frame *const pFrame1 = new Gtk::Frame();
	pHPaned->add1(*Gtk::manage(pFrame1));
	pFrame1->set_size_request(700, 400);
	pFrame1->add(m_EmulationDesignArea);

	Gtk::Frame *const pFrame2 = new Gtk::Frame();
	pHPaned->add2(*Gtk::manage(pFrame2));
	pFrame2->set_size_request(240, 400);
	// TODO: Component Palette

	Gtk::Frame *const pFrame3 = new Gtk::Frame();
	pVPaned->add2(*Gtk::manage(pFrame3));
	pFrame3->set_size_request(650, 160);
	pFrame3->add(m_DebugConsoleWidget);

	UpdateUI();

	show_all_children();
}


GXemulWindow::~GXemulWindow()
{
}


void GXemulWindow::InsertText(const string& msg)
{
	m_DebugConsoleWidget.InsertText(msg);
}


void GXemulWindow::UpdateUI()
{
	m_updating = true;

	// Undo and Redo:
	// (These only work if the action stack contains undoable
	// and redoable actions, respectively.)
	m_refActionGroup->get_action("EditUndo")->set_sensitive(
	    m_gxemul->GetActionStack().IsUndoPossible());
	m_refActionGroup->get_action("EditRedo")->set_sensitive(
	    m_gxemul->GetActionStack().IsRedoPossible());

	// RunState affects the Play/Pause buttons:
	bool nonEmptyEmulation =
	    ( m_gxemul->GetRootComponent()->GetChildren().size() > 0 );
	m_refActionGroup->get_action("EmulationGo")->set_sensitive(
	    nonEmptyEmulation);
	m_refActionGroup->get_action("EmulationPause")->set_sensitive(
	    nonEmptyEmulation);

	// It's also quite meaningless to be able to save an empty setup:
	m_refActionGroup->get_action("FileSave")->set_sensitive(
	    nonEmptyEmulation);
	m_refActionGroup->get_action("FileSaveAs")->set_sensitive(
	    nonEmptyEmulation);

	// Make the Play or Pause button stay down:
	const GXemul::RunState runState = m_gxemul->GetRunState();

	m_toggleActionGo->set_active(runState == GXemul::Running);
	m_toggleActionPause->set_active(runState == GXemul::Paused);

	// Main window Title:
	string title = "GXemul";
	string emulationFilename = m_gxemul->GetEmulationFilename();
	if (!emulationFilename.empty())
		title = emulationFilename + " - " + title;

	set_title(title);

	// Force the design area to be redrawn:
	Glib::RefPtr<Gdk::Window> win = m_EmulationDesignArea.get_window();
	if (win != NULL) {
		Gdk::Rectangle r(0, 0, get_allocation().get_width(),
		    get_allocation().get_height());
		win->invalidate_rect(r, true);
	}

	m_updating = false;
}


void GXemulWindow::on_menu_about()
{
	Gtk::MessageDialog dialog(*this, "GXemul "VERSION,
	    false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK);

	string msg = COPYRIGHT_MSG"\n"SECONDARY_MSG"\n";
	msg += _("If you have questions or feedback, don't "
	    "hesitate to mail me.\nanders@gavare.se");
	dialog.set_secondary_text(msg);

	dialog.run();
}


static void TodoDialog(Gtk::Window *w, const char *msg)
{
	Gtk::MessageDialog dialog(*w, msg,
	    false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
	dialog.run();
}


void GXemulWindow::ShutdownUI()
{
	hide();
}


void GXemulWindow::on_menu_copy()
{
	TodoDialog(this, "GXemulWindow::on_menu_copy(): TODO");
}


void GXemulWindow::on_menu_cut()
{
	TodoDialog(this, "GXemulWindow::on_menu_cut(): TODO");
}


void GXemulWindow::on_menu_delete()
{
	TodoDialog(this, "GXemulWindow::on_menu_delete(): TODO");
}


void GXemulWindow::on_menu_go()
{
	const GXemul::RunState runState = m_gxemul->GetRunState();

	if (!m_updating) {
		if (runState != GXemul::Running)
			m_gxemul->GetCommandInterpreter().
			    RunCommand("continue");
		else
			m_toggleActionGo->set_active(
			    runState == GXemul::Running);
	}
}


void GXemulWindow::on_menu_new_blank()
{
	// TODO: Confirmation if the current emulation is "dirty"?

	m_gxemul->GetCommandInterpreter().RunCommand("close");
}


void GXemulWindow::on_menu_new_from_template()
{
	bool nonEmptyEmulation =
	    ( m_gxemul->GetRootComponent()->GetChildren().size() > 0 );
	if (nonEmptyEmulation)
		m_gxemul->GetCommandInterpreter().RunCommand("close");

	// TODO: List available templates?
	// TODO: DON't hardcore this!
	m_gxemul->GetCommandInterpreter().RunCommand("add testmips");
}


void GXemulWindow::on_menu_open()
{
	// TODO: Confirmation if the current emulation is "dirty"?

	// Mostly inspired by the GTKMM example for a File->Open handler.

	Gtk::FileChooserDialog dialog(_("Choose an emulation to open"),
	    Gtk::FILE_CHOOSER_ACTION_OPEN);
	dialog.set_transient_for(*this);

	dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

	Gtk::FileFilter filter_gxemul;
	filter_gxemul.set_name(_("GXemul files"));
	filter_gxemul.add_pattern("*.gxemul");
	dialog.add_filter(filter_gxemul);

	Gtk::FileFilter filter_any;
	filter_any.set_name(_("Any files"));
	filter_any.add_pattern("*");
	dialog.add_filter(filter_any);

	std::string filename;		// note: std::string, not string
	int result = dialog.run();

	switch(result) {

	case Gtk::RESPONSE_OK:
		filename = dialog.get_filename();
		m_gxemul->GetCommandInterpreter().RunCommand("load "+filename);
		break;

	case Gtk::RESPONSE_CANCEL:
		// Do nothing.
		break;

	default:
		// Unexpected button clicked. TODO
		break;
	}
}


void GXemulWindow::on_menu_paste()
{
	TodoDialog(this, "GXemulWindow::on_menu_paste(): TODO");
}


void GXemulWindow::on_menu_pause()
{
	const GXemul::RunState runState = m_gxemul->GetRunState();

	if (!m_updating) {
		if (runState != GXemul::Paused)
			m_gxemul->GetCommandInterpreter().RunCommand("pause");
		else
			m_toggleActionPause->set_active(
			    runState == GXemul::Paused);
	}
}


void GXemulWindow::on_menu_preferences()
{
	TodoDialog(this, "GXemulWindow::on_menu_preferences(): TODO");
}


void GXemulWindow::on_menu_quit()
{
	m_gxemul->GetCommandInterpreter().RunCommand("quit");
}


void GXemulWindow::on_menu_redo()
{
	m_gxemul->GetCommandInterpreter().RunCommand("redo");
}


void GXemulWindow::on_menu_reset()
{
	m_gxemul->GetCommandInterpreter().RunCommand("reset");
}


void GXemulWindow::on_menu_save()
{
	if (m_gxemul->GetEmulationFilename().empty())
		on_menu_save_as();
	else
		m_gxemul->GetCommandInterpreter().RunCommand("save");
}


void GXemulWindow::on_menu_save_as()
{
	// Mostly inspired by the GTKMM example for a File->Open handler.

	Gtk::FileChooserDialog dialog(
	    _("Choose a name for the emulation (.gxemul)"),
	    Gtk::FILE_CHOOSER_ACTION_SAVE);
	dialog.set_transient_for(*this);

	dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

	Gtk::FileFilter filter_gxemul;
	filter_gxemul.set_name(_("GXemul files"));
	filter_gxemul.add_pattern("*.gxemul");
	dialog.add_filter(filter_gxemul);

	Gtk::FileFilter filter_any;
	filter_any.set_name(_("Any files"));
	filter_any.add_pattern("*");
	dialog.add_filter(filter_any);

	std::string filename;		// note: std::string, not string
	int result = dialog.run();

	switch(result) {

	case Gtk::RESPONSE_OK:
		filename = dialog.get_filename();
		m_gxemul->GetCommandInterpreter().RunCommand("save "+filename);
		break;

	case Gtk::RESPONSE_CANCEL:
		// Do nothing.
		break;

	default:
		// Unexpected button clicked. TODO
		break;
	}
}


void GXemulWindow::on_menu_undo()
{
	m_gxemul->GetCommandInterpreter().RunCommand("undo");
}


#endif	// WITH_GTKMM
