#include "Config.hpp"
#include "ConfigDispatcher.hpp"
#include "ConfigParser.hpp"
#include "LocationInfo.hpp"
#include "ServerInfo.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <cstddef>
#include <exception>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include "Log.hpp"
#include <cstdlib>
#include <sys/socket.h>
#include <vector>
#include <arpa/inet.h>

void	Config::set_servers(std::map <int, std::map <std::string, std::vector <std::string> > >& raw_servers)
{
	ServerInfo* new_server;

	for (std::map <int, _map>::iterator it = raw_servers.begin(); it != raw_servers.end(); it++)
	{
		try
		{	
			new_server = new ServerInfo();
			std::vector <std::string>	new_unique_values;

			handle_server_names(it->second, new_server, new_unique_values);
			handle_port(it->second, new_server, new_unique_values);
			handle_access_log(it->second, new_server);
			handle_host(it->second, new_server, new_unique_values);
			handle_client_max_body_size(it->second, new_server);
			handle_locations(it->second, new_server);

			_servers.push_back(new_server);
			_unique_values.insert(_unique_values.end(), new_unique_values.begin(), new_unique_values.end());
		}
		catch (const std::exception& e)
		{
			delete new_server;

			Log::log(e.what(), STD_ERR | ERROR_FILE);

			continue ;
		}
	}
}

void	Config::handle_locations(_map& server, ServerInfo* new_server)
{
	std::vector <LocationInfo*> locations;
	std::string					location_key_prefix = "location";
	LocationInfo*				new_location = NULL;

	for (_map::iterator it = server.begin(); it != server.end(); it++)
	{
		size_t found = it->first.find(location_key_prefix);

		if (found != std::string::npos)
		{
			size_t	colon = it->first.find(":", found);
			std::string	path = it->first.substr(found + location_key_prefix.size(), colon - (found + location_key_prefix.size()));
			
			if (new_location == NULL || new_location->get_name() != path)
			{
				if (new_location != NULL)
				{
					locations.push_back(new_location);
				}
				new_location = new LocationInfo();
			}

			new_location->set_name(path);

			if (it->first.find("root") != std::string::npos && it->second.empty() == false)
			{
				new_location->setPath(it->second[0]);
			}
			else if (it->first.find("directory_listing") != std::string::npos && it->second.empty() == false)
			{
				if (it->second[0] == "enabled")
				{
					new_location->set_directory_listing(true);
				}
			}
			else if (it->first.find("allowed_methods") != std::string::npos)
			{
				new_location->set_allowed_methods(it->second);
			}
			else
			{
				Log::log("error: config value  at path " + it->first + "'" + it->second[0] + "' not recognized, will be ignored in server initialization\n");
			}
		}
	}
	if (new_location != NULL)
	{
		locations.push_back(new_location);
	}
	new_server->add_locations(locations);
}

void	Config::handle_host(_map& server, ServerInfo* new_server, std::vector <std::string>& new_unique_values)
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

void	Config::handle_server_names(_map& server, ServerInfo* new_server, std::vector <std::string>& new_unique_values)
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

void	Config::handle_port(_map& server, ServerInfo* new_server, std::vector <std::string>& new_unique_values)
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

// validates access log from config
//
// if:	the file does not exist (meaning we can just create it)
// or:	it exists and we have write access to it
//		->	config valid, return the filename
//
// else:
//		->	path invalid, log error & return default 
void	Config::handle_access_log(_map& server, ServerInfo* new_server)
{
	std::string access_log;

	if (server.find("access_log") != server.end() && server["access_log"].empty() == false)
	{
		access_log = server["access_log"][0];

		if (Utils::file_exists(access_log) == false or Utils::write_access(access_log) == true)
		{
			new_server->set_access_log(access_log);
			return ;
		}
	}
	Log::log("error: access log '" + access_log + "' could not be opened, falling back to 'access.log'\n", STD_ERR | ERROR_FILE);

	new_server->set_access_log(ACCESS_LOG_DEFAULT);
}

void	Config::handle_client_max_body_size(_map& server, ServerInfo* new_server)
{
	if (server.find("client_max_body_size") == server.end() or server["client_max_body_size"].empty() == true)
	{
		Log::log("no client max body size config in server '" + new_server->get_server_name()[0] + "', falling back to default (1M)\n", STD_ERR | ERROR_FILE);
		new_server->set_client_max_body_size(CLIENT_MAX_BODY_SIZE_DEFAULT);
		return ;
	}

	std::string client_max_body_size = server["client_max_body_size"][0];
	int	size = Utils::parse_client_max_body_size(client_max_body_size);

	if (size > CLIENT_MAX_BODY_SIZE_MAX)
	{
		Log::log("error: " + client_max_body_size + ": client max body size too high, capping to 10M\n", STD_ERR | ERROR_FILE);
		size = CLIENT_MAX_BODY_SIZE_MAX;
	}
	else if (size == -1)
	{
		Log::log("error: client max body size '" + client_max_body_size + "' is not valid, falling back to default (1M)\n", STD_ERR | ERROR_FILE);
		size = CLIENT_MAX_BODY_SIZE_DEFAULT;
	}
	new_server->set_client_max_body_size(size);
}

