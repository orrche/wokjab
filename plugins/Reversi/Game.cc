/***************************************************************************
 *  Copyright (C) 2005  Kent Gustavsson <oden@gmx.net>
 ****************************************************************************/
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
// Class: Game
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Sun Aug 28 00:24:18 2005
//

#include "Game.h"
#include <sstream>
#include <string.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

Game::Game(WLSignal *wls, WokXMLTag *tag, bool mymove) : WLSignalInstance(wls),
mymove(mymove)
{
	ended = false;
	board_img = gtk_image_new_from_file(PACKAGE_DATA_DIR"/wokjab/board.png");
	black_img = gtk_image_new_from_file(PACKAGE_DATA_DIR"/wokjab/black.png");
	white_img = gtk_image_new_from_file(PACKAGE_DATA_DIR"/wokjab/white.png");
	
	session = tag->GetAttr("session");
	jid = tag->GetFirstTag("iq").GetAttr("from");
	id = tag->GetFirstTag("iq").GetAttr("id");
	
	xml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/jabberreversi.glade", "ReversiWindow", NULL);

	if ( mymove )
	{
		colorm = "white";
		coloro = "black";
	}
	else
	{
		colorm = "black";
		coloro = "white";
	}
	
	markers[4][4] = "white";
	markers[4][5] = "black";
	markers[5][4] = "black";
	markers[5][5] = "white";
	
	if( xml )
	{
		GtkWidget *window1;
					
		window1 = glade_xml_get_widget (xml, "ReversiWindow");
		
		g_signal_connect (G_OBJECT(glade_xml_get_widget (xml, "board")), "expose_event", 
						G_CALLBACK (Game::Expose), this);
		g_signal_connect (G_OBJECT(glade_xml_get_widget (xml, "eventboard")), "button_press_event", 
						G_CALLBACK (Game::Button), this);
		g_signal_connect (G_OBJECT(window1), "delete_event", 
						G_CALLBACK (Game::Destroy), this);
		gtk_widget_show (window1);
	}
	
	scorew = 2;
	scoreb = 2;
	
	if( mymove )
		gtk_label_set_text( GTK_LABEL(glade_xml_get_widget (xml, "StatusLabel")), "My Move");
	else
		gtk_label_set_text( GTK_LABEL(glade_xml_get_widget (xml, "StatusLabel")), "Opponents Move");
	
	std::string color_label(" (");
	color_label += colorm;
	color_label += ")";
	
	gtk_label_set_text( GTK_LABEL(glade_xml_get_widget (xml, "my_color_label")), color_label.c_str());
}


Game::~Game()
{
	ended = true;
	if(xml)
	{
		gtk_widget_destroy( glade_xml_get_widget (xml, "ReversiWindow") );
		g_object_unref(xml);
		xml = NULL;
	}
	
	/*
	gtk_widget_destroy(board_img);
	gtk_widget_destroy(black_img);
	gtk_widget_destroy(white_img);
	*/
}

bool
Game::Possibule(const std::string &color, int x, int y, int *dir)
{
	int direction[8][2] = {{1,1},{1,0},{1,-1},{0,1},{0,-1},{-1,1}, {-1,0}, {-1,-1}};
	bool legal = false;

	memset(dir, 0, sizeof(int)*8);
	if ( markers[x][y] != "")
		return false;

	for( int i = 0 ; i < 8 ; i++)
	{
		int nx,ny;
		nx = x; ny = y;
		nx += direction[i][0];
		ny += direction[i][1];
			
		if( nx >0 && nx < 9 && ny >0 && ny < 9 )
		{
			if( markers[nx][ny] != color && markers[nx][ny].size() )
			{
				nx += direction[i][0];
				ny += direction[i][1];
				while( nx >0 && nx < 9 && ny >0 && ny < 9 )
				{
					if( markers[nx][ny] == color )
					{
						dir[i] = 1;
						legal = true;
						break;
					}
					else if ( markers[nx][ny]== "" )
						break;
						
					nx += direction[i][0];
					ny += direction[i][1];
				}
			}
		}
	}
	
	return legal;
}

