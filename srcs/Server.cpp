#include "Server.hpp"
#if __linux__
# define st_mtimespec st_mtim
#endif

Server::RequestFnc Server::executeMethods[] = {
	&Server::executeOPTIONS,
	&Server::executeGET,
	&Server::executeHEAD,
	&Server::executePOST,
	&Server::executePUT,
	&Server::executeDELETE
};

volatile sig_atomic_t serverKeepRunning;

Server::Server(std::string const &configurationFile):
	validConf(true), clientSockets(), listeners(), cgiOutputs() {
	Logger::setOptions(Logger::DEFAULT/* | Logger::SHOW_VERBOSE */);
	try {
		this->conf.loadFromFile(configurationFile);
		this->conf.check();
		// this->conf.DEBUG_print();
	} catch (std::exception &e) {
		Logger::error(e.what());
		this->validConf = false;
	}
	ft_memset(&this->readfds, 0, sizeof(this->readfds));
	ft_memset(&this->writefds, 0, sizeof(this->writefds));
}

Server::~Server() {
	auto ite = this->listeners.end();
	for (auto it = this->listeners.begin(); it != ite; ++it)
		delete *it;
	this->listeners.clear();
	EVP_cleanup();
	Logger::info("Stopping server");
}

/**
 * Initialize server
 **/
void Server::init(void) {
	this->clientSockets.init(this->conf.maxClients, this->conf.timeToLive);
	std::set<int> ports;

	auto ite = this->conf.servers.end();
	for (auto it = this->conf.servers.begin(); it != ite; ++it) {
		if (ports.find(it->port) == ports.end()) {
			Logger::info("Added server \033[1;35m", (*it).server_name[0], "\033[0m on port \033[1;36m", it->port, "\033[0m");
			PortListener *pl = new PortListener();
			this->listeners.push_back(pl);
			pl->init(it->port);
			pl->bind(this->conf.maxClients);
			ports.insert(it->port);
		}
		if (it->SSL && ports.find(it->portSSL) == ports.end()) {
			Logger::info("Added server \033[1;35m", (*it).server_name[0], "\033[0m on port \033[1;34m", it->portSSL, " (SSL)\033[0m");
			PortListener *pl = new PortListener();
			this->listeners.push_back(pl);
			pl->init(it->portSSL, it->certPath, it->keyPath);
			pl->bind(this->conf.maxClients);
			ports.insert(it->portSSL);
		}
	}

	SSL_load_error_strings();
	OpenSSL_add_ssl_algorithms();
}

/**
 * Stop server
 **/
void Server::stop(int sig) {
	(void)sig;
	serverKeepRunning = 0;
	Logger::info("\nExiting...");
}

/**
 * Start server and listen on socket
 **/
