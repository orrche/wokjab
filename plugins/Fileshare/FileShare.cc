/***************************************************************************
 *  Copyright (C) 2003-2009  Kent Gustavsson <nedo80@gmail.com>
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



#include "FileShare.h"
#include "FileListWid.h"
#include "SearchWid.h"
#include "File.h"

#include <dirent.h>
#include <sstream>
#include	<sys/stat.h>
#include	<sys/types.h>

#include "zlib.h"

FileShare::FileShare(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("Jabber Disco Items Get Node http://jabber.org/protocol/commands", &FileShare::Commands, 500);
	EXP_SIGHOOK("Jabber AdHoc Command Exec p2p search", &FileShare::Com_Search, 500);
	EXP_SIGHOOK("Jabber AdHoc Command Exec p2p get_file_list", &FileShare::Com_GetFileList, 500);
	EXP_SIGHOOK("Jabber AdHoc Command Exec p2p download", &FileShare::Com_Download, 500);
							
	EXP_SIGHOOK("Jabber GUI GetJIDMenu", &FileShare::Menu, 1000);
	EXP_SIGHOOK("Jabber FileShare FileList View", &FileShare::View, 1000);
	EXP_SIGHOOK("Jabber XML IQ New fileshare get xmlns:http://sf.wokjab.net/fileshare", &FileShare::ListRequest, 1000);
	EXP_SIGHOOK("Jabber FileShare SearchWid", &FileShare::OpenSearchWid, 1000);
	EXP_SIGHOOK("Jabber FileShare Search", &FileShare::Search, 1000);
	EXP_SIGHOOK("Jabber FileShare MenuDownload", &FileShare::MenuDownload, 1000);
	EXP_SIGHOOK("Get Main Menu", &FileShare::MainMenu, 1000);
	EXP_SIGHOOK("Jabber FileShare Rebuild", &FileShare::Rebuild, 1000);
	EXP_SIGHOOK("Jabber XML Message xmlns http://sf.wokjab.net/fileshare", &FileShare::IncommingSearch, 1000);
	fileshareid = 0;
	search_result = NULL;
	n_id = 0;
	
	config = new WokXMLTag(NULL, "NULL");
	EXP_SIGHOOK("Config XML Change /fileshare", &FileShare::Config, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/fileshare");
	wls->SendSignal("Config XML Trigger", &conftag);
	
	path = "";
	
	
	std::string filename = (std::string(g_get_home_dir()) + "/.wokjab/fileshare/filelist.db");

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
	int rc = sqlite3_open(filename.c_str(), &db);
	if( rc ){
			fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
			sqlite3_close(db);
	}
	else
	{
		char *zErrMsg = 0;
		std::string query = "CREATE TABLE filelist (id TEXT PRIMARY KEY, file TEXT, path TEXT, size INTEGER);";
		int rc = sqlite3_exec(db, query.c_str(), (int(*)(void *,int,char**,char**)) FileShare::sql_callback, this, &zErrMsg);
		if( rc!=SQLITE_OK ){
			fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
		}
	}
}

FileShare::~FileShare()
{
	delete config;

}


void 
FileShare::SendFile(std::string session, std::string jid, std::string id)
{
	std::stringstream sid;
	sid << "fileshare-" << fileshareid++ << "-";
	for(int i = 0; i < 10; i++)
	{
		sid << (char) (rand() % 20 + 'a' );
	}

	char *zErrMsg = 0;
	filetosend = "";

	for ( unsigned int pos = 0 ; id.find("'", pos) != std::string::npos ; )
	{
		pos = id.find("'", pos );
		id.replace(pos, 1, "''");
		pos+=2;

		if ( pos >= id.size() )
			break;
	}

	filetosend = "";
	std::string query = "SELECT * FROM filelist WHERE id='" + id + "';";
	/* The callback is changing the filetosend variable */
	int rc = sqlite3_exec(db, query.c_str(), (int(*)(void *,int,char**,char**)) FileShare::sql_callback, this, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	std::cout << "ID: " << id << " file: " << filetosend << std::endl;
	if ( filetosend != "" )
	{
		WokXMLTag send(NULL, "send");
		send.AddAttr("to", jid);
		send.AddAttr("session", session);
		send.AddAttr("name", filetosend);
		send.AddAttr("sid", sid.str());
		send.AddAttr("popup", "false");
		send.AddAttr("event", "false");

		send.AddAttr("proxy_type", "auto");
		wls->SendSignal("Jabber Stream File Send", send);
	}
}