int
Game::MoveTo(int x, int y, std::string color)
{
	int direction[8][2] = {{1,1},{1,0},{1,-1},{0,1},{0,-1},{-1,1}, {-1,0}, {-1,-1}};
	int take[8];

	if(!Possibule(color,x,y,take))
		return false;
	
	for( int i = 0; i < 8 ; i++)
	{
		if (!take[i])
			continue;
		int nx,ny;
		nx = x; ny = y;
		
		nx += direction[i][0];
		ny += direction[i][1];
		while( nx >0 && nx < 9 && ny >0 && ny < 9 )
		{
			if( markers[nx][ny] == color )
				break;
			if( color == colorm )
			{
				scoreb--;
				scorew++;
			}
			else
			{
				scoreb++;
				scorew--;
			}
			markers[nx][ny] = color;	
			nx += direction[i][0];
			ny += direction[i][1];
		}
	}
	
	markers[x][y] = color;
	return true;
}

void
Game::GameEnded()
{
	ended = true;
	Redraw();
}

bool
Game::CanMove(bool my)
{
	int dir[8];
	std::string color;
	if (my)
		color = colorm;
	else
		color = coloro;


	for( int x = 1; x < 9 ; x++ )
	{
		for ( int y = 1; y < 9 ; y++)
		{
			if( Possibule(color, x,y,dir))
				return true;
		}
	}
	return false;
}

int
Game::Move(WokXMLTag *tag)
{
	if( tag->GetFirstTag("message").GetFirstTag("x").GetTagList("forfeit").size() )
	{
		ended = true;
		mymove = false;
		gtk_label_set_text( GTK_LABEL(glade_xml_get_widget (xml, "StatusLabel")), "Opponent forfeited");
		
		return 1;
	}
	
	if( mymove )
	{
		std::stringstream str;
		str << "Reversi: " << jid << " tries to move while its your turn, aborted";
		woklib_error(wls, str.str());
		return 1;
	}
		
	WokXMLTag &move = tag->GetFirstTag("message").GetFirstTag("x").GetFirstTag("move");
	if( MoveTo(atoi(move.GetFirstTag("x").GetBody().c_str()),atoi(move.GetFirstTag("y").GetBody().c_str()),coloro) )
	{
		scoreb++;
		if( scoreb + scorew == 64 || !scoreb || !scorew )
		{
			GameEnded();
			return 1;
		}
		
		if(CanMove(true))
			mymove = true;
		Redraw();
	}
	else
	{
		std::stringstream str;
		str << "Reversi: " << jid << " tries to do an invalid move";
		woklib_error(wls, str.str());
	}
	return 1;
}

void
Game::Redraw()
{
	GtkWidget *board = glade_xml_get_widget (xml, "board");
	gtk_widget_queue_draw_area (board, 0, 0, board->allocation.width, board->allocation.height);
	
	std::stringstream buf1,buf2;
	buf2 << scoreb;
	buf1 << scorew;
	gtk_label_set_text( GTK_LABEL(glade_xml_get_widget (xml, "HomepointLabel")), buf1.str().c_str());
	gtk_label_set_text( GTK_LABEL(glade_xml_get_widget (xml, "AwaypointLabel")), buf2.str().c_str());
	if( mymove )
		gtk_label_set_text( GTK_LABEL(glade_xml_get_widget (xml, "StatusLabel")), "My Move");
	else
		gtk_label_set_text( GTK_LABEL(glade_xml_get_widget (xml, "StatusLabel")), "Opponents Move");
	
}


