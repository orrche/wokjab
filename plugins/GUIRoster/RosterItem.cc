/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


//
// Class: RosterItem
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Fri Sep  1 12:38:53 2006
//

#include "RosterItem.h"


RosterItem::RosterItem(WLSignal *wls, GladeXML *xml, std::string id, WokXMLTag *tag, RosterItem *parant) : WLSignalInstance(wls),
id(id),
parant(parant),
xml(xml)
{
	
	indent = 0;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(glade_xml_get_widget(xml, "view_roster")));
	
	if(parant)
	{
		indent = parant->GetIndent() + 1;
	}
	
	if(parant)
	{
		RosterItem *ri = parant->AddChild(this);
		gtk_list_store_insert_after(GTK_LIST_STORE(model), &iter, ri->GetIter());
	}
	else
		gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	
	Update(tag);

}

RosterItem::~RosterItem()
{
	GtkTreeModel *model;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(glade_xml_get_widget(xml, "view_roster")));
	gtk_list_store_remove(GTK_LIST_STORE(model), &iter);

	if ( parant )
	{
		parant->RemoveChild(this);
	}
}

RosterItem *
RosterItem::AddChild(RosterItem *ri)
{
	std::list <RosterItem*>::iterator iter;
	
	for ( iter = children.end() ; iter != children.begin() ; )
	{
		iter--;
		
		if( (*iter)->text < ri->text )
		{
			std::list <RosterItem*>::iterator tmpiter;
			tmpiter = iter;
			tmpiter++;
			children.insert(tmpiter, 1, ri);
			return ((*iter)->GetLastItem());
		}	
	}

	children.push_front(ri);
	return this;
}

RosterItem *
RosterItem::GetLastItem()
{
	if ( children.empty() )
		return this;
	
	std::list <RosterItem*>::iterator iter;
	iter = children.end();
	iter--;
	return (*iter)->GetLastItem();

}

void
RosterItem::RemoveChild(RosterItem *ri)
{
	children.remove ( ri );
}

void
RosterItem::Update(WokXMLTag *tag)
{	
	GError *err = NULL;
	GdkPixbuf *pre_pix = NULL;
	GdkPixbuf *post_pix = NULL;
	if(tag->GetFirstTag("columns").GetFirstTag("pre_pix").GetBody().size())
	{
		pre_pix = gdk_pixbuf_new_from_file(tag->GetFirstTag("columns").GetFirstTag("pre_pix").GetBody().c_str(), &err);
	}
	if(tag->GetFirstTag("columns").GetFirstTag("post_pix").GetBody().size())
	{
		GdkPixbuf *tmp_pix = NULL;
		tmp_pix = gdk_pixbuf_new_from_file(tag->GetFirstTag("columns").GetFirstTag("post_pix").GetBody().c_str(), &err);
		if ( tmp_pix )
		{
			post_pix = gdk_pixbuf_scale_simple(tmp_pix, AVATAR_SIZE, AVATAR_SIZE, GDK_INTERP_BILINEAR);
			g_object_unref(tmp_pix);
		}
	}
	
	std::string strindent;
	for( int i = 0 ; i < indent ; i++ )
		strindent += "  ";
		
	text = strindent.c_str() + tag->GetFirstTag("columns").GetFirstTag("text").GetBody();
	std::string::size_type pos = -1;
	while((pos = text.find("\n", pos+1)) != std::string::npos)
		text.insert(pos+1, strindent);
	
	gtk_list_store_set(GTK_LIST_STORE(model), &iter,
				PRE_PIX_COLUMN, pre_pix,
				TEXT_COLUMN, text.c_str(),
				POST_PIX_COLUMN, post_pix,
				ID_COLUMN, id.c_str(),
				-1);
}

GtkTreeIter *
RosterItem::GetIter()
{
	return &iter;

}

int
RosterItem::GetIndent()
{
	return indent;
}

