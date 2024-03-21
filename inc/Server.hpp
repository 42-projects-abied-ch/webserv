#ifndef SERVER_HPP
#define SERVER_HPP

#include <sstream>
#include "Request.hpp"
//#include "Director.hpp"
//#include "ServerInfo.hpp"
//#include "ClientInfo.hpp"
#include "Utils.hpp"
#include "Log.hpp"

class Request;

class Server
{
	public:
											Server();
											~Server();
		Server&								operator=(const Server& rhs);
											Server(const Server& rhs);
		std::string							respond(Request& rq);

//		ServerInfo*							get_server_info() const;
		Request								get_request() const;
		int									get_error_code() const;
		void								set_error_code(int err);
		std::string							create_response(Request&);
		std::string							response;

	private:
		void								init_status_strings();
		void								init_content_types();
		std::string							get_body(Request& rq);

		std::map<int, std::string>			status_string;
		std::map<std::string, std::string>	content_type;

		// Request								request;
//		ServerInfo*							server_info;
	//	std::map<int, ClientInfo>			clients;
		int									errcode;
};

#endif