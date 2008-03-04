/***************************************************************************
 *  Copyright (C) 2003-2008  Kent Gustavsson <nedo80@gmail.com>
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
// Class: Sound
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Wed Sep 14 13:47:19 2005
//

#include "Sound.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

Sound::Sound(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("Jabber Event Add", &Sound::Event, 1000);
	
	config = new WokXMLTag(NULL, "NULL");
	EXP_SIGHOOK("Config XML Change /sound", &Sound::Config, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/sound");
	wls->SendSignal("Config XML Trigger", &conftag);

}


Sound::~Sound()
{
	delete config;
}

int
Sound::Event(WokXMLTag *tag)
{
	std::list < WokXMLTag *> ::iterator iter;
	
	for( iter = tag->GetTagList("item").begin() ; iter != tag->GetTagList("item").end() ; iter++)
	{
		if( true )
		{
			int pid,status;
			switch (pid = fork()){
				case 0:
					int pid2;
					switch (fork())
					{
						case 0:
							system(config->GetFirstTag("newmessage").GetAttr("data").c_str());
							_exit(1);
					}
					_exit(1);
			}
			
			if(pid > 0)
				waitpid(pid, &status, 0);
		}
	}
	return 1;
}

int
Sound::Config(WokXMLTag *tag)
{

	delete config;
	config = new WokXMLTag(tag->GetFirstTag("config"));
	
	tag->GetFirstTag("config").GetFirstTag("newmessage").AddAttr("type", "string");

	return 1;
}	