void Server::start(void) {
	int		max_fd,
			activity;
	timeval	timeout;
	Client* it;
	std::list<StreamJob*> Jobs;

	serverKeepRunning = 1;
	signal(SIGCHLD, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, Server::stop);

	Logger::info("\033[1;32mWebserv started\033[0m");
	while (serverKeepRunning) {
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;

		// Clear fd_set and update it with Listener sockets
		FD_ZERO(&this->readfds);
		FD_ZERO(&this->writefds);

		// Add listeners to read
		max_fd = clientSockets.addToFdSet(&this->readfds, &this->writefds);
		auto ite = this->listeners.end();
		for (auto it = this->listeners.begin(); it != ite; ++it) {
			(*it)->fdSet(this->readfds);
			max_fd = (*it)->max(max_fd);
		}

		// Add Jobs (write) to select
		for (auto it = Jobs.begin(); it != Jobs.end(); ++it) {
			if ((*it)->isExpired()) {
				delete *it;
				it = Jobs.erase(it);
			} else if (!(*it)->isEmpty()) {
				int fd = *(*it);
				FD_SET(fd, &this->writefds);
				if (fd > max_fd) max_fd = fd;
			}
		}

		// Add CGI outputs (write) to select
		for (auto &fd : this->cgiOutputs) {
			if (!fd.second) { // Only add non-ready cgi fds
				FD_SET(fd.first, &this->readfds);
				if (fd.first > max_fd)
					max_fd = fd.first;
			}
		}

		// Waits for an activity on one of the sockets
		activity = select(max_fd + 1, &this->readfds, &this->writefds, NULL, &timeout);
		if (activity < 0 && errno != EINTR)
			Logger::error("ERRNO ", errno, ": ", strerror(errno));
		if (activity < 1) {
			this->clientSockets.removeTimedOut();
			continue ;
		}

		// Add new accepted clients
		for (auto it = this->listeners.begin(); it != ite; ++it) {
			if ((*it)->fdIsSet(this->readfds)) {
				if ((*it)->isSSL())
					this->clientSockets.addClient((*it)->accept(), (*it)->getCtx());
				else
					this->clientSockets.addClient((*it)->accept());
			}
		}

		// Set state of cgiOutputs
		for (auto it = this->cgiOutputs.begin(), ite = this->cgiOutputs.end(); it != ite; ++it)
			if (FD_ISSET((*it).first, &this->readfds))
				(*it).second = true;

		it = nullptr;
		while ((it = clientSockets.next(it))) {
			// Request
			if (FD_ISSET(*it, &this->readfds)) {
				bool doRecall = false;
				do {
					it->keepAlive();
					Request* request = it->receive(doRecall, Jobs);
					if (request) {
						// Display that we received a request
						struct timeval time;
						ft_memset(&time, 0, sizeof(struct timeval));
						::gettimeofday(&time, 0);
						Logger::info("\033[0;36m", RFC::timeToString(time.tv_sec),
								"\t\033[0;33mRECEIVE\t\033[1;35m", it->getIp(), "\033[0m\t",
								request->getStringMethod(), " ",
								request->getUri(), " ",
								request->getVersion());
						// Generate response
						Response *response = new Response();
						if (response) {
							if (this->preProcessRequest(*request, *response))
								this->execute(*it, *request, response);
							if (request->getMethod() == RFC::HEAD)
								response->setBody(nullptr);
							it->pushResponse(response);
							// Delete Request only if it wasn't just an Expect
							if (request->header("Expect") == "100-continue") {
								request->removeHeader("Expect");
							} else if (!response->getDontSend() && !request->streamBody)
								delete request;
						} else Logger::error("couldn't allocate enough memory for a new Response.");
					}
				} while (doRecall);
			}
			// Response
			if (it->hasResponses() && FD_ISSET(*it, &this->writefds))
				it->setReady(true);
			if (it->hasResponses() && it->nextResponse()->isBodyReady() && it->isReady()) {
				// If there was an empty CGI error without anything
				if (it->sendResponse() == 2) {
					Response *newResponse = new Response();
					newResponse->setStatus(RFC::InternalServerError);
					newResponse->defaultError(RFC::InternalServerError);
					it->insertResponse(newResponse);
					// Response with Request for error file
				}
			}
		}

		// Jobs
		for (auto it = Jobs.begin(), end = Jobs.end(); it != end;) {
			StreamJob* job = *it;
			if (!job->isExpired() && !job->isEmpty() && FD_ISSET(*job, &this->writefds))
				job->stream();
			if (job->isExpired() || job->isCompleted()) {
				delete job;
				it = Jobs.erase(it);
			} else
				++it;
		}

		this->clientSockets.removeTimedOut();
	}
}

/**
 * Match the first server that have the same Hostname and port as the Host of the Request
 * 	Then match the location of the requested ressource.
 * 	if nothing is found, the first match is the default.
 **/
ServerConfiguration const *Server::matchRoute(Request const &request) {
	size_t pos = 0;
	bool isSubfolder = false;
	// Search through all servers
	for (const auto &server : this->conf.servers) {
		// Match by Hostname and port (or SSL port)
		if (server.matchHost(request.host().name) && server.matchPort(request.host().port)) {
			// Set the default server as the first found
			for (const auto &location : server.locations) {
				pos = request.getUri().find(location.path);
				isSubfolder = (request.getUri().length() == location.path.length() || location.path == "/" || request.getUri()[location.path.length()] == '/');
				// Check subfolder
				if (pos == 0 && isSubfolder)
					return (&location);
			}
			return (&server);
		}
	}
	return (&this->conf.servers[0]);
}

