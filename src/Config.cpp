#include "Config.hpp"
#include "ConfigDispatcher.hpp"
#include "ConfigParser.hpp"
#include "LocationInfo.hpp"
#include "Server.hpp"
#include "Utils.hpp"
#include "ConfigSetters.hpp"
#include <algorithm>
#include <cstddef>
#include <exception>
#include <iostream>
#include <netinet/in.h>
#include <ostream>
#include <stdexcept>
#include <string>
#include "Log.hpp"
#include <cstdlib>
#include <sys/socket.h>
#include <vector>
#include <arpa/inet.h>
#include "ConfigSetters.hpp"

// full initialization of the config in one constructor
// 
// HOW TO INITIALIZE:
//		Config config(optional: std::string config_path);
//		
//		@param path:	path to .conf file - if no path is specified, will use: "config_files/webserv.conf"
//
// ConfigParser:
//		1. 	reads the whole config
//		2. 	performs error handling on file structure
//		3. 	stores config in a one level map with the config path as key
//
// ConfigDispatcher:
//		1. 	splits the config into logical parts
//			a. servers
//			b. error pages
//
// Config:
// 		1. 	gets error pages (already fully parsed in ConfigDispatcher 
//			since they are simple top level key-value pairs)
//		2.	performs final parsing and validation on routes & servers
//		3.	loads into Server, LocationInfo & Route
//			objects for server initialization
Config::Config(const std::string& config_path)
{
	ConfigParser 		parser(config_path);
	ConfigDispatcher 	dispatcher(parser.get_config());

	std::map <int, _map> servers = dispatcher.get_servers();

	_error_pages = dispatcher.get_error_pages();
	_error_status_codes = Utils::get_error_status_codes();
	initialize_location_setters();
	initialize_server_setters();
	set_servers(servers);
}

// takes the parsed servers from the config dispatcher and initializes the servers one by one
//
// if:	any of the required fields are missing
//		->	log error & fall back to default values
//
// else:
//		->	initialize the server
//		->	add the server to the list of servers
//		->	add the unique values to the list of unique values
void	Config::set_servers(std::map <int, std::map <std::string, std::vector <std::string> > >& raw_servers)
{
	Server* new_server;

	for (std::map <int, _map>::iterator it = raw_servers.begin(); it != raw_servers.end(); it++)
	{
		try
		{	
			new_server = new Server;

			configure_server_names(it->second, new_server);
			configure_port(it->second, new_server);
			configure_host(it->second, new_server);

			Utils::validate_required_server_values(new_server);

			configure_locations(it->second, new_server);

			for (_map::iterator map_it = it->second.begin(); map_it != it->second.end(); map_it++)
			{
				if (_server_setters.find(map_it->first) == _server_setters.end())
				{
					continue ;
				}
				(_server_setters[map_it->first])(map_it->second, new_server);
			}

			_servers.push_back(new_server);
		}
		catch (const std::exception& e)
		{
			Log::log(e.what(), STD_ERR | ERROR_FILE);
			delete new_server;
			continue ;
		}
	}
}

// initializes the location before setting the values
//
// if:	the new_location is NULL or the name of the current map key is different from the new_location name
//		->	initialize a new location
//
//	->	return the setter for the current map key
Config::location_setter_map::iterator	Config::initialize_location(const std::string& name, const std::string& key, LocationInfo*& new_location)
{
	if (new_location == NULL || name != new_location->get_path())
	{
		if (name != "/cgi-bin")
		{
			if (new_location != NULL)
			{
				_locations.push_back(new_location);
			}
			new_location = new LocationInfo;
			new_location->set_path(name);
		}
		else 
		{
			return configure_cgi(key, new_location);
		}
	}
	std::string current_key = key.substr(key.find_first_of(":") + 1);
	location_setter_map::iterator setter = _location_setters.find(current_key);

	return setter;
}

// initializes the locations for a server
//
// if:		current server key contains the string "location":
//			-> look for the value in the location setter function map
//			if found:
//				-> add/update new location
//			else:
//				-> invalid config setting -> log error and continue
void	Config::configure_locations(const _map& server, Server*& new_server)
{
	LocationInfo*					new_location = NULL;

	for (_map::const_iterator it = server.begin(); it != server.end(); it++)
	{

		std::string name = Utils::extract_location_name(it->first);

		if (name.empty() == true)
		{
			continue ;
		}

		location_setter_map::iterator setter = initialize_location(name, it->first, new_location);
		
		if (setter != _location_setters.end())
		{
			try 
			{
				(setter->second)(it->second, new_location);
			}
			catch (const std::exception& e)
			{
				Log::log(e.what(), STD_ERR | ERROR_FILE);
			}
		}
		else
		{
			Log::log("error: '" + it->first + "' is not a valid location setting\n", STD_ERR | ERROR_FILE);
		}
	}
	if (new_location != NULL)
	{
		if (new_location->get_cgi() == true)
		{
			new_location->set_path("/cgi-bin");
		}
		_locations.push_back(new_location);
	}
	new_server->add_locations(_locations);
	_locations.clear();
}

