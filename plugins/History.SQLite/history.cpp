/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2007-2009 <nedo80@gmail.com>
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
#include <sys/wait.h>

#include <iostream>
#include <sstream>
#include <unistd.h>

#include "history.hpp"
static int callback(void *NotUsed, int argc, char **argv, char **azColName){
	int i;
	WokXMLTag *tag = (WokXMLTag *)NotUsed;
	if ( NotUsed )
	{
		WokXMLTag &row = tag->AddTag("row");
		for(i=0; i<argc; i++)
		{
			WokXMLTag &value = row.AddTag(azColName[i]);
			if ( value.GetName() == "xml")
				value.Add(argv[i] ? DeXMLisize(argv[i]) : "NULL");
			else
				value.AddText(argv[i] ? argv[i] : "NULL");
		}
	}
	return 0;
}

History::History(WLSignal *wls): WoklibPlugin(wls)
{		
	pid = 0;
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
			std::cerr << "Can't open database: "<< sqlite3_errmsg(db) << std::endl;
			sqlite3_close(db);
	}
	else
	{
		char *zErrMsg;
		
		rc = sqlite3_exec( db, "CREATE TABLE history(relation varchar(255), resource varchar(255), time timestamp, to_jid varchar(255), from_jid varchar(255), xml blob)", callback, 0, &zErrMsg );
		if( rc!=SQLITE_OK )
		{
			std::cerr << "SQL error: " << zErrMsg << std::endl;
			sqlite3_free( zErrMsg );
		}

		// Here we then hook the signals since there is no problem with the database.
		
		EXP_SIGHOOK("Jabber XML Message Send", &History::Outgoing , 950);
		EXP_SIGHOOK("Jabber XML Message Normal", &History::Incomming , 950);
		EXP_SIGHOOK("Jabber History GetLast", &History::GetLast, 1000);
	}
}


History::~History()
{
	sqlite3_close(db);
}

int
History::GetLast(WokXMLTag *tag)
{
	char *zErrMsg;
	std::string limit;
	if ( tag->GetAttr("limit").empty() )
		limit = "15";
	else
		limit = tag->GetAttr("limit");
	
	std::string query;
	if( tag->GetAttr("relation").empty() )
		query = "SELECT * FROM ( SELECT * FROM history WHERE time>(strftime('%s', 'now')-172800) ORDER BY time DESC LIMIT " + limit + " ) ORDER BY time ASC";
	else
		query = "SELECT * FROM ( SELECT * FROM history WHERE relation='" + tag->GetAttr("relation") + "' AND time>(strftime('%s', 'now')-172800) ORDER BY time DESC LIMIT " + limit + " ) ORDER BY time ASC";
	
	WokXMLTag &ret = tag->AddTag("history");
	
	int rc = sqlite3_exec( db, query.c_str(), callback, &ret, &zErrMsg );
	if( rc!=SQLITE_OK )
	{
		std::cerr << "SQL error: " << zErrMsg << std::endl;
		sqlite3_free( zErrMsg );
	}

	return 1;	
}

void
History::PushToSQL(std::string command)
{
	char *zErrMsg;
	
	int status;
	if ( pid )
		waitpid( pid, &status, WNOHANG);

	if ( WIFEXITED(status) || WTERMSIG(status) || !pid)
	{
		if ( !WEXITSTATUS(status) )
			lingering_command = "";
		
		lingering_command += command;
		
		pid = fork();
		if ( pid == 0 )
		{
			sqlite3 *fork_db;
			int rc = sqlite3_open((std::string(g_get_home_dir()) + "/.wokjab/history/history.db").c_str(), &fork_db);
			if( rc ){
				std::cerr << "Can't open database: " << sqlite3_errmsg(fork_db) << std::endl;
				sqlite3_close(fork_db);
				_exit(-1);
			}
			
			rc = sqlite3_exec( fork_db, (lingering_command).c_str(), callback, 0, &zErrMsg );
			if( rc!=SQLITE_OK )
			{
				std::cerr << "SQL error: " << zErrMsg << std::endl;
				sqlite3_free( zErrMsg );
				sqlite3_close(fork_db);
				_exit(-1);
			}
			sqlite3_close(fork_db);
			
			_exit(0);
		}		
	}
	
	
	
	
}


int
History::Outgoing(WokXMLTag *tag)
{
	std::string jid = tag->GetFirstTag("message").GetAttr("to");
	std::string resource;
	if ( jid.find("/") != std::string::npos )
	{
		resource = jid.substr(jid.find("/")+1);
		jid = jid.substr(0, jid.find("/"));
	}
	else
		resource = "";
	
	PushToSQL("INSERT INTO history (relation, resource, to_jid, from_jid, xml, time) VALUES ('"+ jid +"', '" + resource + "', '" + tag->GetFirstTag("message").GetAttr("to") + 
					  		"', '" + tag->GetFirstTag("message").GetAttr("from") + "' , '" + XMLisize(tag->GetStr()) + "', strftime('%s','now'));");

	return 1;
}


int
History::Incomming(WokXMLTag *tag)
{
	std::string jid = tag->GetFirstTag("message").GetAttr("from");
	std::string resource;	
	if ( jid.find("/") != std::string::npos )
	{
		resource = jid.substr(jid.find("/")+1);
		jid = jid.substr(0, jid.find("/"));
	}
	else
		resource = "";
	
	PushToSQL("INSERT INTO history (relation, resource, to_jid, from_jid, xml, time) VALUES ('"+ jid +"', '" + resource + "', '" + tag->GetFirstTag("message").GetAttr("to") + 
								"', '" + tag->GetFirstTag("message").GetAttr("from") + "' , '" + XMLisize(tag->GetStr()) + "', strftime('%s','now'));");
	
	
	return 1;
}

