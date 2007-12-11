/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2007 <nedo80@gmail.com>
 * 
 * wokjab is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * wokjab is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with wokjab.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>


#include "history.hpp"
static int callback(void *NotUsed, int argc, char **argv, char **azColName){
  int i;
  NotUsed=0;

  for(i=0; i<argc; i++){
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

History::History(WLSignal *wls): WoklibPlugin(wls)
{		
	{
		std::string filename = (std::string(g_get_home_dir()) + "/.wokjab/history/history.db");
		std::string::size_type pos = filename.find("/");
		while( pos != std::string::npos )
		{
	#ifdef __WIN32
				mkdir(filename.substr(0, pos).c_str());
	#else
				mkdir(filename.substr(0, pos).c_str(), 0700);
	#endif
				pos = filename.find("/", pos + 1);
		}
	}
	int rc = sqlite3_open((std::string(g_get_home_dir()) + "/.wokjab/history/history.db").c_str(), &db);
	if( rc ){
			fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
			sqlite3_close(db);
	}
	else
	{
		char *zErrMsg;
		
		rc = sqlite3_exec( db, "CREATE TABLE history(relation varchar(255), time timestamp, to_jid varchar(255), from_jid varchar(255), xml blob)", callback, 0, &zErrMsg );
		if( rc!=SQLITE_OK )
		{
			fprintf( stderr, "SQL error: %s\n", zErrMsg );
			sqlite3_free( zErrMsg );
		}

		// Here we then hook the signals since there is no problem with the database.
		
		EXP_SIGHOOK("Jabber XML Message Send", &History::Outgoing , 1000);
		EXP_SIGHOOK("Jabber XML Message Normal", &History::Incomming , 1000);
	}
}


History::~History()
{
	sqlite3_close(db);
}

int
History::Outgoing(WokXMLTag *tag)
{
	char *zErrMsg;
	
	int rc = sqlite3_exec( db, ("INSERT INTO history (relation, to_jid, from_jid, xml) VALUES ('"+ tag->GetFirstTag("message").GetAttr("to") +"', '" + tag->GetFirstTag("message").GetAttr("to") + 
					  		"', '" + tag->GetFirstTag("message").GetAttr("from") + "' , '" + XMLisize(tag->GetStr()) + "')").c_str(), callback, 0, &zErrMsg );
	if( rc!=SQLITE_OK )
	{
		fprintf( stderr, "SQL error: %s\n", zErrMsg );
		sqlite3_free( zErrMsg );
	}

	return 1;
}


int
History::Incomming(WokXMLTag *tag)
{
	char *zErrMsg;
		
	int rc = sqlite3_exec( db, ("INSERT INTO history (relation, to_jid, from_jid, xml) VALUES ('"+ tag->GetFirstTag("message").GetAttr("from") +"', '" + tag->GetFirstTag("message").GetAttr("to") + 
					  		"', '" + tag->GetFirstTag("message").GetAttr("from") + "' , '" + XMLisize(tag->GetStr()) + "')").c_str(), callback, 0, &zErrMsg );
	if( rc!=SQLITE_OK )
	{
		fprintf( stderr, "SQL error: %s\n", zErrMsg );
		sqlite3_free( zErrMsg );
	}

	
	return 1;
}

