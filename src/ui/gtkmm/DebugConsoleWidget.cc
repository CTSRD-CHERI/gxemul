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

#include "misc.h"
#include "ui/gtkmm/DebugConsoleWidget.h"
#include "GXemul.h"


DebugConsoleWidget::DebugConsoleWidget(GXemul* gxemul)
	: m_GXemul(gxemul)
{
	Gtk::VBox *const pVBox = new Gtk::VBox();
	add(*Gtk::manage(pVBox));

	m_refTextBuffer = Gtk::TextBuffer::create();
	m_textBufferIterator = m_refTextBuffer->begin();
	InsertText("GXemul "VERSION"     "COPYRIGHT_MSG"\n"SECONDARY_MSG"\n");

	m_TextView.set_buffer(m_refTextBuffer);
	m_TextView.set_cursor_visible(false);
	m_TextView.set_editable(false);

	m_Entry.signal_activate().connect(sigc::mem_fun(*this,
	    &DebugConsoleWidget::on_entry_activate));

	m_ScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC,
	    Gtk::POLICY_AUTOMATIC);
	m_ScrolledWindow.add(m_TextView);

	pVBox->pack_start(m_ScrolledWindow);
	pVBox->pack_start(m_Entry, Gtk::PACK_SHRINK);
}


DebugConsoleWidget::~DebugConsoleWidget()
{
}


void DebugConsoleWidget::InsertText(const string& msg)
{
	m_textBufferIterator = m_refTextBuffer->insert(
	    m_textBufferIterator, msg);
	
	// TODO: Scroll to end of buffer!
}


void DebugConsoleWidget::on_entry_activate()
{
	string command = m_Entry.get_text();
	m_Entry.set_text("");
	m_GXemul->GetCommandInterpreter().RunCommand(command);
}


#endif	// WITH_GTKMM
