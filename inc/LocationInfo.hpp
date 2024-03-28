#ifndef LOCATIONINFO_HPP
#define LOCATIONINFO_HPP

#include <string>
#include <vector>

class LocationInfo 
{
	public:
									LocationInfo();
									~LocationInfo();
									LocationInfo(const LocationInfo& rhs);
		LocationInfo&				operator=(const LocationInfo& rhs);
		bool						get_autoindex() const;
		void						set_autoindex(bool b);
		std::vector <std::string>	get_allowed_methods() const;
		void						set_allowed_methods(std::vector<std::string> allowed_methods);
		bool						directory_listing_enabled() const;
		void						set_directory_listing(bool directory_listing_enabled);
		bool						get_directory_listing() const;
		std::string					get_root() const;
		void						set_root(const std::string& r);
		std::string					get_path() const;
		void						set_path(const std::string& p);
		void						set_return(const std::string& r);
		std::string					get_return() const;
		void						set_alias(const std::string &a);
		std::string					get_alias() const;
		bool						get_cgi() const;
		void						set_cgi(bool cg);
		void						set_cgi_path(const std::string& p);
		std::string					get_cgi_path() const;
		void						set_cgi_extensions(std::vector<std::string> e);
		std::vector<std::string>	get_cgi_extensions() const;
		int							get_client_max_body_size() const;
		void						set_client_max_body_size(int n);
		std::string					get_index_path() const;
		void						set_index_path(const std::string& loc);
		
	private:
		std::string					_path;
		std::string					_root;
		std::string					_index;
		bool						_autoindex;
		std::string					_return;
		std::string					_alias;
		std::string					_cgi_path;
		std::vector<std::string>	_cgi_ext;
		std::vector <std::string>	_allowed_methods;
		bool						_directory_listing_enabled;
		int							_client_max_body_size;
		bool						_cgi;
};

std::ostream &operator<<(std::ostream& out, const LocationInfo& rhs);

#endif