int
FileShare::Com_Download(WokXMLTag *tag)
{
		if ( ! HasPermission(tag->GetAttr("session"), tag->GetFirstTag("iq").GetAttr("from")) )
				return 1;
		
		
		WokXMLTag mesg("message");
    mesg.AddAttr("session", tag->GetAttr("session"));
    WokXMLTag &iq = mesg.AddTag("iq");
    iq.AddAttr("to", tag->GetFirstTag("iq").GetAttr("from"));
    iq.AddAttr("type", "result");
    iq.AddAttr("id", tag->GetFirstTag("iq").GetAttr("id"));
    WokXMLTag &comm = iq.AddTag("command");
    comm.AddAttr("xmlns", "http://jabber.org/protocol/commands");
    comm.AddAttr("node", "p2p download");
    comm.AddAttr("sessionid", "theoneandonly");
		
		if ( tag->GetFirstTag("iq").GetFirstTag("command", "http://jabber.org/protocol/commands").GetAttr("sessionid") == "" )
		{
				
			comm.AddAttr("status", "executing");
    
		  WokXMLTag &xtag = comm.AddTag("x");
		  xtag.AddAttr("xmlns", "jabber:x:data");
		  xtag.AddAttr("type", "form");
		  xtag.AddTag("title").AddText("File Download");
		  xtag.AddTag("instructions").AddText("Enter File ID");
									 
			WokXMLTag &field = xtag.AddTag("field");
		  field.AddAttr("var", "search");
		  field.AddAttr("label", "FileID");
		  field.AddAttr("type", "text-single" );
		}
		else
		{
			std::string id = tag->GetFirstTag("iq").GetFirstTag("command", "http://jabber.org/protocol/commands").GetFirstTag("x", "jabber:x:data").GetFirstTag("field").GetFirstTag("value").GetBody();
			comm.AddAttr("status", "completed");
			if ( id	 != "" )
			{		
				SendFile(tag->GetAttr("session"), tag->GetFirstTag("iq").GetAttr("from"), id);
			}
		}
		wls->SendSignal("Jabber XML Send", mesg);
		return 1;
}

int
FileShare::Com_GetFileList(WokXMLTag *tag)
{
		if ( ! HasPermission(tag->GetAttr("session"), tag->GetFirstTag("iq").GetAttr("from")) )
				return 1;
		
				
		std::stringstream sid;
		sid << "fileshare-" << fileshareid++ << "-";
		for(int i = 0; i < 10; i++)
		{
				sid << (char) (rand() % 20 + 'a' );
		}
		
		std::string filelistfile = (std::string(g_get_home_dir()) + "/.wokjab/fileshare/filelist.xml.gz").c_str();
		
		WokXMLTag send(NULL, "send");
		send.AddAttr("to", tag->GetFirstTag("iq").GetAttr("from"));
		send.AddAttr("session", tag->GetAttr("session"));
		send.AddAttr("name", filelistfile);
		send.AddAttr("sid", sid.str());
		send.AddAttr("popup", "false");
		send.AddAttr("event", "false");
		send.AddAttr("proxy_type", "auto");
		wls->SendSignal("Jabber Stream File Send", send);
		
		return 1;	
}

