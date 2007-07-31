/***************************************************************************
 *  Copyright (C) 2003-2007  Kent Gustavsson <nedo80@gmail.com>
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
	
	int rc = sqlite3_open((std::string(g_get_home_dir()) + "/.wokjab/fileshare/filelist.db").c_str(), &db);
	if( rc ){
			fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
			sqlite3_close(db);
	}
}

FileShare::~FileShare()
{
	delete config;

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
	WokXMLTag group(NULL, "group");
	group.AddAttr("group", "p2p");
#warning Seriously bad !
	group.AddAttr("session", "jabber0");
	
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
			WokXMLTag &item = tag->AddTag("item");
			item.AddAttr("id", dir + '/' + file);
			item.AddAttr("name", file);
			item.AddAttr("type", "file");
			sizestr << sbuf.st_size;
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
				std::cout << "Query: " << query << std::endl;
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
		std::cout << ":" << iter->second << filelist << std::endl;
		PopulateTree(point, iter->second, "/" + iter->first);
	}
	

	gzFile gfile;
	gfile = gzopen ((std::string(g_get_home_dir()) + "/.wokjab/fileshare/filelist.xml.gz").c_str(), "wb");
	gzwrite(gfile, filelist.GetStr().c_str(), filelist.GetStr().size());
	gzclose(gfile);
	
	return 1;
}

int
FileShare::ListRequest(WokXMLTag *tag)
{
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
			std::cout << (*res_iter)->GetAttr("id") << std::endl;
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
