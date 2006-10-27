/***************************************************************************
 *  Copyright (C) 2003-2005  Kent Gustavsson <oden@gmx.net>
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



#ifndef _RESOURCE_H_
#define _RESOURCE_H_

#include <string>


class Resource
{
	public:
		Resource();
		 ~Resource();
	
		const int & GetPriority();
		const std::string & GetShow();
		const std::string & GetResource();
		const std::string & GetStatus();
		void SetPriority(const int & p);
		void SetShow( const std::string &show);
		void SetResource( const std::string &resource);
		void SetStatus( const std::string &status);
		
	protected:
		std::string resource;
		std::string show;
		std::string status;
		int presence;
		int priority;
		
	
};


#endif	//_RESOURCE_H_