int
FileShare::Com_Search(WokXMLTag *tag)
{
		if ( ! HasPermission(tag->GetAttr("session"), tag->GetFirstTag("iq").GetAttr("from")) )
				return 1;
	
		std::string action = tag->GetFirstTag("iq").GetFirstTag("command").GetAttr("action");
		if( action == "cancel" )
		{
				WokXMLTag mesg("message");
		    mesg.AddAttr("session", tag->GetAttr("session"));
		    WokXMLTag &iq = mesg.AddTag("iq");
		    iq.AddAttr("to", tag->GetFirstTag("iq").GetAttr("from"));
		    iq.AddAttr("type", "result");
		    iq.AddAttr("id", tag->GetFirstTag("iq").GetAttr("id"));
		    WokXMLTag &comm = iq.AddTag("command");
		    comm.AddAttr("xmlns", "http://jabber.org/protocol/commands");
		    comm.AddAttr("node", "p2p search");
		    comm.AddAttr("status", "canceled");
		    comm.AddAttr("sessionid", "theoneandonly");
		    
		    wls->SendSignal("Jabber XML Send", mesg);
		    return 1;
		}
		
    WokXMLTag mesg("message");
    mesg.AddAttr("session", tag->GetAttr("session"));
    WokXMLTag &iq = mesg.AddTag("iq");
    iq.AddAttr("to", tag->GetFirstTag("iq").GetAttr("from"));
    iq.AddAttr("type", "result");
    iq.AddAttr("id", tag->GetFirstTag("iq").GetAttr("id"));
    WokXMLTag &comm = iq.AddTag("command");
    comm.AddAttr("xmlns", "http://jabber.org/protocol/commands");
    comm.AddAttr("node", "p2p search");
    comm.AddAttr("status", "executing");
    comm.AddAttr("sessionid", "theoneandonly");
    WokXMLTag &actiontag = comm.AddTag("actions");
    actiontag.AddAttr("execute", "next");
    actiontag.AddTag("next");
    WokXMLTag &xtag = comm.AddTag("x");
    xtag.AddAttr("xmlns", "jabber:x:data");
    xtag.AddAttr("type", "form");
    xtag.AddTag("title").AddText("File Search");
    xtag.AddTag("instructions").AddText("Search for files");
								 
		WokXMLTag &field = xtag.AddTag("field");
    field.AddAttr("var", "search");
    field.AddAttr("label", "Search");
    field.AddAttr("type", "text-single" );
								 
		
		std::list <WokXMLTag *>::iterator fielditer;
		std::list <WokXMLTag *> *fieldlist =  &tag->GetFirstTag("iq").GetFirstTag("command", "http://jabber.org/protocol/commands").GetFirstTag("x", "jabber:x:data").GetTagList("field");
																																						
		for ( fielditer = fieldlist->begin() ; fielditer != fieldlist->end() ; fielditer++)
		{
			if ( (*fielditer)->GetAttr("var").substr(0, 7) == "file://")
			{
					if ( (*fielditer)->GetFirstTag("value").GetBody() == "1" || (*fielditer)->GetFirstTag("value").GetBody() == "true" )
					{
						SendFile(tag->GetAttr("session"), tag->GetFirstTag("iq").GetAttr("from"),(*fielditer)->GetAttr("var").substr(7) );
					}
			}
			if ( 	(*fielditer)->GetAttr("var") == "search" && (*fielditer)->GetFirstTag("value").GetBody() != "" )
			{		
					std::string query = "SELECT * FROM filelist WHERE id LIKE '%" + XMLisize((*fielditer)->GetFirstTag("value").GetBody()) + "%' LIMIT 10;";
				/* The callback is changing the filetosend variable */
	
				delete search_result;
				search_result = new WokXMLTag (NULL, "search");
																																																					 
				char *zErrMsg = 0;
				int rc = sqlite3_exec(db, query.c_str(), (int(*)(void *,int,char**,char**)) FileShare::sql_callback_search, this, &zErrMsg);
	
			 	if( rc!=SQLITE_OK )
				{
					fprintf(stderr, "SQL error: %s\n", zErrMsg);
					sqlite3_free(zErrMsg);
				}
	
				std::cout << *search_result << std::endl;
				std::list <WokXMLTag *>::iterator res_iter;
	
				WokXMLTag msg(NULL, "message");
				msg.AddAttr("session", tag->GetAttr("session"));
				WokXMLTag &message = msg.AddTag("message");
				message.AddAttr("to", tag->GetFirstTag("message").GetAttr("from"));
				if ( !tag->GetFirstTag("message").GetAttr("type").empty() )
					message.AddAttr("type", tag->GetFirstTag("message").GetAttr("type"));
				message.AddTag("thread").AddText(tag->GetFirstTag("message").GetFirstTag("thread").GetBody());
				WokXMLTag &x = message.AddTag("x");
				x.AddAttr("xmlns", "http://sf.wokjab.net/fileshare");
				x.AddAttr("type", "set");
							
				if ( search_result )
				{
					for( res_iter = search_result->GetTagList("result").begin() ; res_iter != search_result->GetTagList("result").end() ; res_iter++)
					{
						WokXMLTag &filefield = xtag.AddTag("field");
						filefield.AddAttr("type", "boolean");
						filefield.AddAttr("var", "file://" +  (*res_iter)->GetAttr("id"));
						filefield.AddAttr("label", (*res_iter)->GetAttr("path"));
						
						/*
						WokXMLTag &filevalue = filefield.AddTag("value");
						filevalue.AddText( "Path: " + (*res_iter)->GetAttr("path") + " id: " + (*res_iter)->GetAttr("id"));
						*/
							
						WokXMLTag &itemtag = filefield.AddTag("file", "http://sf.wokjab.net/fileshare");
						itemtag.AddAttr("id", (*res_iter)->GetAttr("id"));
						itemtag.AddAttr("name", (*res_iter)->GetAttr("path"));
						itemtag.AddAttr("size", (*res_iter)->GetAttr("size"));
						itemtag.AddAttr("hash", (*res_iter)->GetAttr("hash"));
					}
		
					delete search_result;
					search_result = NULL;
				}
			}
		}
								 
		wls->SendSignal("Jabber XML Send", mesg);

		return 1;
}