void	Config::set_routes(std::map <std::string, _map>& raw_routes)
{
	Route *new_route;
	for (std::map <std::string, _map>::iterator it = raw_routes.begin(); it != raw_routes.end(); it++)
	{
		new_route = new Route();

		std::string name = it->first;
		if (name == "/cgi-bin")
		{
			// TODO: handle_cgi();
		}
		else 
		{
			for (_map::iterator current_route = it->second.begin(); current_route != it->second.end(); current_route++)
			{
				new_route->set_name(name);
				if (current_route->first == "allowed_methods")
				{
					new_route->set_allowed_methods(current_route->second);
				}
				else if (current_route->first == "default_file" && current_route->second.empty() == false) 
				{
					new_route->set_default_file(current_route->second[0]);
				}
				else if (current_route->first == "root" && current_route->second.empty() == false)
				{
					new_route->set_root(current_route->second[0]);
				}
				else if (current_route->first == "upload_directory" && current_route->second.empty() == false)
				{
					new_route->set_upload_directory(current_route->second[0]);
				}
				else if (current_route->first == "http_redirect" && current_route->second.empty() == false)
				{
					if (current_route->second[0] == "enabled")
					{
						new_route->set_http_redirect(true);
					}
				}
				else if (current_route->first == "accept_file_upload" && current_route->second.empty() == false)
				{
					if (current_route->second[0] == "enabled")
					{
						new_route->set_accept_file_upload(true);
					}
				}
				else if (current_route->first == "directory_listing" && current_route->second.empty() == false)
				{
					if (current_route->second[0] == "enabled")
					{
						new_route->set_directory_listing(true);
					}
				}
				else
				{
					Log::log("error: config value '" + current_route->first + "' not recognized, will be ignored in route initialization\n", STD_ERR | ERROR_FILE);
				}
			}
		}
		_routes.push_back(new_route);
	}
}

std::vector <ServerInfo *>	Config::get_servers() const
{
	return _servers;
}

std::vector <Route *>	Config::get_routes() const
{
	return _routes;
}

std::string	Config::get_error_page(const int key)
{
	if (_error_pages.find(key) != _error_pages.end())
	{
		return _error_pages[key];
	}
	else 
	{
		return generate_default_error_page(key);
	}
}

std::string	Config::generate_default_error_page(const int status_code)
{
	std::string default_error_code = "400", default_error_message = "bad request", default_html = Utils::file_to_string(DEFAULT_ERROR_PAGE);
	std::string	new_error_code = Utils::itoa(status_code), new_error_message = _error_status_codes[status_code];

	size_t pos_code = default_html.find(default_error_code), pos_message = default_html.find(default_error_message);

	while (pos_code != std::string::npos || pos_message != std::string::npos)
	{
		if (pos_code != std::string::npos)
		{
			default_html.replace(pos_code, default_error_code.size(), new_error_code);
			pos_code = default_html.find(default_error_code, pos_code + new_error_code.size());
		}
		if (pos_message != std::string::npos)
		{
			default_html.replace(pos_message, default_error_message.size(), new_error_message);
			pos_message = default_html.find(default_error_message, pos_message + new_error_message.size());
		}
	}
	std::string new_html_path = "/tmp/" + Utils::itoa(status_code) + ".html";

	std::ofstream	oss(new_html_path.c_str());
	oss << default_html;
	return new_html_path;
}

Config::Config(const std::string& config_path)
{
	ConfigParser 		parser(config_path);
	ConfigDispatcher 	dispatcher(parser.get_config());

	std::map <int, _map> servers = dispatcher.get_servers();
	std::map <std::string, _map> routes = dispatcher.get_routes();

	_error_pages = dispatcher.get_error_pages();
	_error_status_codes = Utils::get_error_status_codes();
	set_routes(routes);
	set_servers(servers);
}

Config::~Config() 
{
	for (std::vector <ServerInfo *>::iterator it = _servers.begin(); it != _servers.end(); it++)
	{
		delete *it;
	}
	for (std::vector <Route *>::iterator it = _routes.begin(); it != _routes.end(); it++)
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