bool Server::preProcessRequest(Request &request, Response &response) {
	// Default Host if there is none
	if (request.host().name.length() == 0)
		request.setDefaultHost(this->conf.servers[0]);
	// Set Request configuration objects
	request.setConf(this->matchRoute(request));
	// Response values inherited from Request
	if (request.conf().advertiseServer)
		response.addHeader("Server", "webserv");
	if (request.conf().gzip)
		response.setCompression(request.getAcceptedCompression());
	response.setUri(request.getUri());
	// Check if request is valid
	if (!request.hasValidRequest() || !request.hasValidHeaders()
		|| (request.header("Transfer-Encoding") == "chunked" && request.contentLength() > 0)) {
		return (this->setResponseError(request, response, RFC::BadRequest));
	} else if (!request.hasValidMethod()) {
		return (this->setResponseError(request, response, RFC::NotImplemented));
	} else if (!request.hasValidVersion()) {
		return (this->setResponseError(request, response, RFC::HTTPVersionNotSupported));
	} else if (request.getUri().length() > 2048)
		return (this->setResponseError(request, response, RFC::RequestURITooLarge));
	else if (request.contentLength() > static_cast<size_t>(request.conf().maxBodySize))
		return (this->setResponseError(request, response, RFC::RequestEntityTooLarge));
	else if (request.getMethod() == RFC::INVALID)
		return (this->setResponseError(request, response, RFC::BadRequest));
	// Check request validity for the route
	if (!checkAuthenticate(request))
		return (this->setResponseError(request, response, RFC::Unauthorized));
	else if (!checkAllowed(request)) {
		std::string allowed;
		for (const auto &method : request.conf().acceptedMethods) {
			if (method.second) {
				if (allowed.length() > 0)
					allowed.append(", ");
				allowed.append(RFC::MethodString[method.first]);
			}
		}
		response.addHeader("Allow", allowed);
		return (this->setResponseError(request, response, RFC::MethodNotAllowed));
	}
	// Check if the request is just an Expect
	if (request.header("Expect") == "100-continue") {
		response.setStatus(RFC::Continue);
		return (false);
	}
	// Redirect to SSL if required
	bool upgradeHeader = (request.header("Upgrade-Insecure-Requests") == "1");
	if (request.conf().SSL && !request.getIsSSL() && (request.conf().forceSSL || upgradeHeader)) {
		response.setStatus(RFC::MovedPermanently);
		response.redirectSSL(request.conf().portSSL, request);
		return (false);
	}
	return (true);
}

/**
 * Check if request is valid and then dispatch to appropriate method depending on request type
 **/
void Server::execute(Client const &client, Request &request, Response *&response) {
	if (isCgiScript(request) && request.getMethod() != RFC::OPTIONS) {
		executeCGI(client, request, response);
		return ;
	}
	(this->*Server::executeMethods[request.getMethod()])(request, *response);
}

/**
 * Execute OPTIONS request
 * Creates a response with appropriate allow header
 **/
void Server::executeOPTIONS(Request const &request, Response &res) {
	if (request.streamBody) {
		request.streamBody->pushJob(File::devNull, res, request.conf().maxBodySize);
		res.setDontSend(true);
	}

	Resource resource(request.conf().root, request.getRelUri());

	if (!resource.isValid() && !resource.isDirectory()) {
		this->setResponseError(request, res, resource.RFCStatus());
		return;
	}

	res.setStatus(RFC::NoContent);
	std::string allowed;
	for (const auto &method : request.conf().acceptedMethods) {
		if (method.second) {
			if (allowed.length() > 0)
				allowed.append(", ");
			allowed.append(RFC::MethodString[method.first]);
		}
	}
	res.addHeader("Allow", allowed);
}

/**
 * Executes GET request
 * Creates a response with some headers and a body
 **/
