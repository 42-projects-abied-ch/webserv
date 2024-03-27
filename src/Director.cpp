#include "Director.hpp"
#include <string.h>
#include "Log.hpp"

Director::Director(const std::string& config_path):fdmax(-1), config(new Config(config_path))
{

}

Director::~Director()
{
	delete config;
}

Director::Director(const Director& rhs)
{
	*this = rhs;
}

Director&	Director::operator=(const Director& rhs)
{
	if (this != &rhs)
	{}
	return (*this);
}

Config*	Director::get_config()
{
	return config;
}

// purpose: gets the inner address of sockaddr sa for both cases that 
// 			its a IPv4 and IPv6 address.
//
// argument: sa -> pointer to the given sockaddr of the addrinfo
//
// return void* -> pointer to the inner address.
void*	Director::get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
		return &(((struct sockaddr_in *)sa)->sin_addr);
	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// purpose: Initializes the Server that is passed in with Server*.
// 			We loop through the addreses of the Server and create for the first one 
//			a listener socket, which we set to be port-reusable and bind it to a port.
// 			we also save information to the Server (file descriptor, address)
//
// argument: si -> pointer to the Server information which we 
// 			 got from config parsing.
//
// return: int -> -1 if there was an error 0 if successfull
int	Director::init_server(Server *si)
{
	si->set_director(this);
	struct addrinfo hints, *ai, *p;
	int listener;
	int rv, yes=1 ;
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	std::stringstream pt;
	pt << si->get_port();
	if ((rv = getaddrinfo(NULL, pt.str().c_str(), &hints, &ai)) != 0) 
	{
		std::stringstream ss;
		ss << "Error reading addrinfo: " << gai_strerror(rv) << std::endl;
		Log::log(ss.str(), ERROR_FILE | STD_ERR);
		return -1;
	}
	for (p = ai; p != NULL; p = p->ai_next)
	{
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0)
		{
			std::stringstream ss;
			ss << "Error opening socket: " << strerror(errno) << std::endl;	
			Log::log(ss.str(), ERROR_FILE | STD_ERR);
			continue;
		}
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
		{
			close(listener);
			std::stringstream ss;
			ss << "Bind error: " << strerror(errno) << std::endl;
			Log::log(ss.str(), ERROR_FILE | STD_ERR);
			continue;
		}
		si->set_fd(listener);
		si->set_addr(*((sockaddr_storage*)(p->ai_addr)));
		si->set_addr_len((size_t)p->ai_addrlen);

//	print address
		std::string ipver;
		if (p->ai_family == AF_INET)
			ipver = "IPv4";
		else
			ipver = "IPv6";
		char ipstr[INET6_ADDRSTRLEN];
        inet_ntop(p->ai_family, p->ai_addr, ipstr, sizeof(ipstr));
        std::cout << ipver << " address: " << ipstr << std::endl;
		break;
	}

	if (p == NULL)
	{
		std::stringstream ss;
		ss << "Select server failed to bind." << std::endl;
		Log::log(ss.str(), ERROR_FILE | STD_ERR);
		return -1;
	}
	si->set_fd(listener);
	freeaddrinfo(ai);
	return 0;
}


// purpose: Take the Servers which we got from Config parsing
// 			and Initializes each one of them, set them so they don't block
// 			and makes them listen. The sockets are but in the read set for 
// 			the select method. We also add the server to the map of nodes 
// 			which is used for iterating through them.
//
// return: int -> -1 if there was an error 0 if successfull
int	Director::init_servers()
{
	FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);

	std::vector<Server*> servers = config->get_servers();
	std::vector<Server*>::iterator e = servers.end();
	std::vector<Server*>::iterator it ;
	for (it = servers.begin(); it != e; it++)
	{
		if (init_server(*it) < 0)
			return -1;
		int listener = (*it)->get_fd();
		if (fcntl(listener, F_SETFL, O_NONBLOCK) < 0)
		{
			std::stringstream ss;
			ss << "Error unblocking socket: " << strerror(errno) << std::endl;
			Log::log(ss.str(), ERROR_FILE | STD_ERR);
			return -1;
		}
		if (listen(listener, 800) == -1)
		{
			std::stringstream ss;
			ss << "Error listening: " << strerror(errno) << std::endl;
			Log::log(ss.str(), ERROR_FILE | STD_ERR);
			return -1;
		}
		FD_SET(listener, &read_fds);
		if (fdmax < listener) fdmax = listener;
		nodes[listener] = *it;
		nodes[listener]->set_type(SERVER_NODE);
		nodes[listener]->set_fd(listener);
	}
	return 0;
}