int
FileShare::Commands(WokXMLTag *tag)
{
	if ( ! HasPermission(tag->GetAttr("session"), tag->GetAttr("jid")))
				return 1;
		
		
	WokXMLTag querytag(NULL, "query");
	WokXMLTag &itemtag = querytag.AddTag("item");
	itemtag.AddAttr("session", tag->GetAttr("session"));
	wls->SendSignal("Jabber Connection GetUserData", &querytag);
		
	WokXMLTag &item = tag->AddTag("item");
	item.AddAttr("node", "p2p search");
	item.AddAttr("name", "P2P Sök");
	item.AddAttr("jid", querytag.GetFirstTag("item").GetFirstTag("jid").GetBody());

		
	WokXMLTag &filelistitem = tag->AddTag("item");
	filelistitem.AddAttr("node", "p2p get_file_list");
	filelistitem.AddAttr("name", "P2P Get File List");
	filelistitem.AddAttr("jid", querytag.GetFirstTag("item").GetFirstTag("jid").GetBody());

			
	WokXMLTag &filedownloaditem = tag->AddTag("item");
	filedownloaditem.AddAttr("node", "p2p download");
	filedownloaditem.AddAttr("name", "P2P Download File");
	filedownloaditem.AddAttr("jid", querytag.GetFirstTag("item").GetFirstTag("jid").GetBody());
		
	return 1;	
}

int
FileShare::MenuDownload(WokXMLTag *tag)
{
	std::string name = tag->GetAttr("filename");
	if ( name.find("/") != std::string::npos )
	{
		name = name.substr(name.rfind("/")+1);
	}
	
	WokXMLTag msg(NULL, "message");
	msg.AddAttr("session", tag->GetAttr("session"));
	WokXMLTag &iq = msg.AddTag("iq");
	iq.AddAttr("to", tag->GetAttr("jid"));
	iq.AddAttr("type", "get");
	WokXMLTag &fileshare = iq.AddTag("fileshare");
	fileshare.AddAttr("xmlns", "http://sf.wokjab.net/fileshare");
	WokXMLTag &file = fileshare.AddTag("file");
	file.AddAttr("id", tag->GetAttr("file_id"));
	
	wls->SendSignal("Jabber XML IQ Send", msg);
	
	new File(wls, msg, name, config->GetFirstTag("download_path").GetAttr("data"));

	return 1;
}

int 
FileShare::sql_callback_search(FileShare *c, int argc, char **argv, char **azColName)
{
	int i;
	
	WokXMLTag &restag = c->search_result->AddTag("result");
	for(i=0; i<argc; i++)
		restag.AddAttr(azColName[i], argv[i] ? argv[i] : "NULL");
	return 0;
}

int 
FileShare::sql_callback(FileShare *c, int argc, char **argv, char **azColName)
{
  int i;
  for(i=0; i<argc; i++){
				if ( azColName[i] == std::string("id") )
					c->filetosend = argv[i] ? argv[i] : "NULL";
  }
  return 0;
}

int
FileShare::OpenSearchWid(WokXMLTag *tag)
{
	new SearchWid(wls);

	return 1;
}

int
FileShare::Config(WokXMLTag *tag)
{
	delete config;
	config = new WokXMLTag(tag->GetFirstTag("config"));
	sharepoints.clear();
	
	tag->GetFirstTag("config").GetFirstTag("download_path").AddAttr("type", "string");
	tag->GetFirstTag("config").GetFirstTag("download_path").AddAttr("label", "Path to download to");
	
	std::list <WokXMLTag*>::iterator iter;
	std::list <WokXMLTag*> *list = new std::list <WokXMLTag*> (tag->GetFirstTag("config").GetFirstTag("share_path").GetTagList("path"));
	for( iter = list->begin() ; iter != list->end() ; iter++)
	{
		if ( (*iter)->GetFirstTag("name").GetAttr("data") == "" )
		{
			tag->GetFirstTag("config").GetFirstTag("share_path").RemoveTag(*iter);
		}
		else
		{
			sharepoints[(*iter)->GetFirstTag("path").GetAttr("data")] = (*iter)->GetFirstTag("name").GetAttr("data");
		}
	}
	
	WokXMLTag &path = tag->GetFirstTag("config").GetFirstTag("share_path").AddTag("path");
	path.GetFirstTag("name").AddAttr("type", "string");
	path.GetFirstTag("name").AddAttr("label", "Mount point");
	path.GetFirstTag("path").AddAttr("type", "string");
	path.GetFirstTag("path").AddAttr("label", "Folder");
	
	path.AddAttr("label", "Path");

	
	return 1;
}

/**
 * Now how to select witch jid's to search thats a good question 
 */
