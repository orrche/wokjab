/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2011 <nedo80@gmail.com>
 *
 * wokjab is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * wokjab is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "WokjabDockWindowMaster.h"

WokjabDockWindowMaster::WokjabDockWindowMaster(std::string type, WokjabDockWindowHandler *handler) {
    this->handler = handler;
	this->type = type;
	win.set_default_size(500,300);
	win.add(nb);
	win.show_all();
	i = 0;


	win.signal_delete_event().connect(sigc::mem_fun(*this,
              &WokjabDockWindowMaster::on_destroy));
}

WokjabDockWindowMaster::~WokjabDockWindowMaster() {
}

bool
WokjabDockWindowMaster::on_destroy(GdkEventAny *event)
{
	handler->RemoveWindow(this);

	return true;
}

Gtk::Notebook *
WokjabDockWindowMaster::GetNotebook() {

	return &nb;
}

std::string
WokjabDockWindowMaster::getType() {
		return type;
}

void
WokjabDockWindowMaster::Raise() {
	return;
	// This doesn't work for some reason...
	win.raise();
}

void
WokjabDockWindowMaster::SetUrgency(bool value) {
	win.set_urgency_hint(value);
}