// purpose: In a loop we poll (with select) the statuses of the sockets.
// 			if they are ready for reading they create a new client sockets (if 
// 			its a server) and handle incoming or outgoing messages (if it's a client)
// 			we have a timeout of 1sec for the select(maybe we should modify)
//			and a timeout for the clients, if they are idle more then TIMEOUT_TIME
//
// return: int -> -1 if there was an error 0 if successfull
int	Director::run_servers()
{
	int ret;
	fd_set readfds_backup;
	fd_set writefds_backup;
	struct timeval timeout_time;
	while (true)
	{
		readfds_backup = read_fds;
		writefds_backup = write_fds;
		timeout_time.tv_sec = 1;
		timeout_time.tv_usec = 0;
		if ((ret = select(fdmax + 1, &readfds_backup, &writefds_backup, NULL, &timeout_time)) < 0 )
		{
			std::stringstream ss;
			ss << "Error while select: " << strerror(errno) << std::endl;
			Log::log(ss.str(), ERROR_FILE | STD_ERR);
			return -1;
		}
		for (int i = 0; i <= fdmax; i++)
		{
			if (nodes.find(i) != nodes.end() && nodes[i]->get_type() == SERVER_NODE)
			{
				if (FD_ISSET(i, &readfds_backup))
					if(create_client_connection(i) < 0)
					{
						std::stringstream ss;
						ss << "Error creating a client connection: " << std::endl;
						Log::log(ss.str(), ERROR_FILE | STD_ERR);
						exit(2); // TODO: Need to deallocate something?
					}
				
			}
			else if (nodes.find(i) != nodes.end() && nodes[i]->get_type() == CLIENT_NODE) 
			{
				if (FD_ISSET(i, &readfds_backup))
				{
					if(read_from_client(i) < 0)
					{
						std::stringstream ss;
						std::cerr << "Error reading from client: " << std::endl;
						Log::log(ss.str(), ERROR_FILE | STD_ERR);
					}
				}
				if (FD_ISSET(i, &write_fds))
				{
					if(write_to_client(i) < 0)
					{
						std::stringstream ss;
						std::cerr << "Error writing to client." << std::endl;
						Log::log(ss.str(), ERROR_FILE | STD_ERR);
					}
				}

			}
		}
/*
		char	remoteIP[INET6_ADDRSTRLEN];	
		//timeout for clients
		time_t curr_time = time(NULL);
		for (int i = 0; i < fdmax; i++)
		{	
			ClientInfo* client;
			if (nodes.find(i) != nodes.end() && nodes[i]->get_type() == CLIENT_NODE)
				client = dynamic_cast<ClientInfo *>(nodes[i]);
			if ((curr_time - client->get_prev_time()) > TIMEOUT_TIME)
			{
				std::stringstream ss2;

				ss2 << "Closing connection from ";
				ss2 << inet_ntop(client->get_addr().ss_family,
					get_in_addr((struct sockaddr *)&client->get_addr()),
					remoteIP, INET6_ADDRSTRLEN);
				ss2 << " on socket " << i << std::endl;
				Log::log(ss2.str(), ACCEPT_FILE | STD_OUT);

				if (FD_ISSET(i, &write_fds))
					FD_CLR(i, &write_fds);
				if (FD_ISSET(i, &read_fds))
					FD_CLR(i, &read_fds);
				if (i == fdmax)
					fdmax--;
				delete client;
				nodes.erase(i);
				close(i);
			}

		}
*/
	}
	return 0;
}

// purpose: having the server socket file descriptor we create the client connection
// 			add this connection to the list of ClientInfos. Log the connection in
// 			the accept log. and set the new socket connection to non-blocking.
//
// argument: listener -> the servers socket that got the client message 
//
// return: int -> -1 if it failed and 0 for success
int	Director::create_client_connection(int listener)
{
	struct sockaddr_storage remoteaddr;
	socklen_t 				addrlen = sizeof remoteaddr;
	char					remoteIP[INET6_ADDRSTRLEN];	

	int newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);
	if (newfd == -1)
	{
		std::stringstream	ss;
		ss << "Error accepting client: " << strerror(errno) << std::endl;
		Log::log(ss.str(), ERROR_FILE | STD_ERR);
		return -1;
	}
	else
	{
		if (fdmax < newfd)
			fdmax = newfd;
		if (nodes.find(newfd) == nodes.end())
		{
			ClientInfo *newcl = new ClientInfo(newfd, remoteaddr, (size_t)addrlen);
			newcl->set_server(dynamic_cast<Server*>(nodes[listener]));
			nodes[newfd] = newcl;
		}
		else
		{
			std::stringstream ss;
			ss << "Tried to overwrite socket: " << newfd << std::endl;
			Log::log(ss.str(), STD_ERR | ERROR_FILE);
			exit(2);
		}
		std::stringstream ss2;

		ss2 << "New connection from ";
		ss2 << inet_ntop(remoteaddr.ss_family,
						get_in_addr((struct sockaddr *)&remoteaddr),
						remoteIP, INET6_ADDRSTRLEN);
		ss2 << " on socket " << newfd << std::endl;
		Log::log(ss2.str(), ACCEPT_FILE | STD_OUT);
		if (fcntl(newfd, F_SETFL, O_NONBLOCK) < 0)
		{
			std::stringstream ss3;
			ss3 << "Error while non-blocking: " << strerror(errno) << std::endl;
			Log::log(ss3.str(), ERROR_FILE | STD_ERR);
			delete nodes[newfd];
			nodes.erase(newfd);
			close(newfd);
		}
		FD_SET(newfd, &read_fds);
	}
	return 0;
}

