#include "ConfigSetters.hpp"
#include "Log.hpp"
#include "Config.hpp"
#include "Server.hpp"

namespace Setters
{
    template <typename T>
    void	set_string_value(const std::vector <std::string>& value, T* target, void (T::*setter)(const std::string&))
    {
        if (value.empty() == false)
        {
            (target->*setter)(value[0]);
        }
    }

    template <typename T>
    void    set_bool_value(const std::vector <std::string>& value, T* target, void (T::*setter)(bool))
    {
        if (value.empty() == false)
        {
            if (value[0] == "enabled")
            {
                (target->*setter)(true);
            }
            else if (value[0] == "disabled")
            {
                (target->*setter)(false);
            }
            else 
            {
                Log::log("error: invalid value for boolean config, use enabled/disabled\n", STD_ERR | ERROR_FILE);
            }
        }
    }

    template <typename T>
    void    set_vector_value(const std::vector <std::string>& value, T* target, void (T::*setter)(const std::vector<std::string>))
    {
        if (value.empty() == false)
        {
            (target->*setter)(value);
        }
    }

    template <typename T>
    void    set_string_check_path(const std::vector <std::string>& value, T* target, void (T::*setter)(const std::string&))
    {
        if (value.empty() == false)
        {
            if (Utils::file_exists(value[0]) == true)
            {
                (target->*setter)(value[0]);
            }
        }
    }

    void	set_root(const std::vector <std::string>& root, LocationInfo*& new_location)
    {
        set_string_value(root, new_location, &LocationInfo::set_root);
    }

    void	set_directory_listing(const std::vector <std::string>& directory_listing, LocationInfo*& new_location)
    {
        set_bool_value(directory_listing, new_location, &LocationInfo::set_directory_listing);
    }

    void	set_allowed_methods(const std::vector <std::string>& allowed_methods, LocationInfo*& new_location)
    {
        set_vector_value(allowed_methods, new_location, &LocationInfo::set_allowed_methods);
    }

    void	set_return(const std::vector <std::string>& rtrn, LocationInfo*& new_location)
    {
        set_string_value(rtrn, new_location, &LocationInfo::set_return);
    }

    void	set_alias(const std::vector <std::string>& alias, LocationInfo*& new_location)
    {
        set_string_value(alias, new_location, &LocationInfo::set_alias);
    }

    void	set_cgi_path(const std::vector <std::string>& cgi_path, LocationInfo*& new_location)
    {
        set_string_value(cgi_path, new_location, &LocationInfo::set_cgi_path);
    }

    void	set_cgi_extension(const std::vector <std::string>& extension, LocationInfo*& new_location)
    {
        set_vector_value(extension, new_location, &LocationInfo::set_cgi_extensions);
    }

    void	set_autoindex(const std::vector <std::string>& autoindex, LocationInfo*& new_location)
    {
        set_bool_value(autoindex, new_location, &LocationInfo::set_autoindex);
    }

    void	set_index(const std::vector <std::string>& index, LocationInfo*& new_location)
    {
        set_string_value(index, new_location, &LocationInfo::set_index_path);
    }

    void	configure_access_log(const std::vector <std::string>& access_log_vector, Server*& new_server)
    {
        set_string_check_path(access_log_vector, new_server, &Server::set_access_log);
    }

    void	configure_index(const std::vector <std::string>& index_vector, Server*& new_server)
    {
        set_string_value(index_vector, new_server, &Server::set_index_path);
    }

    void	configure_autoindex(const std::vector <std::string>& autoindex_vector, Server*& new_server)
    {
        set_bool_value(autoindex_vector, new_server, &Server::set_auto_index);
    }

    void	configure_root(const std::vector <std::string>& root_vector, Server*& new_server)
    {
        set_string_value(root_vector, new_server, &Server::set_root);
    }

    // validates client max body size from config
    //
    // if:	the value is missing or empty
    //		->	log error & fall back to default
    //
    // else if: the value is too high
    //		->	log error & cap it to 10M
    //
    // else if: the value is invalid
    //		->	log error & fall back to default
    void	configure_client_max_body_size(const std::vector <std::string>& body_size_vector, Server*& new_server)
    {
        if (body_size_vector.empty() == true)
        {
            Log::log("no client max body size config in server '" + new_server->get_server_name()[0] + "', falling back to default (1M)\n", STD_ERR | ERROR_FILE);
            new_server->set_client_max_body_size(CLIENT_MAX_BODY_SIZE_DEFAULT);
            return ;
        }

        std::string client_max_body_size = body_size_vector[0];
        int	size = Utils::parse_client_max_body_size(client_max_body_size);

        if (size > CLIENT_MAX_BODY_SIZE_MAX)
        {
            Log::log("error: " + client_max_body_size + ": client max body size too high, capping to 10M\n", STD_ERR | ERROR_FILE);
            size = CLIENT_MAX_BODY_SIZE_MAX;
        }
        else if (size < 0)
        {
            Log::log("error: client max body size '" + client_max_body_size + "' is not valid, falling back to default (1M)\n", STD_ERR | ERROR_FILE);
            size = CLIENT_MAX_BODY_SIZE_DEFAULT;
        }
        new_server->set_client_max_body_size(size);
    }
}