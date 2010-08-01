#include "wokjab.h"
#include <dirent.h>

Wokjab::Wokjab(int argc, char **argv, std::string progname)
: WLSignalInstance(&wl.wls_main)
{
	EXP_SIGHOOK("Wokjab Pluginloader load dir", &Wokjab::pluginloader, 1000);
	pl = NULL;
	client_init = new Initiat(wls, &wl, argc, argv);

	
	std::string configfile = std::string(g_get_home_dir()) + "/."+progname+"/config.xml";


	bool sysplug = true;
	bool normplug = true;


	if( argc > 1 )
	{
		for( int i = 1; i < (argc) ; ++i)
		{
			if ( std::string("-ns") == argv[i] )
				sysplug = false;
			if ( std::string("-nn") == argv[i] )
				normplug = false;
			if ( std::string("-c") == argv[i] && i+1 < argc )
				configfile = argv[i+1];
				
		}
	}
	

	if ( sysplug )
	{
		WokXMLTag path("path");
		path.AddAttr("path", std::string(PACKAGE_PLUGIN_DIR) + "/system");
		wls->SendSignal("Wokjab Pluginloader load dir", path);		
	}
	
	
	WokXMLTag confinit(NULL, "init");
	confinit.AddTag("filename").AddAttr("data", configfile);
	wls->SendSignal("Config XML Init", &confinit);
	
	
	if( argc > 1 )
	{
		for(int i = 1 ; i < (argc-1) ; ++i)
		{
			if( std::string("-p") == argv[i] )
			{
				std::string pluginstr = argv[i+1];
				std::string::size_type startpos= 0;
				std::string::size_type endpos;

				while((endpos = pluginstr.find(",", startpos)) != std::string::npos)
				{
					LoadPlugin(pluginstr.substr(startpos, endpos).c_str());
					startpos = endpos + 1;
				}
				endpos = pluginstr.length();
				LoadPlugin(pluginstr.substr(startpos, endpos).c_str());
			}
		}
	}
	
	if ( normplug )
		pl = new PluginLoader(wls);
	else
		pl = NULL;

	WokXMLTag start("start");
	wls->SendSignal("Program Start", start);
}

Wokjab::~Wokjab()
{
	if ( pl )
		delete pl;
	delete client_init;

	WokXMLTag saveconfig(NULL, "save");
	wls->SendSignal("Config XML Save", &saveconfig);
	WokXMLTag exit("exit");
	wls->SendSignal("Program Exit", exit);
}

WLSignal *
Wokjab::getWLS() 
{ 
	return wls; 
}



void 
Wokjab::LoadPlugin(std::string filename)
{
	std::cout << "Loading plugin " << filename << std::endl;
	
	WokXMLTag tag(NULL, "add");
	tag.AddAttr("filename", filename);
	wls->SendSignal("Woklib Plugin Add", &tag);
}

int
Wokjab::pluginloader(WokXMLTag *tag)
{
	std::string path = tag->GetAttr("path");
	std::cout << "Should be loading path " << path << std::endl;
	DIR             *dip;
	struct dirent   *dit;
    
	std::string filename = path;

	if ((dip = opendir(filename.c_str())) == NULL)
	{
    	perror("opendir");
		return 1;
	}

	while ((dit = readdir(dip)) != NULL)
	{
		std::string file = dit->d_name;

#ifdef __WIN32
		if(dit->d_name[0] != '.' && file.substr(file.size()-3) == "dll")
			LoadPlugin(filename + '/' + file);
#else
		if(dit->d_name[0] != '.' && file.substr(file.size()-2) == "so")
			LoadPlugin(filename + '/' + file);
#endif
	}

	if (closedir(dip) == -1)
	{
		perror("closedir");
		return 1;
	}
}

void
Wokjab::load_plugin_list(GSList * plugins)
{
	/* run through the list */
	GSList *listrunner;
	listrunner = plugins;

	while (listrunner)
	{
		LoadPlugin(((const gchar *)listrunner->data));
		listrunner = listrunner->next;
	}
}


