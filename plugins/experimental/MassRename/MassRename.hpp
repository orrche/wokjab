/***************************************************************************
 *  Copyright (C) 2003-2010 Kent Gustavsson <nedo80@gmail.com>
 ****************************************************************************/
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef _MASS_RENAME_HPP_
#define _MASS_RENAME_HPP_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <gtkmm.h>

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>


using namespace Woklib;

class MassRename: public WoklibPlugin 
{
public:
	MassRename(WLSignal *wls);
	~MassRename();
	
	int Menu(WokXMLTag *tag);
	int Show(WokXMLTag *tag);
	int vcard(WokXMLTag *tag);
	void on_cancel_button_clicked();
	void on_apply_button_clicked();
	void on_ok_button_clicked();
	
protected:
private:
	Glib::RefPtr<Gtk::Builder> builder;
	Glib::RefPtr<Gtk::ListStore> userstore;
	Gtk::Main *kit;
	
	
	class ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumns()
    { 
    	add(change); add(session); add(jid); add(nickname);}

	    Gtk::TreeModelColumn<bool> change;
  	  Gtk::TreeModelColumn<Glib::ustring> session;
  	  Gtk::TreeModelColumn<Glib::ustring> jid;
  	  Gtk::TreeModelColumn<Glib::ustring> nickname;
  };

  ModelColumns Columns;
	
	
};

#endif // _MASS_RENAME_HPP_