void Server::executeGET(Request const &request, Response &res) {
	if (request.streamBody) {
		request.streamBody->pushJob(File::devNull, res, request.conf().maxBodySize);
		res.setDontSend(true);
	}

	std::map<std::string, std::string> file;
	Resource resource(request.conf().root, request.getRelUri());

	if (File::usedFds() + 1 >= FD_SETSIZE) {
		this->setResponseError(request, res, RFC::ServiceUnavailable);
		return ;
	}

	if (!resource.isDirectory() && (request.acceptLanguage().size() > 0 || request.acceptCharset().size() > 0)) {
		file = this->getMatchedFile(resource, request);
		if (file.find("uri") == file.end()) {
			this->setResponseError(request, res, RFC::NotAcceptable);
			return ;
		} else if (file["uri"] != resource.relativePath()) {
			resource.load(request.conf().root, file["uri"]);
		}
	}

	if (!resource.isValid() && !resource.isDirectory()) {
		this->setResponseError(request, res, resource.RFCStatus());
		return ;
	}

	if (resource.isDirectory()) {
		if (!getIndexFromConf(resource, request)) {
			if (request.conf().autoindex == true) {
				autoIndex(request, res);
				return;
			}
			this->setResponseError(request, res, RFC::NotFound);
			return ;
		}
		// * Duplicate of above
		if (!resource.isValid() && !resource.isDirectory()) {
			this->setResponseError(request, res, resource.RFCStatus());
			return ;
		}
	}

	int fd = File::open(resource.relativeRootPath().c_str(), O_RDONLY);
	if (fd == -1) {
		if (errno == EACCES || errno == EPERM)
			this->setResponseError(request, res, RFC::Forbidden);
		else
			this->setResponseError(request, res, RFC::InternalServerError);
		return ;
	}

	res.setStatus(RFC::Ok);
	res.setBody(resource, fd);
	#ifdef __MACH__
		res.addHeader("Last-Modified", RFC::timeToString(resource.mtime()));
	#else
		res.addHeader("Last-Modified", RFC::timeToString(resource.mtime()));
	#endif
	std::string contentType;
	contentType = MimeTypes::getType(resource.relativePath());
	if (contentType.find("text") != std::string::npos) {
		contentType += "; charset=";
		if (file.find("charset") != file.end())
			contentType.append(file["charset"]);
		else
			contentType.append("iso-8859-1");
	}
	res.addHeader("Content-Type", contentType);
	res.addHeader("Content-Location", "/" + resource.relativePath());
	if (file.find("language") != file.end() && file["language"].size())
		res.addHeader("Content-Language", file["language"]);
}

/**
 * Execute POST request
 **/
void Server::executePOST(Request const &request, Response &res) {
	if (request.streamBody) {
		request.streamBody->pushJob(File::devNull, res, request.conf().maxBodySize);
		res.setDontSend(true);
	}
	res.setStatus(RFC::Ok);
	res.addHeader("Content-Length", "0");
}

void Server::callbackPUT(Request const &request, Response &res, int tempFd) {
	Resource resource(request.conf().root, request.getRelUri());
	Resource uploadedFile(request.conf().uploadedFileLocation, resource.fullFilename());

	char buffer[4096];

	bool alreadyExist = uploadedFile.isValid();
	int openFlags = (alreadyExist) ? O_WRONLY | O_TRUNC : O_WRONLY | O_APPEND | O_CREAT;
	if (alreadyExist && uploadedFile.isDirectory()) {
		this->setResponseError(request, res, RFC::Forbidden);
		return ;
	}

	if (lseek(tempFd, 0, SEEK_SET) < 0) {
		this->setResponseError(request, res, RFC::InternalServerError);
		return;
	}

	int fd = 0;
	if ((fd = File::open(uploadedFile.relativeRootPath().c_str(), openFlags, 0666)) == -1) {
		if (errno == EACCES || errno == EPERM)
			this->setResponseError(request, res, RFC::Forbidden);
		else
			this->setResponseError(request, res, RFC::InternalServerError);
		return ;
	}
	fcntl(fd, F_SETFL, O_NONBLOCK);

	// Copy tmp file to definitive file
	// This loop handle write fails with less than what was read
	std::string writeFails = "";
	int rRet = 0, wRet = 0;
	do {
		if (writeFails.length() > 0) {
			if ((wRet = ::write(fd, writeFails.c_str(), writeFails.length())) == -1) {
				File::close(fd);
				this->setResponseError(request, res, RFC::InternalServerError);
				return ;
			} else if (wRet > 0) {
				writeFails.erase(0, wRet);
			}
		}
		while (writeFails.length() == 0 && (rRet = ::read(tempFd, buffer, 4096)) > 0) {
			if ((wRet = ::write(fd, buffer, rRet)) == -1) {
				File::close(fd);
				this->setResponseError(request, res, RFC::InternalServerError);
				return ;
			} else if (wRet != rRet) {
				writeFails.append(buffer + wRet, rRet - wRet);
			}
		}
	} while (writeFails.length() > 0);

	File::close(fd);

	res.addHeader("Content-Location", uploadedFile.relativePath());
	res.addHeader("Content-Length", "0");
}

/**
 * Execute PUT request
 * Creates resource or update it if it exists
 **/