int
FileShare::Search(WokXMLTag *xml)
{
	std::list <WokXMLTag *>::iterator sesiter;
	
	WokXMLTag sesquerytag(NULL,"session");
	wls->SendSignal("Jabber GetSessions", &sesquerytag);
	
	for( sesiter = sesquerytag.GetTagList("item").begin() ; sesiter != sesquerytag.GetTagList("item").end() ; sesiter++)
	{
		WokXMLTag group(NULL, "group");
		group.AddAttr("group", "p2p");
		group.AddAttr("session", (*sesiter)->GetAttr("name"));
		
		wls->SendSignal("Roster Get Members", group);
		std::list <WokXMLTag *>::iterator jiditer;
		for( jiditer = group.GetTagList("jid").begin() ; jiditer != group.GetTagList("jid").end() ; jiditer++)
		{	
			WokXMLTag res(NULL, "resource");
			WokXMLTag &itemtag = res.AddTag("item");
			itemtag.AddAttr("jid", (*jiditer)->GetBody());
			itemtag.AddAttr("session", "jabber0");
			
			wls->SendSignal("Jabber Roster GetResource", res);
			
			std::list <WokXMLTag *>::iterator resiter;
			for ( resiter = itemtag.GetTagList("resource").begin() ; resiter != itemtag.GetTagList("resource").end() ; resiter++ )
			{
				std::string jid = (*jiditer)->GetBody();
				if ( !(*resiter)->GetAttr("name").empty() )
					jid += "/" + (*resiter)->GetAttr("name");
			
				WokXMLTag msg(NULL, "message");
				msg.AddAttr("session", "jabber0");
				WokXMLTag &message = msg.AddTag("message");
				
				std::stringstream str;
				str << "FileShare_" << n_id;
				message.AddTag("thread").AddText(str.str());
				xml->AddAttr("thread", str.str());
				
				message.AddAttr("to", jid);
				WokXMLTag &x = message.AddTag("x");
				x.AddAttr("xmlns", "http://sf.wokjab.net/fileshare");
				
				std::list <WokXMLTag *>::iterator condition;
				for ( condition = xml->GetTagList("condition").begin() ; condition != xml->GetTagList("condition").end() ; condition++)
				{
					x.AddTag(*condition);				
				}
				
				wls->SendSignal("Jabber XML Send", msg);
			}
		}
	}
	return 1;
}

int
FileShare::MainMenu(WokXMLTag *xml)
{
	WokXMLTag *tag;
	tag = &xml->AddTag("item");
	tag->AddAttr("name", "Rebuild Filelist");
	tag->AddAttr("signal", "Jabber FileShare Rebuild");
	
	tag = &xml->AddTag("item");
	tag->AddAttr("name", "P2P Search");
	tag->AddAttr("signal", "Jabber FileShare SearchWid");
	
	return 1;
}

