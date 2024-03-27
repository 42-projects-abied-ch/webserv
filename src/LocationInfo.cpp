#include "LocationInfo.hpp"
#include <ostream>
#include <string>
#include <vector>

LocationInfo::LocationInfo() 
{
	_path = "";
	_autoindex = false; 
	_root = "";
	_index = "";
	_return = "";
	_alias = "";
	_cgi_path = "";
	_cgi_ext = std::vector<std::string>();
	_allowed_methods = std::vector<std::string>(); 
	_directory_listing_enabled = false;
	_client_max_body_size = 4098;
	_cgi = false;
}

LocationInfo::~LocationInfo()
{

}

LocationInfo::LocationInfo(const LocationInfo& rhs)
{
	*this = rhs;
}

bool	LocationInfo::get_cgi() const
{
	return _cgi;
}

void	LocationInfo::set_cgi(bool cg)
{
	_cgi = cg;
}

LocationInfo&	LocationInfo::operator=(const LocationInfo& rhs)
{
	if (this != &rhs)
	{
		_path = rhs._path;
		_autoindex = rhs._autoindex;
		_root = rhs._root;
		_index = rhs._index;
		_return = rhs._return;
		_alias = rhs._alias;
		_cgi_path = rhs._cgi_path;
		_cgi_ext = rhs._cgi_ext;
		_allowed_methods = rhs._allowed_methods;
		_directory_listing_enabled = rhs._directory_listing_enabled;
		_client_max_body_size = rhs._client_max_body_size;
	}
	return (*this);
}

std::string	LocationInfo::get_root() const
{
	return _root;
}

void	LocationInfo::set_root(const std::string& rt)
{
	_root = rt; 
}

bool	LocationInfo::get_autoindex() const
{
	return (_autoindex);
}

void	LocationInfo::set_autoindex(bool val)
{
	_autoindex = val;
}

std::vector <std::string>	LocationInfo::get_allowed_methods() const
{
	return _allowed_methods;
}

void	LocationInfo::set_allowed_methods(std::vector <std::string> allowed_methods)
{
	_allowed_methods = allowed_methods;
}

bool	LocationInfo::directory_listing_enabled() const
{
	return _directory_listing_enabled;
}

void	LocationInfo::set_directory_listing(bool directory_listing_enabled)
{
	_directory_listing_enabled = directory_listing_enabled;
}

bool	LocationInfo::get_directory_listing() const
{
	return _directory_listing_enabled;
}

std::string LocationInfo::get_return() const
{
	return _return;
}

void	LocationInfo::set_return(const std::string& ret)
{
	_return = ret;
}

std::string	LocationInfo::get_alias() const
{
	return _alias;
}

void	LocationInfo::set_alias(const std::string &a)
{
	if (a.empty() == false)
	{
		_alias = a[0];
	}
}

void	LocationInfo::set_cgi_path(const std::string& p)
{
	_cgi_path = p;
}

std::string	LocationInfo::get_cgi_path() const
{
	return _cgi_path;
}

void	LocationInfo::set_cgi_extensions(std::vector<std::string> e)
{
	_cgi_ext = e;
}

std::vector<std::string>	LocationInfo::get_cgi_extensions() const
{
	return _cgi_ext;
}

int	LocationInfo::get_client_max_body_size() const
{
	return _client_max_body_size;
}

void			LocationInfo::set_index_path(const std::string& loc)
{
	_index = loc;
}

std::string		LocationInfo::get_index_path() const
{
	return _index;
}

void	LocationInfo::set_client_max_body_size(int n)
{
	_client_max_body_size = n;
}

std::string	LocationInfo::get_path() const
{
	return _path;
}

void	LocationInfo::set_path(const std::string& path)
{
	_path = path;
}

std::ostream& operator<<(std::ostream& os, const LocationInfo& rhs)
{
	os << "\troot: " << rhs.get_path() << std::endl;
	os << "\tname: " << rhs.get_path() << std::endl;
	if (rhs.directory_listing_enabled() == true)
	{
		os << "\tdirectory listing: enabled" << std::endl;
	}
	else
	{
		os << "\tdirectory listing: disabled" << std::endl;
	}
	if (rhs.get_autoindex() == true)
	{
		os << "\tautoindex: enabled" << std::endl;
	}
	else
	{
		os << "\tautoindex: disabled" << std::endl;
	}
	os << "\tallowed methods: ";
	for (size_t i = 0; i < rhs.get_allowed_methods().size(); i++)
	{
		os << "\t" << rhs.get_allowed_methods()[i] << " ";
	}
	return os;
}