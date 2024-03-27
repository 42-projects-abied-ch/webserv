#include "Server.hpp"
#include "Director.hpp"

int main(int argc, char **argv)
{
	if (argc != 1 && argc != 2)
	{
		std::cerr << "Error. Invalid number of arguments." << std::endl;
		std::cerr << "Usage: " << argv[0] << " [config file <.conf>]" << std::endl;
		return 1;
	}
	else
	{
		std::string	config_file;
		if (argc == 2)
			config_file = argv[1];
		else
			config_file = "config_files/simple.conf";
		Director director(config_file);
		if(director.init_servers() < 0)
		{
			std::cerr << "Error initializing servers." << std::endl;
			return 1;
		}
		if (director.run_servers() < 0)
		{
			std::cerr << "Error." << std::endl;
			return 1;
		}
	}
	return 0;
}