// initializes the CGI before setting the values
//
// if:	the new_cgi is NULL or the name of the current map key is different from the new_cgi name
//		->	initialize a new CGI
//
//	->	return the setter for the current map key
Config::location_setter_map::iterator	Config::configure_cgi(std::string key, LocationInfo*& new_location)
{
	if (std::count(key.begin(), key.end(), ':') < 2)
	{
		Log::log("error: no identifier found for cgi block\n", STD_ERR | ERROR_FILE);
		return _location_setters.end();
	}
	std::string cgi_prefix = "location /cgi-bin";
	std::string cgi_name = key.substr(cgi_prefix.size() + 1);
	cgi_name =  "/" + cgi_name.substr(0, cgi_name.find_first_of(":"));

	if (new_location != NULL)
	{
		if (cgi_name != new_location->get_path())
		{
			if (new_location->get_cgi() == true)
			{
				new_location->set_path("/cgi-bin");
			}
			_locations.push_back(new_location);
			new_location = new LocationInfo;
		}
	}
	else
	{
		new_location = new LocationInfo;
	}
	new_location->set_path(cgi_name);
	new_location->set_cgi(true);

	std::string current_key = key.substr(key.find_last_of(":") + 1);
	
	return _location_setters.find(current_key);
}

void	Config::configure_host(_map& server, Server*& new_server)
{
	if (server.find("host") == server.end() or server["host"].empty() == true)
	{
		throw std::runtime_error("error: missing host in server '" + new_server->get_server_name()[0] + "', server will not be initialized\n");
	}

	std::string host = server["host"][0];

	struct in_addr	ip_address;

	if (host == "localhost")
	{
		host = "127.0.0.1";
	}
	
	if (inet_pton(AF_INET, host.c_str(), &ip_address) != 1)
	{
		throw std::runtime_error("error: '" + host + "' is not a valid IPv4 address, server will not be initialized\n");
	}

	new_server->set_host_address(ip_address);
}

// finds the server_name(s) in the server map and performs some error handling on them 
// before storing them in the current Server
//
//	if:		no server name in config
//			-> skip server in initialization
//
//	if:		server name already given to another server
//			-> skip server in initialization
//
//	else:	
//			-> add server name(s) to vector of unique values
//			-> set server name(s) of current Server object
void	Config::configure_server_names(_map& server, Server*& new_server)
{
	if (server.find("server_name") == server.end() or server["server_name"].empty() == true)
	{
		throw std::runtime_error("error: missing server_name, server will not be initialized\n");
	}

	std::vector <std::string> new_server_names = server["server_name"];

	for (std::vector <std::string>::const_iterator it = new_server_names.begin(); it != new_server_names.end(); it++)
	{
		if (std::find(_server_names.begin(), _server_names.end(), *it) != _server_names.end())
		{
			throw std::runtime_error("error: on server: '" + *it + "': name already taken, server will not be initialized\n");
		}
	}

	_server_names.insert(_server_names.begin(), new_server_names.begin(), new_server_names.end());

	new_server->set_server_name(new_server_names);

}

// finds the port in the server map and performs some error handling on it before storing it in the current Server
//
//	if:		no port in config
//			-> skip server in initialization
//
//	else:	
//			-> set port of current Server object
void	Config::configure_port(_map& server, Server*& new_server)
{
	if (server.find("port") == server.end() or server["port"].empty() == true)
	{
		throw std::runtime_error("error: missing port in server '" + new_server->get_server_name()[0] + "', server will not be initialized\n");
	}

	std::string port = server["port"][0];

	new_server->set_port(std::atoi(port.c_str()));
}

std::string	Config::get_error_page(const int key)
{
	if (_error_pages.find(key) != _error_pages.end())
	{
		return _error_pages[key];
	}
	else 
	{
		return Utils::generate_default_error_page(key);
	}
}

void	Config::initialize_server_setters()
{
	_server_setters["access_log"] = &Setters::configure_access_log;
	_server_setters["client_max_body_size"] = &Setters::configure_client_max_body_size;
	_server_setters["autoindex"] = &Setters::configure_autoindex;
	_server_setters["root"] = &Setters::configure_root;
	_server_setters["index"] = &Setters::configure_index;
}

void	Config::initialize_location_setters()
{
	_location_setters["root"] = &Setters::set_root;
	_location_setters["directory_listing"] = &Setters::set_directory_listing;
	_location_setters["allowed_methods"] = &Setters::set_allowed_methods;
	_location_setters["return"] = &Setters::set_return;
	_location_setters["alias"] = &Setters::set_alias;
	_location_setters["handler"] = &Setters::set_cgi_handler;
	_location_setters["extension"] = &Setters::set_cgi_extension;
	_location_setters["autoindex"] = &Setters::set_autoindex;
	_location_setters["index"] = &Setters::set_index;
}

Config::~Config() 
{
	for (std::vector <Server *>::iterator it = _servers.begin(); it != _servers.end(); it++)
	{
		delete *it;
	}
}

std::ostream &operator<<(std::ostream &out, const Config &config)
{
	std::vector <Server *> servers = config.get_servers();
	for (std::vector <Server *>::iterator server = servers.begin(); server != servers.end(); server++)
	{
		Utils::print_server_info(out, *server);
		std::vector <LocationInfo *> locations = (*server)->get_locations();
		for (std::vector <LocationInfo *>::iterator location = locations.begin(); location != locations.end(); location++)
		{
			Utils::print_location_info(out, *location);
		}
	}
	return out;
}