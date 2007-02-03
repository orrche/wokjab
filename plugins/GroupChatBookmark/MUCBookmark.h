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
// Class: MUCBookmark
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Thu Feb  1 12:33:14 2007
//

#ifndef _MUCBOOKMARK_H_
#define _MUCBOOKMARK_H_


#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;

/**
 * Class to manage groupchat bookmarks
 */
class MUCBookmark : public WoklibPlugin
{
	public:
		MUCBookmark(WLSignal *wls);
		 ~MUCBookmark();

			int SignIn(WokXMLTag *tag);
			int BookmarkResponse(WokXMLTag *tag);
	protected:

	
	
};


#endif	//_MUCBOOKMARK_H_