// purpose: we check if the client closed the connection. if he did we log it,
// 			free the ClientInfo and erase the client_fd from the select set and 
// 			the iterator list. Else we parse (TODO danil) the request message and fill
// 			the ClienInfo with information. After parsing we put the socket in 
// 			the set of writing sockets of the select, so we can answer the request. 
//
// argument: clientfd -> the file descriptor
//
// return: int -> -1 if it failed and 0 for success

int	Director::read_from_client(int client_fd)
{
	char			msg[MSG_SIZE];
	char			remoteIP[INET6_ADDRSTRLEN];	
	int				num = 0;
	ClientInfo		*ci;

	ci = dynamic_cast<ClientInfo *>(nodes[client_fd]);
	memset(&msg, 0, MSG_SIZE);
	num = read(client_fd, msg, MSG_SIZE);
	if (!num)
	{
		std::stringstream ss;
		ss << "Connection closed by " << inet_ntop(AF_INET, get_in_addr((struct sockaddr *)&ci->get_addr()),
						remoteIP, INET6_ADDRSTRLEN);
		ss << " on socket " << client_fd << std::endl;
		Log::log(ss.str(), ACCEPT_FILE | STD_OUT);
		if (FD_ISSET(client_fd, &write_fds))
			FD_CLR(client_fd, &write_fds);
		if (FD_ISSET(client_fd, &read_fds))
			FD_CLR(client_fd, &read_fds);
		if (client_fd == fdmax)
			fdmax--;
		delete nodes[client_fd];
		nodes.erase(client_fd);
		close(client_fd);
	}
	else if (num == -1)
	{
		if (FD_ISSET(client_fd, &write_fds))
			FD_CLR(client_fd, &write_fds);
		if (FD_ISSET(client_fd, &read_fds))
			FD_CLR(client_fd, &read_fds);
		if (client_fd == fdmax)
			fdmax--;
		nodes.erase(client_fd);
		close(client_fd);
		std::stringstream ss;
		ss << "Error reading from socket: " << client_fd << std::endl;
		Log::log(ss.str(), ERROR_FILE | STD_ERR);
		return -1;	
	}
	else 
	{
		ci->set_time();
		try
		{
			Request	req;
			req.init(msg);
			memset(msg, 0, sizeof(msg));
			ci->get_server()->create_response(req); 
			FD_CLR(client_fd, &read_fds);
			if (client_fd == fdmax)	fdmax--;
			FD_SET(client_fd, &write_fds);
			if (client_fd > fdmax)	fdmax = client_fd;
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
		}
	}
	return 0;
}


// purpose: (TODO) after getting the request message, this function sends back
// 			the answer, which in a simple case is the requested file or in case of a
// 			cgi the generated file. 
//
// return: int -> -1 if it failed and 0 for success
int	Director::write_to_client(int fd)
{
	int				num_bytes;
	ClientInfo*		cl = dynamic_cast<ClientInfo*>(nodes[fd]);

	std::string content = cl->get_server()->get_response();
	int sz = content.size();

	if (sz < MSG_SIZE)
		num_bytes = write(fd, content.c_str(), sz);
	else
		num_bytes = write(fd, content.c_str(), MSG_SIZE);

	if (num_bytes < 0)
	{
		std::stringstream ss;
		ss << "Error sending a response: " << strerror(errno);
		Log::log(ss.str(), STD_ERR | ERROR_FILE);
		if (FD_ISSET(fd, &write_fds))
		{
			FD_CLR(fd, &write_fds);
			if (fd == fdmax) { fdmax--; }  
		}
		if (FD_ISSET(fd, &read_fds))
		{
			FD_CLR(fd, &read_fds);
			if (fd == fdmax) { fdmax--; }  
		}
		close(fd);
		nodes.erase(fd);
	}
	if (num_bytes == (int)(content.size()) || num_bytes == 0)
	{
		std::stringstream ss;
		ss << "Response send to socket:" << fd << std::endl;;
		Log::log(ss.str(), STD_OUT);
		if (FD_ISSET(fd, &write_fds))
		{
			FD_CLR(fd, &write_fds);
			if (fd == fdmax) { fdmax--; }  
		}
		FD_SET(fd, &read_fds);
		if (fd == fdmax) { fdmax=fd; }  
		cl->get_server()->reset();
	}
	else
	{
		cl->set_time();
		cl->get_server()->set_response(cl->get_server()->get_response().substr(num_bytes));
	}
	return (0);
}
