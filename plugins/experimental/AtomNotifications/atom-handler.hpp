/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2008 <nedo80@gmail.com>
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

#ifndef _ATOM_HANDLER_HPP_
#define _ATOM_HANDLER_HPP_



#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>


using namespace Woklib;
class AtomHandler;

class AtomHandler: public WoklibPlugin 
{
public:
	AtomHandler(WLSignal *wls);
			
			
	int NewAtom(WokXMLTag *tag);
	int Link(WokXMLTag *tag);
	int Menu(WokXMLTag *tag);
protected:
	std::list <WokXMLTag *> atomitems;
private:

};

#endif // _ATOM_HANDLER_HPP_
