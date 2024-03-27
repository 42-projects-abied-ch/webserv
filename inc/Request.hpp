#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <map>
#include <sstream>
#include <algorithm>

#include "Utils.hpp"


class Request
{
	public:
        // constructors and destructors
		Request();
		~Request();
		Request(const Request& rhs);
		Request&		operator=(const Request& rhs);

        // init and clean request
        void                        init(std::string request);
        void                        clean(void);
        

        // setters and getters
        std::map <std::string, std::string> get_headers() const{ return this->headers;}
        std::string get_method() const{ return this->method;}
        std::string get_uri() const{ return this->uri;}
        std::string get_protocol() const{ return this->protocol;}
        std::string get_body() const{ return this->body;}
        std::string get_userinfo() const{ return this->userinfo;}
        std::string get_host() const{ return this->host;}
        std::string get_port() const{ return this->port;}
        std::string get_path() const{ return this->path;}
        std::string get_query() const{ return this->query;}
        std::string get_fragment() const{ return this->fragment;}
        int get_errcode() const{ return this->errcode;}
        std::string get_header(const std::string& key) const;


	private:

		std::string 					protocol;
        std::string 					method;
        std::string 					uri;
        std::map <std::string, std::string>             headers;
        std::string 					body;
        std::string 					userinfo;
        std::string 					host;
        std::string 					port;
        std::string 					path;
        std::string 					query;
        std::string 					fragment;
        int                             errcode;
		void                        parse(std::string request);
        void                        validate_request(void);
        void                        validate_uri(void);
        void                        pct_decode(void);
        void                        check_length(void);

};
std::ostream& operator<<(std::ostream& os, const Request& req);

#define BOLD "\033[1m"
#define RESET "\033[0m"
#define UNDERLINE "\033[4m"
#define RED "\033[31m"
#define GREEN "\033[32m"

#define ALPHA "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define DIGIT "0123456789"
#define HEXDIG "0123456789ABCDEFabcdef"
#define UNRESERVED ALPHA DIGIT "-._~"
#define SUBDELIMS "!$&'()*+,;="
#define PCHAR UNRESERVED SUBDELIMS ":@%"
#define TCHAR UNRESERVED SUBDELIMS "/?#"
#define USERINFO UNRESERVED SUBDELIMS ":%"
#define FRAGMENT PCHAR "/?%"
#define QUERY PCHAR "/?%" 
#define PATH PCHAR "/%"
#define HOST UNRESERVED SUBDELIMS "%:"



#endif
