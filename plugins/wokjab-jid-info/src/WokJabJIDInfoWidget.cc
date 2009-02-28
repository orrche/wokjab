/***************************************************************************
 *  Copyright (C) 2003-2009  Kent Gustavsson <oden@gmx.net>
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

#include "../include/WokJabJIDInfoWidget.h"
#include <Woklib/WokXMLTag.h>
#include <iostream>

WokJabJIDInfoWidget::WokJabJIDInfoWidget (WLSignal * wls, const std::string &jid, const std::string &session):
WLSignalInstance (wls),
jid(jid),
session(session)
{
	GtkWidget *window;
	GtkWidget *vbox1;
	GtkWidget *hbox1;
	GtkWidget *label17;
	GtkWidget *JIDLabel;
	GtkWidget *PersonalFrame;
	GtkWidget *table1;
	

	GtkWidget *label1;
	GtkWidget *ClientFrame;
	GtkWidget *table2;
	GtkWidget *label11;
	GtkWidget *AboutFrame;
	GtkWidget *scrolledwindow1;
	GtkWidget *label6;

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width (GTK_CONTAINER (window), 2);
	gtk_window_set_title (GTK_WINDOW (window),
			      "Contact Information for");

	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox1);
	gtk_container_add (GTK_CONTAINER (window), vbox1);

	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);

	label17 = gtk_label_new ("Contact Information for ");
	gtk_widget_show (label17);
	gtk_box_pack_start (GTK_BOX (hbox1), label17, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (label17), GTK_JUSTIFY_LEFT);

	JIDLabel = gtk_label_new (jid.c_str());
	gtk_widget_show (JIDLabel);
	gtk_box_pack_start (GTK_BOX (hbox1), JIDLabel, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (JIDLabel), GTK_JUSTIFY_LEFT);

	PersonalFrame = gtk_frame_new (NULL);
	gtk_widget_show (PersonalFrame);
	gtk_box_pack_start (GTK_BOX (vbox1), PersonalFrame, TRUE, TRUE, 0);

	table1 = gtk_table_new (4, 2, FALSE);
	gtk_widget_show (table1);
	gtk_container_add (GTK_CONTAINER (PersonalFrame), table1);
	gtk_container_set_border_width (GTK_CONTAINER (table1), 2);
	gtk_table_set_row_spacings (GTK_TABLE (table1), 2);
	gtk_table_set_col_spacings (GTK_TABLE (table1), 2);

	NicknameLabel = gtk_label_new ("Nickname:");
	gtk_widget_show (NicknameLabel);
	gtk_table_attach (GTK_TABLE (table1), NicknameLabel, 0, 1, 0, 1,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (NicknameLabel), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (NicknameLabel), 0, 0.5);

	FullnameLabel = gtk_label_new ("Full Name:");
	gtk_widget_show (FullnameLabel);
	gtk_table_attach (GTK_TABLE (table1), FullnameLabel, 0, 1, 1, 2,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (FullnameLabel), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (FullnameLabel), 0, 0.5);

	WebsiteLabel = gtk_label_new ("Web Site:");
	gtk_widget_show (WebsiteLabel);
	gtk_table_attach (GTK_TABLE (table1), WebsiteLabel, 0, 1, 2, 3,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (WebsiteLabel), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (WebsiteLabel), 0, 0.5);

	BirthdayLabel = gtk_label_new ("Birthday:");
	gtk_widget_show (BirthdayLabel);
	gtk_table_attach (GTK_TABLE (table1), BirthdayLabel, 0, 1, 3, 4,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (BirthdayLabel), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (BirthdayLabel), 0, 0.5);

	NicknameLabelText = gtk_label_new ("");
	gtk_widget_show (NicknameLabelText);
	gtk_table_attach (GTK_TABLE (table1), NicknameLabelText, 1, 2, 0, 1,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (NicknameLabelText),
			       GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (NicknameLabelText), 0, 0.5);

	FullnameLabelText = gtk_label_new ("");
	gtk_widget_show (FullnameLabelText);
	gtk_table_attach (GTK_TABLE (table1), FullnameLabelText, 1, 2, 1, 2,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (FullnameLabelText),
			       GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (FullnameLabelText), 0, 0.5);

	WebsiteLabelText = gtk_label_new ("");
	gtk_widget_show (WebsiteLabelText);
	gtk_table_attach (GTK_TABLE (table1), WebsiteLabelText, 1, 2, 2, 3,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (WebsiteLabelText),
			       GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (WebsiteLabelText), 0, 0.5);

	BirthdayLabelText = gtk_label_new ("");
	gtk_widget_show (BirthdayLabelText);
	gtk_table_attach (GTK_TABLE (table1), BirthdayLabelText, 1, 2, 3, 4,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (BirthdayLabelText),
			       GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (BirthdayLabelText), 0, 0.5);

	label1 = gtk_label_new ("Personal Infortmation");
	gtk_widget_show (label1);
	gtk_frame_set_label_widget (GTK_FRAME (PersonalFrame), label1);
	gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);

	ClientFrame = gtk_frame_new (NULL);
	gtk_widget_show (ClientFrame);
	gtk_box_pack_start (GTK_BOX (vbox1), ClientFrame, TRUE, TRUE, 0);

	table2 = gtk_table_new (3, 2, FALSE);
	gtk_widget_show (table2);
	gtk_container_add (GTK_CONTAINER (ClientFrame), table2);
	gtk_container_set_border_width (GTK_CONTAINER (table2), 2);
	gtk_table_set_row_spacings (GTK_TABLE (table2), 2);
	gtk_table_set_col_spacings (GTK_TABLE (table2), 2);

	ClientLabel = gtk_label_new ("Client:");
	gtk_table_attach (GTK_TABLE (table2), ClientLabel, 0, 1, 0, 1,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (ClientLabel), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (ClientLabel), 0, 0.5);

	ClientLabelText = gtk_label_new ("");
	gtk_table_attach (GTK_TABLE (table2), ClientLabelText, 1, 2, 0, 1,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (ClientLabelText), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (ClientLabelText), 0, 0.5);

	VersionLabel = gtk_label_new ("Version:");
	gtk_table_attach (GTK_TABLE (table2), VersionLabel, 0, 1, 1, 2,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (VersionLabel), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (VersionLabel), 0, 0.5);

	VersionLabelText = gtk_label_new ("");
	gtk_table_attach (GTK_TABLE (table2), VersionLabelText, 1, 2, 1, 2,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (VersionLabelText),
			       GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (VersionLabelText), 0, 0.5);

	OSLabel = gtk_label_new ("OS:");
	gtk_table_attach (GTK_TABLE (table2), OSLabel, 0, 1, 2, 3,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (OSLabel), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (OSLabel), 0, 0.5);

	OSLabelText = gtk_label_new ("");
	gtk_table_attach (GTK_TABLE (table2), OSLabelText, 1, 2, 2, 3,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (OSLabelText), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (OSLabelText), 0, 0.5);

	label11 = gtk_label_new ("Client Information");
	gtk_widget_show (label11);
	gtk_frame_set_label_widget (GTK_FRAME (ClientFrame), label11);
	gtk_label_set_justify (GTK_LABEL (label11), GTK_JUSTIFY_LEFT);

	AboutFrame = gtk_frame_new (NULL);
	gtk_widget_show (AboutFrame);
	gtk_box_pack_start (GTK_BOX (vbox1), AboutFrame, TRUE, TRUE, 0);

	scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledwindow1);
	gtk_container_add (GTK_CONTAINER (AboutFrame), scrolledwindow1);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW
					     (scrolledwindow1),
					     GTK_SHADOW_IN);

	textview1 = gtk_text_view_new ();
	gtk_widget_show (textview1);
	gtk_container_add (GTK_CONTAINER (scrolledwindow1), textview1);
	gtk_text_view_set_editable (GTK_TEXT_VIEW (textview1), FALSE);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (textview1), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW (textview1), GTK_WRAP_WORD_CHAR);
	label6 = gtk_label_new ("About");
	gtk_widget_show (label6);
	gtk_frame_set_label_widget (GTK_FRAME (AboutFrame), label6);
	gtk_label_set_justify (GTK_LABEL (label6), GTK_JUSTIFY_LEFT);
	
	
	WokXMLTag querytag(NULL, "query");
	WokXMLTag &itemtag = querytag.AddTag("item");
	itemtag.AddAttr("jid", jid);
	itemtag.AddAttr("session", session);
	
	wls->SendSignal("Jabber Roster GetResource", &querytag);
		
	std::string jidresource;
	if(itemtag.GetTagList("resource").size() == 0)
		jidresource = jid;
	else
	{
		if ( itemtag.GetFirstTag("resource").GetAttr("name").size() )
			jidresource = jid + '/' + itemtag.GetFirstTag("resource").GetAttr("name");
		else
			jidresource = jid;
	}
	
	{
		WokXMLTag msgtag(NULL, "message");
		msgtag.AddAttr("session", session);
		WokXMLTag &iqtag = msgtag.AddTag("iq");
		iqtag.AddAttr("type", "get");
		iqtag.AddAttr("to", jidresource);
		WokXMLTag &iqqeuerytag = iqtag.AddTag("query");
		iqqeuerytag.AddAttr("xmlns", "jabber:iq:version");
		
		wls->SendSignal("Jabber XML IQ Send", &msgtag);
		id_version = std::string("Jabber XML IQ ID ") + iqtag.GetAttr("id");
		EXP_SIGHOOK(id_version.c_str(), &WokJabJIDInfoWidget::xml, 1000);
	}
	
	{
		WokXMLTag msgtag(NULL, "message");
		msgtag.AddAttr("session", session);
		WokXMLTag &iq_tag = msgtag.AddTag("iq");
		WokXMLTag &vcard = iq_tag.AddTag("vCard");
		iq_tag.AddAttr("to", jidresource);
		iq_tag.AddAttr("type", "get");
		vcard.AddAttr("xmlns", "vcard-temp");
		vcard.AddAttr("prodid", "-//HandGen//NONSGML vGen v1.0//EN");
		vcard.AddAttr("version", "2.0");
		wls->SendSignal("Jabber XML IQ Send", &msgtag);
		id_vcard = std::string("Jabber XML IQ ID ") + iq_tag.GetAttr("id");
		
		EXP_SIGHOOK(id_vcard.c_str(), &WokJabJIDInfoWidget::vcard, 1000);
	}
	gtk_widget_show(window);
}


WokJabJIDInfoWidget::~WokJabJIDInfoWidget ()
{
	EXP_SIGUNHOOK(id_version.c_str(), &WokJabJIDInfoWidget::xml, 1000);
	EXP_SIGUNHOOK(id_vcard.c_str(), &WokJabJIDInfoWidget::vcard, 1000);
}

int 
WokJabJIDInfoWidget::vcard (WokXMLTag *msgtag)
{
	WokXMLTag *tag = &msgtag->GetFirstTag("iq");
	WokXMLTag &vcard = tag->GetFirstTag("vCard");
	WokXMLTag *tmptag;
	
	if( vcard.GetFirstTag("BDAY").GetBody().length() )
	{
		gtk_label_set_text(GTK_LABEL(BirthdayLabelText), vcard.GetFirstTag("BDAY").GetBody().c_str());
		gtk_widget_show(BirthdayLabel);
		gtk_widget_show(BirthdayLabelText);
	}
	if( vcard.GetFirstTag("NICKNAME").GetBody().length() )
	{
		gtk_label_set_text(GTK_LABEL(NicknameLabelText), vcard.GetFirstTag("NICKNAME").GetBody().c_str());
		gtk_widget_show(NicknameLabel);
		gtk_widget_show(NicknameLabelText);
	}
	if( vcard.GetFirstTag("FN").GetBody().length() )
	{
		gtk_label_set_text(GTK_LABEL(FullnameLabelText), vcard.GetFirstTag("FN").GetBody().c_str());
		gtk_widget_show(FullnameLabel);
		gtk_widget_show(FullnameLabelText);
	}
	if( vcard.GetFirstTag("URL").GetBody().length() )
	{
		gtk_label_set_text(GTK_LABEL(WebsiteLabelText), vcard.GetFirstTag("URL").GetBody().c_str());
		gtk_widget_show(WebsiteLabel);
		gtk_widget_show(WebsiteLabelText);
	}
	if( vcard.GetFirstTag("DESC").GetBody().length() )
	{
		gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview1)), vcard.GetFirstTag("DESC").GetBody().c_str(), -1);
	}
}

int
WokJabJIDInfoWidget::xml (WokXMLTag *tag_msg)
{
	WokXMLTag *tag_iq = &tag_msg->GetFirstTag("iq");
	WokXMLTag *query_iq;

	if( ( query_iq = &tag_iq->GetFirstTag("query")))
	{
		WokXMLTag *tag;
		if( query_iq->GetFirstTag("name").GetBody().length() )
		{
			gtk_label_set_text(GTK_LABEL(ClientLabelText), query_iq->GetFirstTag("name").GetBody().c_str());
			gtk_widget_show(ClientLabel);
			gtk_widget_show(ClientLabelText);
		}
		if( query_iq->GetFirstTag("version").GetBody().length() )
		{
			gtk_label_set_text(GTK_LABEL(VersionLabelText), query_iq->GetFirstTag("version").GetBody().c_str());
			gtk_widget_show(VersionLabel);
			gtk_widget_show(VersionLabelText);
		}	
		if( query_iq->GetFirstTag("os").GetBody().length() )
		{
			gtk_label_set_text(GTK_LABEL(OSLabelText), query_iq->GetFirstTag("os").GetBody().c_str());
			gtk_widget_show(OSLabel);
			gtk_widget_show(OSLabelText);
		}	
			
	}	
			
	return 1;
}
