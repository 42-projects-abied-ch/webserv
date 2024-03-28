#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <string>
#include <unistd.h>
#include <map>

class Server;

namespace Utils 
{
	bool											write_access(const std::string& path);
	bool											file_exists(const std::string& path);
	void											get_pair(const std::string& str, std::vector <std::pair <std::string, int> >& ret, size_t left, size_t right, int& line_number);
	std::string										trim(const std::string& str, const std::string& delimiters);
	std::string										trim_comment(const std::string& str, const std::string& delimiters);
	std::string										itoa(int num);
	std::vector <std::string>						split(const std::string& str, const std::string& delimiters);
	std::vector <std::string>						split_keep_quoted_words(const std::string& str, const std::string& delimiters);
	std::vector <std::pair <std::string, int> > 	split_keep_delimiters(const std::string& str, const std::string& delimiters);
	std::string 									get_file_extension(const std::string& file_path);
	int												extract_numeric_value(const std::string& str);
	int												parse_client_max_body_size(const std::string& client_max_body_size);
	std::string										file_to_string(const std::string& path);
	std::map <int, std::string>						get_error_status_codes();
	std::string										pathconcat(std::string s1, std::string s2);
	std::string										generate_default_error_page(const int status_code);
	std::string										extract_location_name(const std::string& location);
	void											validate_required_server_values(Server* new_server);

};

#define UNCLOSED_QUOTE "unclosed quote sequence"
#define DEFAULT_ERROR_PAGE "<!DOCTYPE html>\n\
<html lang=\"en\">\n\
<head>\n\
<meta charset=\"UTF-8\">\n\
<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n\
<title>400</title>\n\
<link href=\"error.css\" rel=\"stylesheet\">\n\
</head>\n\
<body>\n\
<div class=\"container bsod\">\n\
  <div class=\"neg\">\n\
    <h1 class=\"bg title\">error 400</h1>\n\
    <p>bad request</p>\n\
  </div>\n\
  <div class=\"nav\">\n\
    <a href=\"/\" class=\"link\">go home</a>\n\
    <a href=\"/contact\" class=\"link\">contact us</a>\n\
  </div>\n\
</div>\n\
</body>\n\
</html>"

#endif