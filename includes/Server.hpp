#ifndef SERVER_HPP
# define SERVER_HPP

// C++
# include <iostream>
# include <algorithm>
# include <vector>
# include <set>
# include <string>
# include <exception>
// C
# include <sys/socket.h>
# include <sys/types.h>
# include <sys/select.h>
# include <sys/stat.h>
# include <sys/wait.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/time.h>
# include <unistd.h>
# include <cstdlib>
# include <signal.h>
# include <fcntl.h>
# include <errno.h>
# include <stdio.h>
# include <dirent.h>
// Webserv
# include "PortListener.hpp"
# include "Configuration.hpp"
# include "Resource.hpp"
# include "Request.hpp"
# include "Response.hpp"
# include "CGIResponse.hpp"
# include "ClientSet.hpp"
# include "MimeTypes.hpp"
# include "openssl/ssl.h"
# include "openssl/err.h"
# include "CGI.hpp"
# include "CGIBody.hpp"
# include "Utility.hpp"
# include "StreamJob.hpp"
# include "File.hpp"
# include "Logger.hpp"

class Server {
private:
	Configuration	conf;
	bool			validConf;
	ClientSet		clientSockets;
	fd_set			readfds;
	fd_set			writefds;
	std::vector<PortListener*>			listeners;
	std::vector<std::pair<int, bool> >	cgiOutputs;

	typedef void (Server::*RequestFnc)(Request const &request, Response &response);
	static RequestFnc executeMethods[6];

	ServerConfiguration const *matchRoute(Request const &request);
	bool preProcessRequest(Request &request, Response &response);
	void executeOPTIONS(Request const &request, Response &response);
	void executeGET(Request const &request, Response &response);
	void executePOST(Request const &request, Response &response);
	void executePUT(Request const &request, Response &response);
	void executeDELETE(Request const &request, Response &response);
	void executeHEAD(Request const &request, Response &response);
	void executeCGI(Client const &client, Request const &request, Response *&response);
	void autoIndex(Request const &request, Response &response);

	bool checkAuthenticate(const Request &request) const;
	bool checkAllowed(Request const &request) const;

	bool setResponseError(Request const &request, Response &response, RFC::Status status);
	bool getIndexFromConf(Resource &resource, Request const &request);
	std::string findCorrectFile(const Request &request);
	std::string getCharsetFromHTML(const std::string &file);
	std::map<std::string, std::string> getMatchedFile(Resource const &resource, Request const &request);
	bool isCgiScript(const Request &request);
	void freeCGI(char **argv, char **envp);
public:
	Server(std::string const &configurationFile);
	virtual ~Server();

	void execute(Client const &client, Request &request, Response *&response);
	bool isValidConf(void) const;
	void callbackPUT(Request const &request, Response &res, int tempFd);

	void init(void);
	void start(void);
	static void	stop(int);
};

#endif