void Server::executePUT(Request const &request, Response &res) {
	Resource resource(request.conf().root, request.getFileUri());
	int fd;

	if (File::usedFds() + 2 >= FD_SETSIZE) {
		this->setResponseError(request, res, RFC::ServiceUnavailable);
		return ;
	}

	if (!request.conf().allowFileUpload) {
		this->setResponseError(request, res, RFC::Forbidden);
		return ;
	}

	Resource testFolder(request.conf().uploadedFileLocation, resource.relativeFolder());

	if (!testFolder.isDirectory()) {
		this->setResponseError(request, res, RFC::InternalServerError); // folder for uploaded file doesn't exist
		return ;
	}

	timeval now;
	gettimeofday(&now, NULL);
	Resource tempFile(request.conf().uploadedFileLocation, ::to_string((int) (now.tv_sec + now.tv_usec)));

	bool alreadyExist = tempFile.isValid();
	int openFlags = (alreadyExist) ? O_RDWR | O_TRUNC : O_RDWR | O_APPEND | O_CREAT;
	if (alreadyExist && tempFile.isDirectory()) {
		this->setResponseError(request, res, RFC::Forbidden);
		return ;
	}

	if ((fd = File::open(tempFile.relativeRootPath().c_str(), openFlags, 0666)) == -1) {
		if (errno == EACCES || errno == EPERM)
			this->setResponseError(request, res, RFC::Forbidden);
		else
			this->setResponseError(request, res, RFC::InternalServerError);
		return ;
	}
	fcntl(fd, F_SETFL, O_NONBLOCK);

	if (request.streamBody) {
		request.streamBody->pushJob(fd, res, request.conf().maxBodySize, this, &Server::callbackPUT, tempFile.relativeRootPath());
		res.setDontSend(true);
	} else {
		this->callbackPUT(request, res, fd);
		File::close(fd);
		tempFile.unlink();
	}
	if (alreadyExist) {
		res.setStatus(RFC::Created);
	} else {
		res.setStatus(RFC::NoContent);
	}
}

/**
 * Execute DELETE request
 * Deletes specified resource
 **/
void Server::executeDELETE(Request const &request, Response &res) {
	Resource resource(request.conf().root, request.getRelUri());

	if (!resource.unlink()) {
		this->setResponseError(request, res, resource.RFCStatus());
		return ;
	}
	res.setStatus(RFC::NoContent);
}

/**
 * Execute HEAD request
 * Construct same header as GET request without the body
 **/
void Server::executeHEAD(Request const &request, Response &res) {
	this->executeGET(request, res);
}

void Server::freeCGI(char **argv, char **envp) {
	for (size_t i = 0; argv[i]; ++i)
		delete argv[i];
	for (size_t i = 0; envp[i]; ++i)
		delete envp[i];
	delete[] envp;
}

