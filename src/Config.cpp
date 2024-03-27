#include "Config.hpp"
#include "ConfigDispatcher.hpp"
#include "ConfigParser.hpp"
#include "LocationInfo.hpp"
#include "Server.hpp"
#include "Utils.hpp"
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
//			b. routes
//			c. error pages
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
	std::map <std::string, _map> routes = dispatcher.get_routes();

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
// else if: a unique value is already taken (host, port, server name)
//		->	log error & skip the server
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
			std::vector <std::string>	new_unique_values;

			configure_server_names(it->second, new_server, new_unique_values);
			configure_port(it->second, new_server, new_unique_values);
			configure_host(it->second, new_server, new_unique_values);
			configure_locations(it->second, new_server);

			for (_map::iterator itt = it->second.begin(); itt != it->second.end(); itt++)
			{
				if (_server_setters.find(itt->first) == _server_setters.end())
				{
					continue ;
				}
				(_server_setters[itt->first])(itt->second, new_server);
			}

			_servers.push_back(new_server);
			_unique_values.insert(_unique_values.end(), new_unique_values.begin(), new_unique_values.end());
		}
		catch (const std::exception& e)
		{
			Log::log(e.what(), STD_ERR | ERROR_FILE);
			delete new_server;
			continue ;
		}
	}
}

// extracts the location path from the current map key
//
// if:		"location" in key
//			->	return the path
//
// else:
//			->	return empty string


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
	std::string cgi_prefix = "location /cgi-bin";
	std::string cgi_name = key.substr(cgi_prefix.size() + 1);
	cgi_name =  "/" + cgi_name.substr(0, cgi_name.find_first_of(":"));

	if (new_location != NULL)
	{
		if (cgi_name != new_location->get_path())
		{
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

void	Config::configure_host(_map& server, Server*& new_server, std::vector <std::string>& new_unique_values)
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
	if (std::find(_unique_values.begin(), _unique_values.end(), host) != _unique_values.end())
	{
		throw std::runtime_error("error: '" + host + "': host address already taken, server will not be initialized\n");
	}
	else if (inet_pton(AF_INET, host.c_str(), &ip_address) != 1)
	{
		throw std::runtime_error("error: '" + host + "' is not a valid IPv4 address, server will not be initialized\n");
	}
	new_unique_values.push_back(host);

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
void	Config::configure_server_names(_map& server, Server*& new_server, std::vector <std::string>& new_unique_values)
{
	if (server.find("server_name") == server.end() or server["server_name"].empty() == true)
	{
		throw std::runtime_error("error: missing server_name, server will not be initialized\n");
	}

	std::vector <std::string> new_server_names = server["server_name"];

	for (std::vector <std::string>::const_iterator it = new_server_names.begin(); it != new_server_names.end(); it++)
	{
		if (std::find(_unique_values.begin(), _unique_values.end(), *it) != _unique_values.end())
		{
			throw std::runtime_error("error: on server: '" + *it + "': name already taken, server will not be initialized\n");
		}
	}

	new_server->set_server_name(new_server_names);

	new_unique_values.insert(new_unique_values.end(), new_server_names.begin(), new_server_names.end());
}

// finds the port in the server map and performs some error handling on it before storing it in the current Server
//
//	if:		no port in config
//			-> skip server in initialization
//
//	if:		port already given to another server
//			-> skip server in initialization
//
//	else:	
//			-> add port to vector of unique values
//			-> set port of current Server object
void	Config::configure_port(_map& server, Server*& new_server, std::vector <std::string>& new_unique_values)
{
	if (server.find("port") == server.end() or server["port"].empty() == true)
	{
		throw std::runtime_error("error: missing port in server '" + new_server->get_server_name()[0] + "', server will not be initialized\n");
	}

	std::string port = server["port"][0];

	if (std::find(_unique_values.begin(), _unique_values.end(), port) != _unique_values.end())
	{
		throw std::runtime_error("error: on server '" + new_server->get_server_name()[0] + "': port " + port + " already taken: '" + new_server->get_server_name()[0] + "' will not be initialized\n");
	}
	new_server->set_port(std::atoi(port.c_str()));

	new_unique_values.push_back(port);
}



std::vector <Server *>	Config::get_servers() const
{
	return _servers;
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
	_location_setters["handler"] = &Setters::set_cgi_path;
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

Config::Config(const Config& rhs) {*this = rhs;}

Config	&Config::operator=(const Config& rhs)
{
	if (this != &rhs)
	{
		_servers = rhs._servers;
		_error_pages = rhs._error_pages;
	}
	return *this;
}

std::ostream &operator<<(std::ostream &out, const Config &config)
{
	std::vector <Server *> servers = config.get_servers();
	for (std::vector <Server *>::iterator it = servers.begin(); it != servers.end(); it++)
	{
		std::cout << "host: " << inet_ntoa((*it)->get_host_address()) << std::endl;
		std::cout << "port: " << (*it)->get_port() << std::endl;
		std::cout << "server_name: " << (*it)->get_server_name()[0] << std::endl;
		std::cout << "root: " << (*it)->get_root() << std::endl;
		std::cout << "index: " << (*it)->get_index_path() << std::endl;
		std::cout << "autoindex: " << (*it)->get_auto_index() << std::endl;
		std::vector <std::string> server_name = (*it)->get_server_name();
		for (std::vector <std::string>::iterator it = server_name.begin(); it != server_name.end(); it++)
		{
			std::cout << *it << " ";
		}
		std::cout << std::endl;
		std::cout << "access_log: " << (*it)->get_access_log() << std::endl;
		std::cout << "client_max_body_size: " << (*it)->get_client_max_body_size() << std::endl;
		std::cout << "locations: " << std::endl;
		std::vector <LocationInfo *> locations = (*it)->get_locations();
		for (std::vector <LocationInfo *>::iterator it = locations.begin(); it != locations.end(); it++)
		{
			std::cout << "\tname: " << (*it)->get_path() << std::endl;
			std::cout << "\t\troot: " << (*it)->get_path() << std::endl;
			if ((*it)->directory_listing_enabled() == true)
			{
				std::cout << "\t\tdirectory listing: enabled" << std::endl;
			}
			else
			{
				std::cout << "\t\tdirectory listing: disabled" << std::endl;
			}
			if ((*it)->get_autoindex() == true)
			{
				std::cout << "\t\tautoindex: enabled" << std::endl;
			}
			else
			{
				std::cout << "\t\tautoindex: disabled" << std::endl;
			}
			std::cout << "\t\tallowed_methods: ";
			std::vector <std::string> allowed_methods = (*it)->get_allowed_methods();
			for (std::vector <std::string>::iterator it = allowed_methods.begin(); it != allowed_methods.end(); it++)
			{
				std::cout << *it << " ";
			}
			std::cout << std::endl;
			if ((*it)->get_cgi() == true)
			{
				std::cout << "\t\tcgi_path: " << (*it)->get_cgi_path() << std::endl;
				std::cout << "\t\tcgi_extension: " << (*it)->get_cgi_extensions()[0] << std::endl;
			}
		}
	}
	return out;
}