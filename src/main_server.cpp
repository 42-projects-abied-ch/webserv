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
	// Server server;
	// server.set_port(8080);
	// server.set_type(SERVER_NODE);

	Director director(argv[1]);
//	director.add_server_info(server);
	if(director.init_servers() < 0)
	{
		std::cerr << "Error initializing servers." << std::endl;
		return (1);
	}
	if (director.run_servers() < 0)
	{
		std::cerr << "Error." << std::endl;
		return (1);
	}
	return 0;
}
