#ifndef CONFIGSETTERS_HPP
#define CONFIGSETTERS_HPP

#include "LocationInfo.hpp"
#include "Server.hpp"
#include "Config.hpp"

namespace Setters
{
    void	                                set_root(const std::vector <std::string>& root, LocationInfo*& new_location);
    void                                    set_directory_listing(const std::vector<std::string> &upload_path, LocationInfo *&new_location);
    void	                                set_allowed_methods(const std::vector <std::string>& methods, LocationInfo*& new_location);
    void                                    set_return(const std::vector<std::string> &upload_path, LocationInfo *&new_location);
    void                                    set_alias(const std::vector<std::string> &alias, LocationInfo *&new_location);
    void	                                set_cgi_path(const std::vector <std::string>& cgi_path, LocationInfo*& new_location);
    void	                                set_cgi_extension(const std::vector <std::string>& cgi_extension, LocationInfo*& new_location);
    void	                                set_autoindex(const std::vector <std::string>& autoindex, LocationInfo*& new_location);
    void	                                set_index(const std::vector <std::string>& index, LocationInfo*& new_location);
    void	                                configure_access_log(const std::vector <std::string>& server, Server*& new_server);
    void	                                configure_index(const std::vector <std::string>& server, Server*& new_server);
    void	                                configure_autoindex(const std::vector <std::string>& server, Server*& new_server);
    void	                                configure_root(const std::vector <std::string>& server, Server*& new_server);
    void	                                configure_client_max_body_size(const std::vector <std::string>& server, Server*& new_server);
}

#endif