void Server::executeCGI(Client const &client, Request const &request, Response *&res) {
	Resource resource(request.conf().root, request.getRelUri());

	// Up to 7 FDS can be used by CGI
	if (File::usedFds() + 7 >= FD_SETSIZE) {
		this->setResponseError(request, *res, RFC::ServiceUnavailable);
		return ;
	}

	res->addHeader("Content-Type", "text/html");
	res->setStatus(RFC::Ok);

	std::map<std::string, std::string> test;
	std::vector<std::string> const *exec = nullptr;

	cgi cgi(client, resource, request);

	for (auto &header : request.getHeaders()) {
		cgi.addHeader(header);
	}

	char **envp = cgi.getEnv();
	if (envp == NULL){
		setResponseError(request, *res, RFC::InternalServerError);
		return ;
	}

	for (const auto &e : request.conf().cgiExtensions) {
		if (e.first == resource.extension()) {
			exec = &e.second;
		}
	}

	char *argv[exec->size() + 2];
	ft_memset(argv, 0, exec->size() + 2);
	// If there is only an extension, execute the file directly
	if (exec->size() == 0) {
		argv[0] = ft_strdup(("./" + resource.relativeRootPath()).c_str());
		argv[1] = 0;
	// Else append each params as an execve param
	} else {
		size_t i = 0;
		auto ite = exec->end();
		for (auto it = exec->begin(); it != ite; ++it)
			argv[i++] = ft_strdup((*it).c_str());
		argv[i++] = ft_strdup(resource.absolutePath().c_str());
		argv[i] = 0;
	}

	// State
	int fdin[2] = { -1, -1 };
	int fdout[2] = { -1, -1 };
	if (File::pipe(fdin) == -1 || File::pipe(fdout) == -1) {
		File::close(fdin[0]); File::close(fdin[1]);
		File::close(fdout[0]); File::close(fdout[1]);
		freeCGI(argv, envp);
		this->setResponseError(request, *res, RFC::InternalServerError);
		return ;
	}
	fcntl(fdin[1], F_SETFL, O_NONBLOCK); // Only the write side need fcntl
	// fcntl(fdout[1], F_SETFL, O_NONBLOCK);

	CGIResponse *newResponse = new CGIResponse(*res, this->cgiOutputs);
	delete res;
	res = newResponse;

	// Send body to child
	if (request.streamBody) {
		request.streamBody->pushJob(fdin[1], *res, request.conf().maxBodySize);
		res->setDontSend(false);
	} else {
		File::close(fdin[1]);
	}
	// Exec
	pid_t pid = fork();
	if (pid < 0) {
		File::close(fdin[0]);
		File::close(fdout[0]);
		File::close(fdout[1]);
		freeCGI(argv, envp);
		this->setResponseError(request, *res, RFC::InternalServerError);
		return ;
	} else if (pid == 0) {
		File::close(fdout[0]);
		File::close(fdin[1]);
		// dup input
		File::dup2(fdin[0], 0);
		File::close(fdin[0]);
		// dup for output
		File::dup2(fdout[1], 1);
		File::close(fdout[1]);
		execve(argv[0], argv, envp);
		std::cout << "X-Error-Internal:500\r\n\r\n";
		exit(EXIT_FAILURE); // handle excecve error
	} else {
		File::close(fdin[0]);
		File::close(fdout[1]);
		freeCGI(argv, envp);
		this->cgiOutputs.push_back(std::make_pair(fdout[0], false));
		newResponse->setBody(new CGIBody(fdout[0]));
		newResponse->forMethod(request.getMethod());
	}
}

bool Server::isValidConf(void) const {
	return (this->validConf);
}

/**
 * Return error response with file from conf
 **/
bool Server::setResponseError(Request const &request, Response &res, RFC::Status status) {
	res.setStatus(status);
	if (File::usedFds() + 1 <= FD_SETSIZE) {
		auto errorPage = request.conf().errorPages.find(status);
		if (errorPage != request.conf().errorPages.end()) {
			Resource resource(errorPage->second);
			if (resource.isValid()) {
				int fd = File::open(errorPage->second.c_str(), O_RDONLY);
				if (fd)
					res.setBody(resource, fd);
			}
		}
	}
	if (!res.getBody()) {
		res.defaultError(status);
	}
	if (status == RFC::Unauthorized)
		res.addHeader("WWW-Authenticate", "Basic");
	if (status == RFC::ServiceUnavailable)
		res.addHeader("Retry-After", "60");
	return (false);
}

/**
 * Check in the conf if method is allowed for request
 **/
bool Server::checkAllowed(Request const &request) const {
	for (const auto &m : request.conf().acceptedMethods) {
		if (m.first == request.getMethod() && m.second) {
			return (true);
		}
	}
	return (false);
}

/**
 * Get index page for directory
 **/
bool Server::getIndexFromConf(Resource &resource, Request const &request) {
	Resource r;
	for (const auto &page : request.conf().indexPages) {
		r.load(resource.relativeRootPath(page));
		if (r.isValid()) {
			resource = r;
			return (true);
		}
	}
	return (false);
}


/**
 * Check that requst is authenticated if necessary
 **/
bool Server::checkAuthenticate(const Request &request) const {
	if (request.conf().authBasic && request.conf().users.size()) {
		if (request.authorization().first != "Basic")
			return false;
		if (request.authorization().second.size()) {
			for (const auto &user : request.conf().users) {
				if (user == request.authorization().second)
					return true;
			}
			return false;
		} else {
			return false;
		}
	}
	return true;
}

/**
 * If lhs has size inferior to rhs, returns true
 **/
bool nbrWords(std::vector<std::string > lhs, std::vector<std::string> rhs) {
	if (lhs.size() < rhs.size())
		return true;
	return false;
}

/**
 * find files with appropriate language
 **/
