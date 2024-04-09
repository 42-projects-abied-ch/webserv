#ifndef CGI_HPP
#define CGI_HPP

#include <sched.h>
#include <string>
#include <map>
#include <vector>
#include "LocationInfo.hpp"
#include "Request.hpp"

class CGI
{
    public:

        std::string                             execute(std::vector <LocationInfo *> locations);
        void                                    initialize_environment_map(Request& request);
        void                                    clear();

        CGI(char** env=NULL);
        ~CGI();

    private:

        char**                                  _env;
        std::vector <LocationInfo*>             _locations;
        std::string                             _response_body;
        std::map <std::string, std::string>     _env_map;

        void                                    set_pipes(int request_fd[2], int response_fd[2]);
        void                                    delete_char_array(char** arguments);
        void                                    close_pipes(int count, ...);
        char**                                  set_arguments(const std::string& args, LocationInfo*& location);
        void                                    execute_script(int request_fd[2], int response_fd[2], char** arguments);
        void                                    parent(pid_t pid, int request_fd[2], int response_fd[2], char** arguments);
        LocationInfo*                           get_location(const std::string& script, const std::vector <LocationInfo *> locations);
};

#endif