void
FileShare::PopulateTree(WokXMLTag *tag, std::string dir, std::string virt_dir)
{
	DIR             *dip;
	struct dirent   *dit;
	char *zErrMsg = 0;

	if ( g_utf8_validate(dir.c_str(), -1, NULL) == FALSE)
		return;
	
	if ((dip = opendir(dir.c_str())) == NULL)
	{
  		perror("opendir");
		return;
	}
	
	while ((dit = readdir(dip)) != NULL)
	{
		std::string file = dit->d_name;
		std::stringstream sizestr;
		if ( file[0] == '.' )
			continue;
		if ( g_utf8_validate(file.c_str(), -1, NULL) == FALSE)
			continue;
		
		struct	stat	sbuf;
		stat((dir + '/' + file).c_str(),&sbuf);

		if ( sbuf.st_mode & S_IFDIR )
		{
			WokXMLTag &item = tag->AddTag("item");
			item.AddAttr("type", "folder");
			item.AddAttr("name", file);
			sizestr << "0";
			PopulateTree(&item, dir + '/' + file, virt_dir+'/'+file);
		}
		else if ( sbuf.st_mode & S_IFREG )
		{
			sizestr << sbuf.st_size;

			WokXMLTag &item = tag->AddTag("item");
			item.AddAttr("id", dir + '/' + file);
			item.AddAttr("name", file);
			item.AddAttr("type", "file");
			item.AddAttr("size", sizestr.str());
		}
		
		std::string qdir = dir;
		std::string qfile = file;
		for ( unsigned int pos = 0 ; qdir.find("'", pos) != std::string::npos ; )
		{
			pos = qdir.find("'", pos );
			qdir.replace(pos, 1, "''");
			pos+=2;
			if ( pos >= qdir.size() )
				break;
		}
		
		for ( unsigned int pos = 0 ; qfile.find("'", pos) != std::string::npos ; )
		{
			pos = qfile.find("'", pos );
			qfile.replace(pos, 1, "''");
			pos+=2;
			
			if ( pos >= qfile.size() )
				break;
		}
		std::string query = "INSERT INTO filelist (id, file, path, size) values ('" + qdir + '/' + qfile + "', '" + qdir + '/' + qfile + "', '"+virt_dir + '/' + qfile +"','"+sizestr.str()+"');";
		int rc = sqlite3_exec(db, query.c_str(), (int(*)(void *,int,char**,char**)) FileShare::sql_callback, this, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
	}

	if (closedir(dip) == -1)
	{
		perror("closedir");
		return;
	}
}

WokXMLTag *
FileShare::AddFolder(WokXMLTag *point, std::string name)
{
	std::list <WokXMLTag *>::iterator titer;
	
	for ( titer = point->GetTagList("item").begin() ; titer != point->GetTagList("item").end() ; titer++)
	{
		if ( (*titer)->GetAttr("name") == name)
		{
			point = *titer;
			return point;
		}
	}
	
	point = &point->AddTag("item");
	point->AddAttr("name", name);
	point->AddAttr("type", "folder");
	
	return point;
}

int
FileShare::Rebuild(WokXMLTag *tag)
{
	char *zErrMsg = 0;

	std::string filename = path;

	std::string query = "DELETE FROM filelist;";
	int rc = sqlite3_exec(db, query.c_str(), (int(*)(void *,int,char**,char**)) FileShare::sql_callback, this, &zErrMsg);
	if( rc!=SQLITE_OK ){
			fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
	}
	
	
	
	WokXMLTag filelist (NULL, "filelist");
	std::map <std::string, std::string>::iterator iter;
	for ( iter = sharepoints.begin() ; iter != sharepoints.end() ; iter++)
	{
		WokXMLTag *point;
		point = &filelist;
		for(unsigned int pos = 0; pos < iter->first.size() ;)
		{
			if( iter->first.find("/", pos) != std::string::npos )
			{
				point = AddFolder(point, iter->first.substr(pos, iter->first.find("/", pos)));
				
				pos = iter->first.find("/", pos) + 1;
			}
			else
			{
				point = AddFolder(point, iter->first.substr(pos));
				break;
			}
		}
		PopulateTree(point, iter->second, "/" + iter->first);
	}
	

	gzFile gfile;
	gfile = gzopen ((std::string(g_get_home_dir()) + "/.wokjab/fileshare/filelist.xml.gz").c_str(), "wb");
	gzwrite(gfile, filelist.GetStr().c_str(), filelist.GetStr().size());
	gzclose(gfile);
	
	return 1;
}

bool
FileShare::HasPermission(std::string session, std::string jid)
{
	if ( jid.find("/") != std::string::npos )
		jid = jid.substr(0, jid.find("/"));
	
	WokXMLTag group(NULL, "group");
	group.AddAttr("group", "p2p");
	group.AddAttr("session", session);
	
	wls->SendSignal("Roster Get Members", group);
	std::list <WokXMLTag *>::iterator jiditer;
	for( jiditer = group.GetTagList("jid").begin() ; jiditer != group.GetTagList("jid").end() ; jiditer++)
	{	
		if ( jid == (*jiditer)->GetBody() )
			return true;
	}
		
		return false; 
}

int
FileShare::ListRequest(WokXMLTag *tag)
{
	if ( !HasPermission(tag->GetAttr("session"), tag->GetFirstTag("iq").GetAttr("from") ))
		return 1;
	
	std::list <WokXMLTag *>::iterator iter;
	for( iter = tag->GetFirstTag("iq").GetFirstTag("fileshare").GetTagList("filelist").begin() ; iter != tag->GetFirstTag("iq").GetFirstTag("fileshare").GetTagList("filelist").end() ; iter++)
	{	
		/* Sending the filelist */
		
		WokXMLTag msgtag(NULL, "message");
		msgtag.AddAttr("session", tag->GetAttr("session"));
		WokXMLTag &iqtag = msgtag.AddTag("iq");
		iqtag.AddAttr("to", tag->GetFirstTag("iq").GetAttr("from"));
		iqtag.AddAttr("type", "result");
		iqtag.AddAttr("id", tag->GetFirstTag("iq").GetAttr("id"));
		WokXMLTag &fileshare = iqtag.AddTag("fileshare");
		fileshare.AddAttr("xmlns", "http://sf.wokjab.net/fileshare");
		
		std::stringstream sid;
		sid << "fileshare-" << fileshareid++ << "-";
		for(int i = 0; i < 10; i++)
		{
				sid << (char) (rand() % 20 + 'a' );
		}
		fileshare.AddAttr("sid", sid.str() );
		
		wls->SendSignal("Jabber XML Send", msgtag);
		
		std::string filelistfile = (std::string(g_get_home_dir()) + "/.wokjab/fileshare/filelist.xml.gz").c_str();
		
		WokXMLTag send(NULL, "send");
		send.AddAttr("to", tag->GetFirstTag("iq").GetAttr("from"));
		send.AddAttr("session", tag->GetAttr("session"));
		send.AddAttr("name", filelistfile);
		send.AddAttr("sid", sid.str());
		send.AddAttr("proxy_type", "auto");
		wls->SendSignal("Jabber Stream File Send", send);
	}
	
	for( iter = tag->GetFirstTag("iq").GetFirstTag("fileshare").GetTagList("file").begin() ; iter != tag->GetFirstTag("iq").GetFirstTag("fileshare").GetTagList("file").end() ; iter++)
	{
		/* Sending a file */
		WokXMLTag msgtag(NULL, "message");
		msgtag.AddAttr("session", tag->GetAttr("session"));
		WokXMLTag &iqtag = msgtag.AddTag("iq");
		iqtag.AddAttr("to", tag->GetFirstTag("iq").GetAttr("from"));
		iqtag.AddAttr("type", "result");
		iqtag.AddAttr("id", tag->GetFirstTag("iq").GetAttr("id"));
		WokXMLTag &fileshare = iqtag.AddTag("fileshare");
		fileshare.AddAttr("xmlns", "http://sf.wokjab.net/fileshare");
		
		std::stringstream sid;
		sid << "fileshare-" << fileshareid++ << "-";
		for(int i = 0; i < 10; i++)
		{
				sid << (char) (rand() % 20 + 'a' );
		}
		fileshare.AddAttr("sid", sid.str() );
		
		wls->SendSignal("Jabber XML Send", msgtag);
		
		char *zErrMsg = 0;
		filetosend = "";
		
		std::string id = (*iter)->GetAttr("id");
		for ( unsigned int pos = 0 ; id.find("'", pos) != std::string::npos ; )
		{
			pos = id.find("'", pos );
			id.replace(pos, 1, "''");
			pos+=2;
			
			if ( pos >= id.size() )
				break;
		}
		
		filetosend = "";
		std::string query = "SELECT * FROM filelist WHERE id='" + id + "';";
		/* The callback is changing the filetosend variable */
		int rc = sqlite3_exec(db, query.c_str(), (int(*)(void *,int,char**,char**)) FileShare::sql_callback, this, &zErrMsg);

		if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		}
		if ( filetosend != "" )
		{
			WokXMLTag send(NULL, "send");
			send.AddAttr("to", tag->GetFirstTag("iq").GetAttr("from"));
			send.AddAttr("session", tag->GetAttr("session"));
			send.AddAttr("name", filetosend);
			send.AddAttr("sid", sid.str());
			send.AddAttr("proxy_type", "auto");
			wls->SendSignal("Jabber Stream File Send", send);
		}
//		EXP_SIGHOOK("Jabber Stream File Incomming " + sid.str(), &FileShare::IncommingFilelist, 1000);
	}
	
	return 1;
}

