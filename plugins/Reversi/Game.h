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
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Sun Aug 28 00:24:18 2005
//

#ifndef _GAME_H_
#define _GAME_H_


#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <gtk/gtk.h>
#include <glade/glade.h>

using namespace Woklib;

class Game : public WLSignalInstance
{
	public:
		Game(WLSignal *wls, WokXMLTag *tag, bool mymove = false);
		 ~Game();
	
		int Move(WokXMLTag *tag);
		void MyMove(int x, int y);
		int MoveTo(int x, int y, std::string color);
		void Redraw();

		static gboolean Expose (GtkWidget *widget, GdkEventExpose *event, Game *c);
		static gboolean Button (GtkButton *button, GdkEventButton *event, Game *c);
		static void Destroy( GtkWidget *widget, Game *c );
		bool CanMove(bool my);
		bool Possibule(const std::string &color, int x, int y, int *dir);
		void GameEnded();
	protected:
		std::string session, jid, id;
		std::map <int, std::map < int , std::string > > markers;
		GladeXML *xml;
		bool mymove;
		int scoreb, scorew;
		std::string colorm, coloro;
		bool ended;
		
		GtkWidget *board_img;
		GtkWidget *black_img;
		GtkWidget *white_img;
	
};


#endif	//_GAME_H_

