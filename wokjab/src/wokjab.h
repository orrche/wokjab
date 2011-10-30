
#ifndef __WOKJAB_H_
#define __WOKJAB_H_


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


#include <Woklib/WokLib.h>
#include <Woklib/WLSignal.h>
#include <Woklib/WokLibSignal.h>

#include "../include/main.h"
#include "../include/Initiat.h"
#include "plugin-loader.hpp"

#include <iostream>
#include <string>

class Wokjab : WLSignalInstance
{
public:
	Wokjab(int argc, char **argv, std::string progname);
	~Wokjab();
	
	WLSignal *getWLS();
	void LoadPlugin(std::string filename);
	int pluginloader(WokXMLTag *tag);
	void load_plugin_list(GSList * plugins);
	
protected:
	WokLib wl;
	PluginLoader *pl;
	Initiat *client_init;	
};



#endif // __WOKJAB_H_
