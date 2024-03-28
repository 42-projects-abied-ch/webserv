#include "ClientInfo.hpp"

ClientInfo::ClientInfo()
{

}

ClientInfo::~ClientInfo()
{

}

ClientInfo::ClientInfo(int tfd, const struct sockaddr_storage& address, size_t taddrlen) :
	Node(tfd, address, taddrlen, CLIENT_NODE)
{
	type = CLIENT_NODE;
	prev_time = time(NULL);
}

ClientInfo::ClientInfo(const ClientInfo& rhs) : Node()
{
	(void)rhs;
}

ClientInfo&	ClientInfo::operator=(const ClientInfo& rhs)
{
	if (this != &rhs)
	{
		fd = rhs.fd;
		addr = rhs.addr;
	}
	return (*this);
}

time_t	ClientInfo::get_prev_time() const
{
	return prev_time;
}

void	ClientInfo::set_prev_time(time_t tm)
{
	prev_time = tm;
}

void	ClientInfo::set_time()
{
	prev_time = time(NULL);
}

void	ClientInfo::set_server(Server *si)
{
	server = si;
}

Server*		ClientInfo::get_server() const
{
	return server;
}


// void	ClientInfo::set_request(Request rq)
// {
// 	request = rq;
// }

// Request	ClientInfo::get_request() const
// {
// 	return request;
// }