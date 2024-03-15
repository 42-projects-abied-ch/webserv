#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "LocationInfo.hpp"
#include "Utils.hpp"
#include "ServerInfo.hpp"
#include <map>
#include "Log.hpp"
#include "Director.hpp"

class Config
{
	public:

		std::vector <ServerInfo *>				get_servers();
		std::string								get_error_page(const int key);
	


		void									set_servers(std::map <int, std::map <std::string, std::vector <std::string> > >& raw_servers);
		void									set_error_pages(std::map <int, std::string>& error_pages);
		void									set_routes(std::map <std::string, std::map <std::string, std::vector <std::string> > >& raw_routes);

		std::string								handle_access_log(const std::string& access_log);
		int										handle_client_max_body_size(const std::string& client_max_body_size);

		Config();
		~Config();
	
	private:

		std::vector <ServerInfo *>				_servers;
		std::map <int, std::string>				_error_pages;

		Config(const Config& rhs);
		Config &operator=(const Config& rhs);
};

#define CLIENT_MAX_BODY_SIZE_DEFAULT 1000000
#define ACCESS_LOG_DEFAULT "access.log"

#endif