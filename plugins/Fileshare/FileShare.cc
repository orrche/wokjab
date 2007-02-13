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
	
	EXP_SIGHOOK("Get Main Menu", &FileShare::MainMenu, 1000);
	EXP_SIGHOOK("Jabber FileShare Rebuild", &FileShare::Rebuild, 1000);
	fileshareid = 0;
	
	path = "/home/nedo/wokjabmess";
	
	int rc = sqlite3_open((std::string(g_get_home_dir()) + "/.wokjab/fileshare/filelist.db").c_str(), &db);
	if( rc ){
			fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
			sqlite3_close(db);
	}
}

FileShare::~FileShare()
{


}

int 
FileShare::sql_callback(FileShare *c, int argc, char **argv, char **azColName)
{
  int i;
  for(i=0; i<argc; i++){
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
				if ( azColName[i] == std::string("id") )
					c->filetosend = argv[i] ? argv[i] : "NULL";
  }
  printf("\n");
  return 0;
}

int
FileShare::MainMenu(WokXMLTag *xml)
{
	WokXMLTag *tag;
	tag = &xml->AddTag("item");
	tag->AddAttr("name", "Rebuild Filelist");
	tag->AddAttr("signal", "Jabber FileShare Rebuild");
	

	return 1;
}

int
FileShare::Rebuild(WokXMLTag *tag)
{
	DIR             *dip;
	struct dirent   *dit;
	char *zErrMsg = 0;

	std::string filename = path;

	if ((dip = opendir(filename.c_str())) == NULL)
	{
  perror("opendir");
		return 1;
	}
	std::string query = "DELETE FROM filelist;";
	int rc = sqlite3_exec(db, query.c_str(), (int(*)(void *,int,char**,char**)) FileShare::sql_callback, this, &zErrMsg);
	if( rc!=SQLITE_OK ){
			fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
	}
	WokXMLTag filelist (NULL, "filelist");
	
	
	while ((dit = readdir(dip)) != NULL)
	{
		std::string file = dit->d_name;
		
		if ( file[0] == '.' )
			continue;
			
		struct	stat	sbuf;
		stat((filename + '/' + file).c_str(),&sbuf);

		if ( sbuf.st_mode & S_IFDIR )
		{
			WokXMLTag &item = filelist.AddTag("item");
			item.AddAttr("type", "folder");
			item.AddAttr("name", file);
		}
		else if ( sbuf.st_mode & S_IFREG )
		{
			WokXMLTag &item = filelist.AddTag("item");
			item.AddAttr("id", filename + '/' + file);
			item.AddAttr("name", file);
			item.AddAttr("type", "file");
		}
		
		std::string query = "INSERT INTO filelist (id, file) values ('" + filename + '/' + file + "', '" + filename + '/' + file + "');";
		int rc = sqlite3_exec(db, query.c_str(), (int(*)(void *,int,char**,char**)) FileShare::sql_callback, this, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
	}

	if (closedir(dip) == -1)
	{
		perror("closedir");
		return 1;
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
		std::string query = "SELECT * FROM filelist WHERE id='" + (*iter)->GetAttr("id") + "';";
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
	new FileListWid(wls, tag);
	
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
	std::cout << "XML: " << tag << std::endl;
	
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
