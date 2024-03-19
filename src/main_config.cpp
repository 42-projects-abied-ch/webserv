
#include <iostream>
#include <vector>
#include "Director.hpp"
#include "Config.hpp"
#include "ConfigParser.hpp"
#include "ConfigDispatcher.hpp"

int main()
{
	try
	{
		Director director;
		ConfigDispatcher config(ConfigParser().get_config());
		Config conf(config.get_error_pages());
		std::map<int, std::map<std::string, std::vector<std::string> > > server_map = config.get_servers();
		conf.set_servers(server_map);
	}
	catch (const std::exception &e)
	{
		std::cerr << "error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return 0;
}
