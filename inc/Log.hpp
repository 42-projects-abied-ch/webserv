#ifndef LOG_HPP 
#define LOG_HPP

#include <iostream>
#include <fstream> 
#include <string>
#include <sstream>
#include <sys/time.h>
#include <map>

// #define RESET          "/033[0m"
// #define LIGHT_BLUE     "/033[94m"
// #define WHITE          "/033[37m"
// #define BLINK           "/033[5m"
// #define YELLOW         "/033[33m"
// #define CYAN           "/033[36m"
// #define LIGHT_RED      "/033[91m"
// #define RED            "/033[31m"
// #define DARK_GREY      "/033[90m"
// #define LIGHTMAGENTA   "/033[95m"

enum LogDest
{
	STD_OUT = 1,
	STD_ERR = 2,
	ERROR_FILE = 4,
	ACCEPT_FILE = 8
};
// purpose: basic Logging class that handles several output at one time
//			see comments on function log for use.

class Log
{
	public:
		static bool				log(const std::string& msg, int output = STD_OUT);
		static void				init_outfiles(const std::string& error_file, const std::string& accept_file);
		static std::string		get_error_file();
		static void				set_error_file(const std::string& ferror);
		static std::string		get_accept_file();
		static void				set_accept_file(const std::string& faccept);

	private:
								Log();
								~Log();
								Log(const Log& rhs);
		Log&					operator=(const Log& rhs);
		static std::string 		logmessage(const std::string& msg);
		static std::string		get_time_stamp();
		static std::ofstream	logfile;
		static std::string		error_file;
		static std::string		accept_file;
};

#endif