void
Game::MyMove(int x, int y)
{
	if(!mymove)
	{
		if ( !ended )	
			woklib_message(wls, "Not your turn");
		return;
	}
		
	std::stringstream buf1,buf2;
	WokXMLTag msgtag(NULL, "message");
	msgtag.AddAttr("session", session);
	WokXMLTag &msg = msgtag.AddTag("message");
	msg.AddAttr("to", jid);
	msg.AddTag("thread").AddText(id);
	WokXMLTag &xtag = msg.AddTag("x");
	xtag.AddAttr("xmlns", "http://rhymbox.com/protocol/reversi.html#1.0");
	WokXMLTag &move = xtag.AddTag("move");
	buf1 << x;
	move.AddTag("x").AddText(buf1.str());
	buf2 << y;
	move.AddTag("y").AddText(buf2.str());
	
	if( MoveTo(x,y, colorm)) 
	{
		scorew++;
		wls->SendSignal("Jabber XML Send", &msgtag);
		if( scoreb + scorew == 64 || !scoreb || !scorew )
		{
			GameEnded();
			return;
		}
		if(CanMove(false))
			mymove = false;
		Redraw();
	}
	else
		woklib_message(wls, "That move isn't possibule");
}

void
Game::Destroy( GtkWidget *widget, GdkEvent *event, Game *c)
{
	if( !c->ended && c->scoreb + c->scorew != 64 && c->scoreb && c->scorew)
	{
		WokXMLTag mtag(NULL,"message");
		mtag.AddAttr("session", c->session);
		WokXMLTag &msgtag = mtag.AddTag("message");
		msgtag.AddAttr("to", c->jid);
		msgtag.AddTag("thread").AddText(c->id);
		WokXMLTag &xtag = msgtag.AddTag("x");
		xtag.AddAttr("xmlns", "http://rhymbox.com/protocol/reversi.html#1.0");
		xtag.AddTag("forfeit");
		
		c->wls->SendSignal("Jabber XML Send", &mtag);
	}
	
	if( c->xml ) 
	{
		WokXMLTag removetag(NULL, "remove");
		removetag.AddAttr("id", c->id);
		removetag.AddAttr("session", c->session);
		
		c->wls->SendSignal("Jabber Games Reversi RemoveSession", &removetag);
	}
}

gboolean
Game::Button (GtkButton *button, GdkEventButton *event, Game *c)
{
	int x,y;
	
	x = static_cast <int> ((event->x - 1) / 42 + 1);
	y = static_cast <int> ((event->y - 1) / 42 + 1);
	
	if ( x > 0 && x < 9 && y > 0 && y < 9 )
		c->MyMove(x,y);

	return TRUE;
}

gboolean
Game::Expose (GtkWidget *widget, GdkEventExpose *event, Game *c)
{

	gdk_draw_pixbuf (GDK_DRAWABLE(widget->window),
                  widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                  gtk_image_get_pixbuf (GTK_IMAGE(c->board_img)),
                  0,
                  0,
                  0,
                  0,
                  gdk_pixbuf_get_width (gtk_image_get_pixbuf(GTK_IMAGE(c->board_img))),
                  gdk_pixbuf_get_height (gtk_image_get_pixbuf(GTK_IMAGE(c->board_img))),
                  GDK_RGB_DITHER_NONE,
                  0,
                  0);
							
	std::map< int , std::map<int, std::string> >::iterator xiter;
	std::map<int, std::string>::iterator yiter;
	
	for( xiter = c->markers.begin(); xiter != c->markers.end() ; xiter++)
	{
		for( yiter = c->markers[xiter->first].begin(); yiter != c->markers[xiter->first].end(); yiter++)
		{
			GtkWidget *img;
			if(yiter->second == "white")
				img = c->white_img;
			else if ( yiter->second == "black")
				img = c->black_img;
			else
				continue;
			
			gdk_draw_pixbuf (GDK_DRAWABLE(widget->window),
                  widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                  gtk_image_get_pixbuf (GTK_IMAGE(img)),
                  0,
                  0,
                  xiter->first*41 - 41,
                  yiter->first*41 - 41,
                  gdk_pixbuf_get_width (gtk_image_get_pixbuf(GTK_IMAGE(img))),
                  gdk_pixbuf_get_height (gtk_image_get_pixbuf(GTK_IMAGE(img))),
                  GDK_RGB_DITHER_NONE,
                  0,
                  0);
	
		}
	}
	return TRUE;
}