std::list<std::vector<std::string> > matchLanguage(std::list<std::vector<std::string> > files, std::vector<AcceptQuality> const &languages, std::string *language) {
	std::list<std::vector<std::string> > ret;
	for (const auto &lan : languages) {
		for (const auto &file : files) {
			if (std::find(file.begin(), file.end(), lan.value) != file.end()) {
				ret.push_back(file);
			}
		}
		if (ret.size() != 0 && language->size() == 0) {
			*language = lan.value;
		}
	}
	return ret;
}

/**
 * find files with appropriate charset
 **/
std::list<std::vector<std::string> > matchCharset(std::list<std::vector<std::string> > files, std::vector<AcceptQuality> const &charsets, std::string *charset) {
	std::list<std::vector<std::string> > ret;
	for (const auto &chr : charsets) {
		for (const auto &file : files) {
			if (std::find(file.begin(), file.end(), chr.value) != file.end()) {
				ret.push_back(file);
			}
		}
		if (ret.size() != 0) {
			*charset = chr.value;
			return ret;
		}
	}
	return ret;
}

/**
 * find files that are similar to uri
 **/
bool matchUri(std::vector<std::string> uri, std::vector<std::string> file) {
	std::vector<std::string>::const_iterator it;
	for (const auto &u : uri) {
		for (it = file.begin(); it != file.end(); it++) {
			if (u == *it) {
				break;
			}
		}
		if (it == file.end()) {
			return false;
		}
	}
	return true;
}

/**
 * Find correct file when headers accept-charset or accept-language are present
 **/
std::map<std::string, std::string> Server::getMatchedFile(Resource const &resource, Request const &request) {
	std::map<std::string, std::string> ret;
	std::string temp;

	DIR *dirp = opendir(resource.folder().c_str());
	if (dirp == NULL)
		return ret;
	struct dirent *dp;
	std::vector<std::string> splitResource = split(resource.fullFilename(), ".");
	std::list<std::vector<std::string> > files;
	while ((dp = readdir(dirp)) != NULL) {
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..") || dp->d_name[0] == '.')
			continue;
		if (matchUri(splitResource, split(std::string(dp->d_name), ".")))
			files.push_back(split(std::string(dp->d_name), "."));
	}
	closedir(dirp);

	if (request.acceptCharset().size() > 0) {
		files = matchCharset(files, request.acceptCharset(), &temp);
		ret.insert(std::make_pair("charset", temp));
	}

	if (request.acceptLanguage().size() > 0) {
		files = matchLanguage(files, request.acceptLanguage(), &temp);
		ret.insert(std::make_pair("language", temp));
		if (files.size() == 0) {
			ret.insert(std::make_pair("uri", resource.relativePath()));
		}
	}

	if (files.size() == 0)
		return ret;

	files.sort(nbrWords);
	ret.insert(std::make_pair("uri", join(files.front(), ".")));
	return ret;
}

/**
 * Return true if file is to be executed
 **/
bool Server::isCgiScript(Request const &request) {
	Resource resource(request.conf().root, request.getRelUri());

	// Logger::debug("filename<", resource.filename(), "> extension<", resource.extension(), ">");
	for (const auto &e : request.conf().cgiExtensions) {
		// Logger::debug("[", e.first, "]");
		if (resource.extension() == e.first)
			return true;
	}
	return false;
}

void Server::autoIndex(Request const &request, Response &res) {
	Resource resource(request.conf().root, request.getRelUri());

	DIR *dirp = opendir(resource.absolutePath().c_str());
	if (dirp == NULL) {
		setResponseError(request, res, RFC::InternalServerError);
		return ;
	}
	struct dirent *dp;
	std::list<std::string> files;
	while ((dp = readdir(dirp)) != NULL) {
		std::string dName(dp->d_name);
		if (dName.front() != '.' || dName == "." || dName == "..")
			files.push_back(std::string(dp->d_name));
	}
	closedir(dirp);
	std::string strBody;
strBody.append("<!DOCTYPE html>\n\
<html lang=\"en\">\n\
	<head>\n\
		<meta charset=\"utf-8\" />\n\
		<title>Index</title>\n\
	<head>\n\
	<body>\n\
		<h1>Index</h1>\n\
		<ul>\n");
	for (const auto &f : files) {
		strBody.append("\t\t\t<li><a href=\"/" + resource.folder(f) + "\">" + f + "</a></li>\n");
	}
strBody.append("\
		</ul>\n\
	</body>\n\
</html>");
	res.setStatus(RFC::Ok);
	res.addHeader("Content-Type", "text/html");
	res.setBody(strBody);
}
