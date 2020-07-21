#ifndef STREAMBODY_HPP
# define STREAMBODY_HPP
# include "BodyList.hpp"
# include "StreamJob.hpp"
# include "Client.hpp"
# include "Response.hpp"
# include <list>

class StreamJob;
class Client;
class Server;

typedef void (Server::*responseCallback)(const Request&, Response&, int);

class StreamBody
{
private:
	Client& client;
	std::list<StreamJob*>& jobList;
	StreamBody();
public:
	StreamBody(Client& client, std::list<StreamJob*>& jobList);
	~StreamBody();
	void pushJob(int fd, Response& response, int maxBodySize, Server *server = nullptr, responseCallback callback = nullptr, std::string fileTmp = "");
};

#endif /* STREAMBODY_HPP */
