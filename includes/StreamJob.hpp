#ifndef STREAMJOB_HPP
# define STREAMJOB_HPP

# include "Client.hpp"
# include "Response.hpp"
# include "File.hpp"

class Server;
class Client;

typedef void (Server::*responseCallback)(const Request&, Response&, int);
class StreamJob {
	int			fd;
	Client&		client;
	BodyList*	list;
	Response&	response;
	responseCallback callback;
	bool		expired;
	int			maxBodySize;
	int			totalSize;
	Server		*server;
	std::string	fileTmp;
public:
	StreamJob(int fd, Client& client, Response& resp, int maxBodySize, Server *server = nullptr, responseCallback callback = nullptr, std::string fileTmp = "");
	~StreamJob();
	bool isEmpty(void) const;
	operator int();
	bool isCompleted(void) const;
	bool stream();
	void expire();
	bool isExpired();
};

#endif /* STREAMJOB_HPP */
