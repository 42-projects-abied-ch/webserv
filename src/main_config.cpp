
#include <iostream>
#include "Config.hpp"

int main(int argc, char **argv)
{
	try
	{
		Config conf;
		(void)argv;
		if (argc == 2)
		{
			Config conf(argv[1]);
		}
		std::cout << conf;
		// for (std::vector<Server *>::iterator it = servers.begin(); it != servers.end(); ++it)
		// {
		// 	std::cout << "Server port: " << (*it)->get_port() << std::endl;
		// 	std::vector<std::string> server_names = (*it)->get_server_name();
		// 	std::cout << "Server names: ";
		// 	for (std::vector<std::string>::iterator it = server_names.begin(); it != server_names.end(); ++it)
		// 	{
		// 		std::cout << *it << " ";
		// 	}
		// 	std::cout << std::endl;
		// 	std::cout << "Server root: " << (*it)->get_root() << std::endl;
		// 	std::cout << "Server error log: " << (*it)->get_error_log() << std::endl;
		// 	std::cout << "Server access log: " << (*it)->get_access_log() << std::endl;
		// 	std::cout << "Server host address: " << inet_ntoa((*it)->get_host_address()) << std::endl;
		// 	std::cout << "Server client max body size: " << (*it)->get_client_max_body_size() << std::endl;
		// 	std::cout << "Server index path: " << (*it)->get_index_path() << std::endl;
		// 	std::cout << "Server auto index: " << (*it)->get_auto_index() << std::endl;
		// 	std::cout << "Server locations: " << std::endl;
		// 	std::vector<LocationInfo *> locations = (*it)->get_locations();
		// 	for (std::vector<LocationInfo *>::iterator it = locations.begin(); it != locations.end(); ++it)
		// 	{
		// 		std::cout << "Location path: " << (*it)->get_path() << std::endl;
		// 		std::cout << "Location root: " << (*it)->get_root() << std::endl;
		// 		std::cout << "Location directory listing: " << (*it)->get_directory_listing() << std::endl;
		// 		std::cout << "Location allowed methods: ";
		// 		std::vector<std::string> allowed_methods = (*it)->get_allowed_methods();
		// 		for (std::vector<std::string>::iterator it = allowed_methods.begin(); it != allowed_methods.end(); ++it)
		// 		{
		// 			std::cout << *it << " ";
		// 		}
		// 		std::cout << std::endl;
		// 		std::cout << "Location return: " << (*it)->get_return() << std::endl;
		// 		std::cout << "Location alias: " << (*it)->get_alias() << std::endl;
		// 		std::cout << "Location cgi path: " << (*it)->get_cgi_path() << std::endl;
		// 		std::cout << "Location cgi extension: " << (*it)->get_cgi_extensions()[0] << std::endl;
		// 		std::cout << "Location auto index: " << (*it)->get_autoindex() << std::endl;
		// 		std::cout << "Location index: " << (*it)->get_index_path() << std::endl;
		// 	}
		// }
	}
	catch (const std::exception &e)
	{
		std::cerr << "error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
