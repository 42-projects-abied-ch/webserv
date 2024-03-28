#include "Server.hpp"
#include "LocationInfo.hpp"
#include <netinet/in.h>
#include <string>
#include <vector>
#include "Director.hpp"
#include <fstream>
#include "Log.hpp"

Server::Server() 
{
	_init_status_strings();
	_init_content_types();
	_autoindex = false;
	_errcode = 0;
	_autoindex = false;
	_index = "";
	_root = "";
	_port = 0;
	_client_max_body_size = 4098;
	_server_name = std::vector<std::string>();
	struct in_addr lo= {0};
	_host_address = lo;
	_error_log = "";
	_access_log = "";
	_error_pages = std::map<int, std::string>();
	_locations = std::vector<LocationInfo *>();
}

Server::~Server()
{
	std::vector<LocationInfo*>::iterator it;
	for (it = _locations.begin(); it != _locations.end(); it++)
	{
		delete *it;
	}
}


Server::Server(int tfd, struct sockaddr_storage ss, size_t taddr_len):
	Node(tfd, ss, taddr_len, SERVER_NODE)
{
	_init_status_strings();
	_init_content_types(); 
}

Server::Server(const Server& rhs) : Node()
{*this = rhs;}

Server&	Server::operator=(const Server& rhs)
{
	if (this != &rhs)
	{
		_port = rhs._port;
		_server_name = rhs._server_name;
		_autoindex = rhs._autoindex;
		_root = rhs._root;
		_error_log = rhs._error_log;
		_access_log = rhs._access_log;
		_locations = rhs._locations;
	}
	return (*this);
}

int	Server::get_port() const
{
	return (_port);
}

void	Server::set_port(int prt)
{
	_port = prt;
}

void	Server::set_server_name(const std::vector <std::string>& tserver_name)
{
	_server_name = tserver_name;
}

std::vector <std::string>	Server::get_server_name() const
{
	return _server_name;
}

bool	Server::get_auto_index() const
{
	return (_autoindex); 
}

void	Server::set_auto_index(bool t)
{
	_autoindex = t;
}

std::string	Server::get_error_log() const
{
	return (_error_log);
}

void	Server::set_error_log(const std::string& log)
{
	_error_log = log;
}

std::string	Server::get_access_log() const
{
	return (_access_log);
}

void	Server::set_access_log(const std::string& log)
{
	_access_log = log;
}

struct in_addr		Server::get_host_address() const
{
	return _host_address;
}

void	Server::set_host_address(struct in_addr& host)
{
	_host_address = host;
}

int	Server::get_client_max_body_size() const
{
	return _client_max_body_size;
}

void	Server::set_client_max_body_size(const int client_max_body_size)
{
	_client_max_body_size= client_max_body_size;
}

void	Server::set_index_path(const std::string& loc)
{
	_index = loc;
}

std::string	Server::get_index_path() const
{
	return _index;
}

void	Server::add_locations(std::vector <LocationInfo*> locations)
{
	this->_locations = locations;
}

std::string	Server::get_root() const
{
	return _root;
}

void	Server::set_root(const std::string& rt)
{
	_root = rt;
}

std::vector <LocationInfo*>	Server::get_locations() const
{
	return _locations;
}

std::string Server::respond(Request& rq)
{
	if (rq.get_method() == "GET" || rq.get_method() == "HEAD")
	{
		std::ifstream file("index.html");
		if (file.fail())
		{
			return "Error"; 
		}
		std::ostringstream ss;
		ss << file.rdbuf();
		return ss.str();
	}
	return "Error";
}

//Server

int		Server::get_error_code() const
{
	return _errcode;
}

void	Server::set_error_code(int cd)
{
	_errcode = cd;
}

void	Server::set_director(Director *d)
{
	_director = d;
}

Director*	Server::get_director() const
{
	return _director;
}


std::string Server::get_response() const
{
	return _response;
}

void	Server::set_response(const std::string& rs)
{
	_response = rs;
}

