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

#include <algorithm>
#include "RosterItem.h"


RosterItem::RosterItem(WLSignal *wls, GladeXML *xml, std::string id, WokXMLTag *tag, RosterItem *parant) : WLSignalInstance(wls),
id(id),
parant(parant),
xml(xml)
{
	dataxml = new WokXMLTag(*tag);
	indent = -1;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(glade_xml_get_widget(xml, "view_roster")));
	
	if(parant)
	{
		indent = parant->GetIndent() + 1;
	}
	
	if(parant)
	{
		RosterItem *ri = parant->AddChild(this);
		if (ri)
		{
			gtk_tree_store_insert_after(GTK_TREE_STORE(model), &iter, parant->GetIter(), ri->GetIter());

			
		}
		else
		{
			gtk_tree_store_insert_after(GTK_TREE_STORE(model), &iter, parant->GetIter(), NULL);
//			std::cout << "Expanding " << parant->text << std::endl;
//			GtkTreePath *gtp = gtk_tree_model_get_path(model, parant->GetIter());
//			gtk_tree_view_expand_row(GTK_TREE_VIEW(glade_xml_get_widget(xml, "view_roster")), gtp, TRUE);
//			gtk_tree_path_free(gtp);
		}
	}
	
	Update(tag);
	// gtk_tree_view_expand_all(GTK_TREE_VIEW(glade_xml_get_widget(xml, "view_roster")));

}

RosterItem::~RosterItem()
{
	GtkTreeModel *model;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(glade_xml_get_widget(xml, "view_roster")));
	gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);

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
	return NULL;
}

RosterItem *
RosterItem::UpdatePosition(RosterItem *ri)
{
	std::list <RosterItem*>::iterator iter;
	
	std::list <RosterItem*>::iterator pos = std::find(children.begin(), children.end(), ri);
	
	children.erase(pos);
	
	for ( iter = children.end() ; iter != children.begin() ; )
	{
		iter--;
		int iterorder = atoi((*iter)->dataxml->GetAttr("order").c_str());
		int riorder = atoi(ri->dataxml->GetAttr("order").c_str());
		if ( iterorder == riorder )
		{
			if( (*iter)->text < ri->text )
			{
				std::list <RosterItem*>::iterator tmpiter;
				tmpiter = iter;
				tmpiter++;
				children.insert(tmpiter, 1, ri);
				return (*iter);
			}
		}
		else if( iterorder < riorder )
		{
			std::list <RosterItem*>::iterator tmpiter;
			tmpiter = iter;
			tmpiter++;
			children.insert(tmpiter, 1, ri);
			return (*iter);
		}
	}

	children.push_front(ri);
	return NULL;	
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
	delete dataxml;
	dataxml = new WokXMLTag(*tag);
	
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
			int pwidth = gdk_pixbuf_get_width(tmp_pix);
			int pheight = gdk_pixbuf_get_height(tmp_pix);
			int w,h;
			
			if ( pwidth > pheight )
			{
				w = AVATAR_SIZE;
				h = int(pheight / ( ((double)pwidth) / AVATAR_SIZE ));
			}
			else
			{
				w = int(pwidth / ( ((double)pheight) / AVATAR_SIZE ));
				h = AVATAR_SIZE;
			}

			post_pix = gdk_pixbuf_scale_simple(tmp_pix, w, h, GDK_INTERP_BILINEAR);
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
	
	if ( parant )
	{
		RosterItem *ri = parant->UpdatePosition(this);
		if( ri )
			gtk_tree_store_move_after(GTK_TREE_STORE(model), &iter, ri->GetIter());
		else
		{
			gtk_tree_store_move_after(GTK_TREE_STORE(model), &iter, NULL);
			parant->UpdateExpand();
		}
	}
	gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
				PRE_PIX_COLUMN, pre_pix,
				TEXT_COLUMN, text.c_str(),
				POST_PIX_COLUMN, post_pix,
				ID_COLUMN, id.c_str(),
				-1);

	UpdateExpand();
		
	if( pre_pix )
		g_object_unref(pre_pix);
	if( post_pix )
		g_object_unref(post_pix);
	
	/*
	GtkTreePath*        gtk_tree_model_get_path             (GtkTreeModel *tree_model,
                                                         GtkTreeIter *iter);
	gtk_tree_view_expand_row            (GtkTreeView *tree_view,
                                                         GtkTreePath *path,
                                                         gboolean open_all);
	*/
}

void
RosterItem::UpdateExpand()
{
	GtkTreePath *gtp = gtk_tree_model_get_path(model, &iter);
	if ( dataxml->GetAttr("expanded") == "false")
	{
		gtk_tree_view_collapse_row(GTK_TREE_VIEW(glade_xml_get_widget(xml, "view_roster")), gtp);
	}
	else
	{
		gtk_tree_view_expand_row(GTK_TREE_VIEW(glade_xml_get_widget(xml, "view_roster")), gtp, FALSE);
	}
		
	gtk_tree_path_free(gtp);

	/* Updating all the children to since gtk doesn't seam to remember that setting when you collapse a parant... */

	std::list <RosterItem*>::iterator iter;
	for ( iter = children.begin() ; iter != children.end() ; iter++)
	{
		(*iter)->UpdateExpand();
	}
}

GtkTreeIter *
RosterItem::GetIter()
{
	if ( parant )
		return &iter;
	return NULL;

}

int
RosterItem::GetIndent()
{
	return indent;
}

