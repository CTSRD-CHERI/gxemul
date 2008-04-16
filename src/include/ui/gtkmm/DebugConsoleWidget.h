#ifndef DEBUGCONSOLEWIDGET_H
#define DEBUGCONSOLEWIDGET_H

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


class GXemul;

class DebugConsoleWidget : public Gtk::VBox
{
public:
	DebugConsoleWidget(GXemul* gxemul);
	virtual ~DebugConsoleWidget();

	void InsertText(const string& msg);

private:
	void on_entry_activate();

protected:
	GXemul*			m_GXemul;

	Gtk::ScrolledWindow	m_ScrolledWindow;

	Gtk::TextView		m_TextView;
	Glib::RefPtr<Gtk::TextBuffer> m_refTextBuffer;
	Glib::RefPtr<Gtk::TextBuffer::Mark> m_endMark;
	Gtk::TextBuffer::iterator m_textBufferIterator;

	Gtk::Entry		m_Entry;
};

#endif	// DEBUGCONSOLEWIDGET_H