void	Server::_init_status_strings()
{
	_status_string[100] = "Continue";
	_status_string[101] = "Switching Protocol";
	_status_string[200] = "OK";
	_status_string[201] = "Created";
	_status_string[202] = "Accepted";
	_status_string[203] = "Non-Authoritative Information";
	_status_string[204] = "No Content";
	_status_string[205] = "Reset Content";
	_status_string[206] = "Partial Content";
	_status_string[300] = "Multiple Choice";
	_status_string[301] = "Moved Permanently";
	_status_string[302] = "Moved Temporarily";
	_status_string[303] = "See Other";
	_status_string[304] = "Not Modified";
	_status_string[307] = "Temporary Redirect";
	_status_string[308] = "Permanent Redirect";
	_status_string[400] = "Bad Request";
	_status_string[401] = "Unauthorized";
	_status_string[403] = "Forbidden";
	_status_string[404] = "Not Found";
	_status_string[405] = "Method Not Allowed";
	_status_string[406] = "Not Acceptable";
	_status_string[407] = "Proxy Authentication Required";
	_status_string[408] = "Request Timeout";
	_status_string[409] = "Conflict";
	_status_string[410] = "Gone";
	_status_string[411] = "Length Required";
	_status_string[412] = "Precondition Failed";
	_status_string[413] = "Payload Too Large";
	_status_string[414] = "URI Too Long";
	_status_string[415] = "Unsupported Media Type";
	_status_string[416] = "Requested Range Not Satisfiable";
	_status_string[417] = "Expectation Failed";
	_status_string[418] = "I'm a teapot";
	_status_string[421] = "Misdirected Request";
	_status_string[425] = "Too Early";
	_status_string[426] = "Upgrade Required";
	_status_string[428] = "Precondition Required";
	_status_string[429] = "Too Many Requests";
	_status_string[431] = "Request Header Fields Too Large";
	_status_string[451] = "Unavailable for Legal Reasons";
	_status_string[500] = "Internal Server Error";
	_status_string[501] = "Not Implemented";
	_status_string[502] = "Bad Gateway";
	_status_string[503] = "Service Unavailable";
	_status_string[504] = "Gateway Timeout";
	_status_string[505] = "HTTP Version Not Supported";
	_status_string[506] = "Variant Also Negotiates";
	_status_string[507] = "Insufficient Storage";
	_status_string[510] = "Not Extended";
	_status_string[511] = "Network Authentication Required";
}

void	Server::_init_content_types()
{
    _content_type["default"] = 	"text/html";
    _content_type[".html"] 	= 	"text/html";
    _content_type[".htm"] 	= 	"text/html";
    _content_type[".css"] 	= 	"text/css";
    _content_type[".txt"] 	= 	"text/plain";
    _content_type[".bmp"] 	= 	"image/bmp";
    _content_type[".gif"] 	= 	"image/gif";
    _content_type[".ico"] 	= 	"image/x-icon";
    _content_type[".ico"] 	= 	"image/x-icon";
    _content_type[".jpg"] 	= 	"image/jpeg";
    _content_type[".jpeg"]	= 	"image/jpeg";
    _content_type[".png"] 	= 	"image/png";
    _content_type[".pdf"] 	= 	"application/pdf";
    _content_type[".gz"] 	= 	"application/x-gzip";
    _content_type[".doc"] 	= 	"application/msword";
    _content_type[".avi"] 	= 	"video/x-msvideo";
    _content_type[".mp3"] 	= 	"audio/mp3";
}

void	Server::create_response(Request& rq)
{
	std::stringstream 	ss;
	std::string 		ex;
	char				buf[100];
	std::string			body;
	bool				failed = false;

	if ((_errcode = rq.get_errcode()) == 0)
	{
		try
		{
			body = _get_body(rq);
		}
		catch(const std::exception& e)
		{
			failed = true;
		}
	}
	else
	{
		failed = true;
	}
	
	if (failed)
	{
		std::ifstream error_file(_director->get_config()->get_error_page(_errcode).c_str());
		if (error_file.fail())
		{
			Log::log("Error reading error page file.\n", STD_ERR | ERROR_FILE);
			body = DEFAULT_ERROR_PAGE; //TODO
		}
		std::stringstream ss;
		ss << error_file.rdbuf();
		body = ss.str();
	}

	ss << "HTTP/1.1 " << _errcode << " " << _status_string[_errcode]  << "\r\n";

	time_t	curr_time = time(NULL);
	struct tm tim = *gmtime(&curr_time);
	strftime(buf, sizeof(buf), "%a, %d, %b %Y %H:%M:%S %Z", &tim);
	ss << "Date: " << buf << "\r\n";

	ss << "Server: Awesome SAD Server/1.0" << "\r\n";

	ss << "Content Length: " << body.length() << "\r\n";

	ex = Utils::get_file_extension(rq.get_path()); 
	if (_errcode != 200 || ex == "")
		ex = "default";
	ss << "Content-Type: " << _content_type[ex] << "\r\n";

	ss << "Connection: " << rq.get_header("CONNECTION") << "\r\n";
	ss << "\r\n";
	ss << body;
	_response = ss.str();
}

