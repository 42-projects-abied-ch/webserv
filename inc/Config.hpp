#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "LocationInfo.hpp"
#include "Server.hpp"
#include <map>
#include <netinet/in.h>
#include <ostream>
#include <string>
#include <vector>

class Server;


// Config class
//
// this class is responsible for the final configuration of the server
// 
// it uses the parsed configuration file to create the ServerInfo &
// LocationInfo objects, fill them, and to set the error pages
class Config
{
	public:

		typedef std::map <std::string, std::vector <std::string> > _map;
		typedef void (*location_setter)(const std::vector <std::string>&, LocationInfo*&);
		typedef std::map <std::string, location_setter> location_setter_map;
		typedef void (*server_setter)(const std::vector <std::string>&, Server*&);
		typedef std::map <std::string, server_setter> server_setter_map;

		std::string								get_error_page(const int key);
		std::vector <Server *>					get_servers() const;

		Config(const std::string& config_path="config_files/webserv.conf");
		~Config();
	
	private:

		void									set_servers(std::map <int, _map>& raw_servers);
		void									set_error_pages(const std::map <int, std::string>& error_pages);

		void									configure_port(_map& server, Server*& new_server, std::vector <std::string>& new_unique_values);
		void									configure_server_names(_map& server, Server*& new_server, std::vector <std::string>& new_unique_values);
		void									configure_host(_map& server, Server*& new_server, std::vector <std::string>& new_unique_values);
		location_setter_map::iterator			configure_cgi(std::string key, LocationInfo*& new_location);
		location_setter_map::iterator			initialize_location(const std::string& name, const std::string& key, LocationInfo*& new_location);
		void									configure_locations(const _map& server, Server*& new_server);

		void									initialize_location_setters();
		void									initialize_server_setters();

		std::vector <Server *>					_servers;
		std::map <int, std::string>				_error_pages;
		
		std::vector <LocationInfo *>			_locations;
		std::map <int, std::string>             _error_status_codes;
		std::vector <std::string>				_unique_values;

		location_setter_map						_location_setters;
		server_setter_map						_server_setters;

		Config(const Config& rhs);
		Config &operator=(const Config& rhs);
};

std::ostream& operator<<(std::ostream& out, const Config& config);

#define CLIENT_MAX_BODY_SIZE_DEFAULT 1000000
#define CLIENT_MAX_BODY_SIZE_MAX 10000000
#define MAX_URL_LENGTH 1024
#define MAX_HEADER_LENGTH 4096
#define ACCESS_LOG_DEFAULT "access.log"

#endif