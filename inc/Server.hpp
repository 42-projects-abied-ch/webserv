#ifndef SERVER_HPP
#define SERVER_HPP

#include <ostream>
#include <string> 
#include <vector>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <dirent.h>
#include "Node.hpp"
#include "Request.hpp"
#include "LocationInfo.hpp"

class LocationInfo;
class Director;
class ClientInfo;

class Server : public Node
{
	public:

											Server();
											~Server();
											Server(const Server& rhs) : Node() { *this = rhs; }

		int									get_port() const { return _port; }
		void								set_port(int port) { _port = port; }

		std::vector <std::string>			get_server_name() const { return _server_name; }
		void								set_server_name(const std::vector <std::string>& tserver_name) { _server_name = tserver_name; }

		bool								get_auto_index() const { return _autoindex; }
		void								set_auto_index(bool autoindex) {_autoindex = autoindex; }

		std::string							get_root() const { return _root; }
		void								set_root(const std::string& root) { _root = root; }

		std::string							get_error_log() const { return _error_log; }
		void								set_error_log(const std::string& error_log) { _error_log = error_log; }

		std::string							get_access_log() const { return _access_log; }
		void								set_access_log(const std::string& access_log) { _access_log = access_log; }

		struct in_addr						get_host_address() const { return _host_address; }
		void								set_host_address(struct in_addr& host_address) { _host_address = host_address; }

		int									get_client_max_body_size() const { return _client_max_body_size; }
		void								set_client_max_body_size(const int client_max_body_size) { _client_max_body_size = client_max_body_size; }

		Director*							get_director() const { return _director; }
		void								set_director(Director *director) { _director = director; }

		std::vector <LocationInfo *>		get_locations() const { return _locations; }
		void								set_locations(std::vector <LocationInfo *> locations) { _locations = locations; }

		std::string							get_index_path() const { return _index; }
		void								set_index_path(const std::string& index) { _index = index; }

		int									get_error_code() const { return _errcode; }
		void								set_error_code(int errcode) { _errcode = errcode; }

		std::string							get_relocation() const { return _reloc; }
		void								set_relocation(const std::string& relocation) { _reloc = relocation; }

		std::string							get_error_page(const int status_code);
		void								add_error_page(const int status_code, const std::string& error_page_path) { _error_pages[status_code] = error_page_path; }

		std::string							respond(Request& rq);
		void								create_response(Request&, ClientInfo* client_info);
		void								reset();

	private:

		int									_port;
		int									_client_max_body_size;
		std::vector <std::string>			_server_name;
		struct in_addr						_host_address;
		bool								_autoindex;
		std::string							_index;
		std::string							_root;
		std::string							_error_log;
		std::string							_access_log;
		std::string							_reloc;
		std::map<int, std::string>			_error_pages;
		std::vector<LocationInfo*>			_locations;
		std::map<int, std::string>			_status_string;
		std::map<std::string, std::string>	_content_type;
		int									_errcode;
		Director*							_director;

		std::string							get_body(Request& rq, ClientInfo *ci);
		int									process(Request &rq, ClientInfo* ci, std::string& loc_path);
		bool								handle_empty_location_path(Request& request, const std::string& ret_file);
		bool								handle_directory_request(
														std::string& ret_file,
														const Request& request,
														const LocationInfo& location
													);

		void								get_best_location_match(
														std::vector<LocationInfo*> locs, 
														Request& rq,
														std::string& best_match, 
														LocationInfo* locinfo
													);

		int									get_directory_list(std::string &path, std::string& body);
		void								init_cgi(Request rq, LocationInfo loc);

		std::string							do_get(std::string& location_path);
		void								do_post(std::string& location_path, Request& request);
		void								do_delete(std::string& location_path, Request& request);
		bool								do_cgi(LocationInfo& location, Request& request, ClientInfo* client);

		bool								validate_cgi(const std::string& script_file_path);
		std::string							get_cgi_path(LocationInfo& location, Request& request, ClientInfo* client);

		bool								configure_max_body_size(LocationInfo& location, Request& request);
		bool								method_allowed(LocationInfo& location, Request& request);
		std::string							configure_file_path(LocationInfo& location, Request& request);

											Server(int tfd, struct sockaddr_storage ss, size_t addr_len);
		Server&								operator=(const Server&) { return *this; }
};

std::ostream& operator<<(std::ostream& os, const Server& server_info);

#endif