std::string		Server::_get_body(Request& rq)
{	
	std::string	loc_path;

	_errcode = _process(rq, loc_path);
	if (_errcode)
	{
		throw std::runtime_error("error");
	}
	if (rq.get_method() == "GET" || rq.get_method() == "HEAD")
	{
		std::ifstream file(loc_path.c_str());
		if (file.fail())
		{
			_errcode = 400;
			Log::log("Error reading request file", STD_ERR | ERROR_FILE); 
			throw std::runtime_error("error");
		}
		std::ostringstream ss;
		ss << file.rdbuf();
		return ss.str();
	}
	else if (rq.get_method() == "PUT" || rq.get_method() == "POST")
	{
		if (Utils::file_exists(loc_path) && rq.get_method() == "POST")
		{
			_errcode = 204;
			throw std::runtime_error("error");
		}
		std::ofstream file(loc_path.c_str(), std::ios::binary);
		if (file.fail())
		{
			_errcode = 404;
			Log::log("Error. Couldn't open location path.\n", STD_ERR | ERROR_FILE);
			throw std::runtime_error("error");
		}
		file.write(rq.get_body().c_str(), rq.get_body().size());
	}
	else if (rq.get_method() == "DELETE")
	{
		if (Utils::file_exists(loc_path) == false)
		{
			_errcode = 404;
			Log::log("Error. File to be deleted doesn't exist.\n", STD_ERR | ERROR_FILE);
			throw std::runtime_error("error");
		}
		if (remove(loc_path.c_str()) != 0)
		{
			_errcode = 500;
			Log::log("Error. Couldn't remove file to be removed.\n", STD_ERR | ERROR_FILE);
			throw std::runtime_error("error");
		}
	}
	_errcode = 200;
	return "";
}

int		Server::_process(Request& rq, std::string& ret_file)
{
	//std::string ret_file;
	LocationInfo loc_info;
	std::string loc_path;

	_get_best_location_match(_locations, rq, loc_path, &loc_info);
	if (!loc_path.empty())
	{
		// is body smaller than max?
		if ((int)rq.get_body().size() > loc_info.get_client_max_body_size())
		{
			Log::log("Error. Client body is too big.\n", STD_ERR | ERROR_FILE);
			return (_errcode = 413);
		}
		// is method allowed?
		std::vector<std::string> vec = loc_info.get_allowed_methods();
		std::vector<std::string>::iterator end = vec.end();
		std::vector<std::string>::iterator begin = vec.begin();
		if(std::find(begin,	end, rq.get_method()) != end)
		{
			Log::log("Error. Method not allowed.\n", STD_ERR | ERROR_FILE);
			return (_errcode = 405);
		}
		// return handler
		if (loc_info.get_return().empty() == false)
		{
			loc_path = loc_info.get_return();
			return (_errcode = 301);
		}
		// handle alias || create loc_path path
		if (loc_info.get_alias().empty() == false) 
			ret_file = Utils::pathconcat(loc_info.get_alias(), rq.get_path().substr(loc_info.get_path().size()));
		else
		{
			if (loc_info.get_root().empty())
				ret_file = Utils::pathconcat(get_root(), rq.get_path());
			else
				ret_file = Utils::pathconcat(loc_info.get_root(), rq.get_path());
		} 
		std::cout << ret_file << std::endl;
		std::cout << _errcode << std::endl;
		// handle cgi
		// if (loc_info.get_name().find("cgi-bin") != std::string::npos)
		// {
		// }


		// handle if loc_path is a directory 
		struct stat fst;
		if (stat(ret_file.c_str(), &fst) != 0)
		{
			_errcode = 400;
			Log::log("Stat function failed.\n", STD_ERR | ERROR_FILE);
			return (_errcode);
		}
		if (S_ISDIR(fst.st_mode))
		{
			if (ret_file[ret_file.size() -1 ] != '/')
			{
				loc_path = rq.get_path() + "/";
				return (_errcode = 301);
			}
			if (loc_info.get_index_path().empty() == false)
				ret_file += loc_info.get_index_path();
			else
				ret_file += get_index_path();
			if (Utils::file_exists(ret_file) == false)
			{
				if (loc_info.get_directory_listing())
				{
					ret_file.erase(ret_file.find_last_of('/') + 1);
					_autoindex = true;
					return 0;
				}
				else 
					return (_errcode = 403);
			}
			// if (stat(ret_file.c_str(), &fst) != 0)
			// {
			// 	_errcode = 400;
			// 	Log::log("Stat function failed.\n", STD_ERR | ERROR_FILE);
			// 	return (_errcode);
			// }
			// if (S_ISDIR(fst.st_mode))
			// {
			// 	_errcode = 301;
			// 	if (!loc_info.get_index_path().empty())

			// }
		}
	}
	else
	{
		loc_path = Utils::pathconcat(get_root(), rq.get_path());
		struct stat fst;
		if (stat(loc_path.c_str(), &fst) != 0)
		{
			_errcode = 400;
			Log::log("Stat function on failed.\n", STD_ERR | ERROR_FILE);
			return (_errcode);
		}
		if (S_ISDIR(fst.st_mode))
		{
			if (ret_file[ret_file.size() - 1] != '/')
			{
				_errcode = 301;
			}
		}
	}
	return 0;
}