int
FileShare::IncommingSearch(WokXMLTag *tag)
{
	char *zErrMsg = 0;
	if ( tag->GetFirstTag("message").GetFirstTag("x").GetAttr("type") == "set" )
	{
		std::string thread = tag->GetFirstTag("message").GetFirstTag("thread").GetBody();
		wls->SendSignal("Jabber FileShare SearchResult " + thread, tag);
		
		return 1;
	}
	if ( tag->GetFirstTag("message").GetFirstTag("x").GetFirstTag("condition").GetBody().empty() )
		return 1;
		
	std::string query = "SELECT * FROM filelist WHERE id LIKE '%" + tag->GetFirstTag("message").GetFirstTag("x").GetFirstTag("condition").GetBody() + "%' LIMIT 10;";
	/* The callback is changing the filetosend variable */
	
	delete search_result;
	search_result = new WokXMLTag (NULL, "search");
	int rc = sqlite3_exec(db, query.c_str(), (int(*)(void *,int,char**,char**)) FileShare::sql_callback_search, this, &zErrMsg);
	
 	if( rc!=SQLITE_OK )
	{
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	
	std::list <WokXMLTag *>::iterator res_iter;
	
	WokXMLTag msg(NULL, "message");
	msg.AddAttr("session", tag->GetAttr("session"));
	WokXMLTag &message = msg.AddTag("message");
	message.AddAttr("to", tag->GetFirstTag("message").GetAttr("from"));
	if ( !tag->GetFirstTag("message").GetAttr("type").empty() )
		message.AddAttr("type", tag->GetFirstTag("message").GetAttr("type"));
	message.AddTag("thread").AddText(tag->GetFirstTag("message").GetFirstTag("thread").GetBody());
	WokXMLTag &x = message.AddTag("x");
	x.AddAttr("xmlns", "http://sf.wokjab.net/fileshare");
	x.AddAttr("type", "set");
	WokXMLTag &result = x.AddTag("result");
	int res_count=0;
	if ( search_result )
	{
		for( res_iter = search_result->GetTagList("result").begin() ; res_iter != search_result->GetTagList("result").end() ; res_iter++)
		{
			WokXMLTag &itemtag = result.AddTag("item");
			itemtag.AddAttr("id", (*res_iter)->GetAttr("id"));
			itemtag.AddAttr("name", (*res_iter)->GetAttr("path"));
			itemtag.AddAttr("size", (*res_iter)->GetAttr("size"));
			itemtag.AddAttr("hash", (*res_iter)->GetAttr("hash"));
			res_count++;
			/* 
				* Here there should be more fields
				*
				* path, hash, size
				*
				*/
		}
		
		if( res_count )
		{
			wls->SendSignal("Jabber XML Send", msg);		
		}
		
		delete search_result;
		search_result = NULL;
	}
	return 1;
}

int
FileShare::IncommingFile(WokXMLTag *tag)
{
	

	return 1;
}

int
FileShare::IncommingFilelist(WokXMLTag *tag)
{
	WokXMLTag msg(NULL, "message");
	msg.AddAttr("session", tag->GetAttr("session"));
	WokXMLTag &iq = msg.AddTag("iq");
	iq.AddAttr("id", tag->GetFirstTag("iq").GetAttr("id"));
	iq.AddAttr("to", tag->GetFirstTag("iq").GetAttr("from"));
	iq.AddAttr("type", "result");
	WokXMLTag &si = iq.AddTag("si");
	si.AddAttr("xmlns", "http://jabber.org/protocol/si");
	WokXMLTag &feature = si.AddTag("feature");
	feature.AddAttr("xmlns", "http://jabber.org/protocol/feature-neg");
	WokXMLTag &x = feature.AddTag("x");
	x.AddAttr("type", "submit");
	x.AddAttr("xmlns", "jabber:x:data");
	WokXMLTag &field = x.AddTag("field");
	field.AddAttr("var", "stream-method");
	WokXMLTag &value = field.AddTag("value");
	value.AddText("http://jabber.org/protocol/bytestreams");
	
	wls->SendSignal("Jabber XML Send", msg);
	new FileListWid(wls, tag, config);
	
	return 1;
}

int
FileShare::View(WokXMLTag *tag)
{
/*
	<iq to='apa@jabber.org' type='get'>
	<fileshare xmlns="http://sf.wokjab.net/fileshare" />
	</iq>

*/
	std::string mto = tag->GetAttr("jid");
	
	if( mto.find("/") == std::string::npos )
	{
		WokXMLTag restag(NULL, "query");
		WokXMLTag &itemtag = restag.AddTag("item");
		itemtag.AddAttr("jid", mto);
		itemtag.AddAttr("session", tag->GetAttr("session"));
		
		wls->SendSignal("Jabber Roster GetResource", &restag);
		mto = mto + "/" + itemtag.GetFirstTag("resource").GetAttr("name");		
	}

	WokXMLTag msgtag(NULL, "message");
	msgtag.AddAttr("session", tag->GetAttr("session"));
	WokXMLTag &iqtag = msgtag.AddTag("iq");
	iqtag.AddAttr("to", mto);
	iqtag.AddAttr("type", "get");
	WokXMLTag &fileshare = iqtag.AddTag("fileshare");
	fileshare.AddAttr("xmlns", "http://sf.wokjab.net/fileshare");
	fileshare.AddTag("filelist");
	
	
	wls->SendSignal("Jabber XML IQ Send", msgtag);
	EXP_SIGHOOK("Jabber XML IQ ID " + iqtag.GetAttr("id"), &FileShare::FileListResponse, 1000);
	
	return 1;
}

int
FileShare::FileListResponse(WokXMLTag *tag)
{
	EXP_SIGHOOK("Jabber Stream File Incomming " + tag->GetFirstTag("iq").GetFirstTag("fileshare").GetAttr("sid"), &FileShare::IncommingFilelist, 100);
	return 1;
}

int
FileShare::Menu(WokXMLTag *tag)
{
	WokXMLTag &item = tag->AddTag("item");
	item.AddAttr("name", "View File List");
	item.AddAttr("signal", "Jabber FileShare FileList View");
	
	return 1;
}
