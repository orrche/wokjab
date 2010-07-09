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

#include "MassRename.hpp"


MassRename::MassRename(WLSignal *wls) : WoklibPlugin(wls),
	kit(new Gtk::Main(0,NULL))
{
	EXP_SIGHOOK("Jabber MassRename Show", &MassRename::Show, 1000);
	EXP_SIGHOOK("GetMenu", &MassRename::Menu, 1000);
	
}

MassRename::~MassRename()
{
	delete kit;
}

int
MassRename::Menu(WokXMLTag *tag)
{
	WokXMLTag &menuitem = tag->AddTag("item");
	menuitem.AddAttr("name", "Mass Rename");
	menuitem.AddAttr("signal", "Jabber MassRename Show");
	
	return 1;
}

int
MassRename::vcard(WokXMLTag *tag)
{
	std::string session = tag->GetAttr("session");
	std::string jid = tag->GetFirstTag("iq").GetAttr("from");
	std::string nickname = tag->GetFirstTag("iq").GetFirstTag("vCard", "vcard-temp").GetFirstTag("NICKNAME").GetBody();
	if ( nickname == "" )
		nickname = tag->GetFirstTag("iq").GetFirstTag("vCard", "vcard-temp").GetFirstTag("FN").GetBody();
	
	Gtk::TreeModel::iterator iter;
	
	for ( iter = userstore->children().begin() ; iter != userstore->children().end(); iter++ )
	{
		if ( session == (*iter)[Columns.session] && 
			jid == (*iter)[Columns.jid] )
		{
			(*iter)[Columns.nickname] = nickname;
		}
	}
	
	return 1;
}

void
MassRename::on_ok_button_clicked()
{
	on_apply_button_clicked();
	on_cancel_button_clicked();
}

void
MassRename::on_apply_button_clicked()
{
	Gtk::TreeModel::iterator iter;
	for ( iter = userstore->children().begin() ; iter != userstore->children().end(); iter++ )
	{
		WokXMLTag nick_change("change");
		nick_change.AddAttr("session", (Glib::ustring)(*iter)[Columns.session]);
		nick_change.AddAttr("jid", (Glib::ustring)(*iter)[Columns.jid]);
		nick_change.AddAttr("nickname", (Glib::ustring)(*iter)[Columns.nickname]);
		
		wls->SendSignal("Roster Change JID Nickname", nick_change);
	
	}
}

void
MassRename::on_cancel_button_clicked()
{
	Gtk::Widget *win;
	builder->get_widget("main_window", win);
	delete win;
 	builder.clear();
	userstore.clear();
}

int
MassRename::Show(WokXMLTag *tag)
{
	if ( !builder )
	{
	 	builder = Gtk::Builder::create_from_file(PACKAGE_GLADE_DIR"/wokjab/MassRename.xml");
	 	userstore = Gtk::ListStore::create(Columns);

	 	Gtk::TreeView *jidview;
	 	builder->get_widget("jidview", jidview);
	 	jidview->set_model(userstore);
	 	
//	 	jidview->append_column("Change", Columns.change);
	  jidview->append_column("session", Columns.session);
	  jidview->append_column("jid", Columns.jid);
	  jidview->append_column("nickname", Columns.nickname);

		Gtk::Button *ok_btn;
		builder->get_widget("ok_btn", ok_btn);
		
		ok_btn->signal_clicked().connect(sigc::mem_fun(*this, &MassRename::on_ok_button_clicked));

		Gtk::Button *cancel_btn;
		builder->get_widget("cancel_btn", cancel_btn);
		
		cancel_btn->signal_clicked().connect(sigc::mem_fun(*this, &MassRename::on_cancel_button_clicked));

		Gtk::Button *apply_btn;
		builder->get_widget("apply_btn", apply_btn);
		
		apply_btn->signal_clicked().connect(sigc::mem_fun(*this, &MassRename::on_apply_button_clicked));
		
		Gtk::Widget *win_temp;
		builder->get_widget("main_window", win_temp);
		win_temp->signal_hide().connect(sigc::mem_fun(*this, &MassRename::on_cancel_button_clicked));
 	}
 	
 	Gtk::Widget *win;
	builder->get_widget("main_window", win);
 	
 	if ( win )
 	{
 		WokXMLTag roster("roster");
 		wls->SendSignal("Roster Get Roster", roster);
 		
 		std::list <WokXMLTag *>::iterator sess_iter;
		std::list <WokXMLTag *>::iterator user_iter;	
 		for ( sess_iter = roster.GetTagList("session").begin() ;
 			sess_iter != roster.GetTagList("session").end(); sess_iter++)
 		{
 			for ( user_iter = (*sess_iter)->GetTagList("user").begin() ;
 				user_iter != (*sess_iter)->GetTagList("user").end(); user_iter++)
 			{
 				
 				if ( (*user_iter)->GetAttr("nick") == "" || (*user_iter)->GetAttr("nick") == (*user_iter)->GetAttr("jid") )
 				{
	 				Gtk::TreeModel::Row row = *(userstore->append());
//					row[Columns.change] = true;
					row[Columns.session] = (*sess_iter)->GetAttr("name").c_str();
					row[Columns.jid] = (*user_iter)->GetAttr("jid").c_str();
					row[Columns.nickname] = (*user_iter)->GetAttr("nick").c_str();
					
					// The v-card request
					WokXMLTag msgtag("message");
					msgtag.AddAttr("session", (*sess_iter)->GetAttr("name"));
					WokXMLTag &iq_tag = msgtag.AddTag("iq");
					iq_tag.AddAttr("to", (*user_iter)->GetAttr("jid"));
					iq_tag.AddAttr("type", "get");
					WokXMLTag &vcard = iq_tag.AddTag("vCard", "vcard-temp");
					vcard.AddAttr("prodid", "-//HandGen//NONSGML vGen v1.0//EN");
					vcard.AddAttr("version", "2.0");
	        wls->SendSignal("Jabber XML IQ Send", &msgtag);

					EXP_SIGHOOK("Jabber XML IQ ID " + msgtag.GetFirstTag("iq").GetAttr("id"), &MassRename::vcard, 1000);

				}
 			}
 		}
		win->show_all();
	}
	
	return 1;	
}