void	Server::_get_best_location_match(std::vector<LocationInfo*> locs, 
										Request &rq,
										std::string& best_match,
										LocationInfo* locinfo)
{
	(void)locinfo;
	int max_len = 0;
	std::vector<LocationInfo*>::iterator e = locs.end();
	std::vector<LocationInfo*>::iterator it;
	for (it = locs.begin(); it != e; it++)
	{
		if(rq.get_path().find((*it)->get_path()) == 0)
		{
			if ((*it)->get_path() == "/" ||  
			(*it)->get_path().size() == rq.get_path().size()|| 
			rq.get_path()[(*it)->get_path().size()] == '/')
			{
				if ((int)(*it)->get_path().size() > max_len)
				{
					*locinfo = *(*it);
					best_match = (*it)->get_path();
					max_len = (*it)->get_path().size();
				}
			}
		}
	}
}

int	get_directory_list(std::string &path, std::string& body)
{
	std::stringstream ss;	

	DIR	*dir;
	dir = opendir(path.c_str());
	if (dir == NULL)
	{
		Log::log("Error oppening directory.\n", STD_ERR | ERROR_FILE);
		return -1;
	}
	ss << "<html>\n";
	ss << "<head>\n<title> Index of " << path <<"</title>\n</head>\n";
	ss << "<body>\n";
	ss << "<h1> Index of " + path + "</h1>\n";
	ss << "<table style=\"width:80%; font-size:15px\">\n";
	ss << "<hr>\n";
	ss << "<th style=\"text-align:left\"> File Name <\th>\n";
	ss << "<th style=\"text-align:left\"> Last Modification </th>\n";
	ss << "<th style=\"text-align:left\"> File Size </th>\n";

	struct stat fst;
	std::string f_path;

	struct dirent *dir_entry;
	while((dir_entry = readdir(dir)) != NULL)
	{
		if ((dir_entry->d_name[0] == '.') && (dir_entry->d_name[1] = '\0'))
			continue;
		f_path = path + dir_entry->d_name;
		stat(f_path.c_str(), &fst);
		ss << "<tr>\n<td>\n";
        ss << "<a href=\"" << dir_entry->d_name;
        if (S_ISDIR(fst.st_mode))
            ss << "/";
        ss << "\">";
        ss << dir_entry->d_name;
        if (S_ISDIR(fst.st_mode))
            ss << "/";
        ss << "</a>\n</td>\n";
        ss << "<td>\n" << ctime(&fst.st_mtime) << "</td>\n";
        ss << "<td>\n";
        if (!S_ISDIR(fst.st_mode))
            ss << Utils::itoa(fst.st_size);
        ss << "</td>\n";
		ss << "</tr>\n";
    }
    ss << "</table>\n";
    ss << "<hr>\n";
    ss << "</body>\n";
    ss << "</html>\n";
    body = ss.str();
    return (0);
}

void	Server::reset()
{
	_autoindex = false;
	_errcode = 0;
	_response.clear();
}
