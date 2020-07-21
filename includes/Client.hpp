#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <list>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <unistd.h>
# include "openssl/ssl.h"
# include "openssl/err.h"
# include "Request.hpp"
# include "BodyList.hpp"
# include "Response.hpp"
# include "Utility.hpp"
# include "StreamJob.hpp"
# include "Request.hpp"
# include "File.hpp"

class ClientSet;
class Response;
class Request;
class StreamJob;
class StreamBody;

class Client {
private:
	enum ChunkStates {
		NEED_SIZE,
		NEED_R_1,
		NEED_N_1,
		GO_READ,
		NEED_R_2,
		NEED_N_2,
		NEED_R_END,
		NEED_N_END,
		GO_END
	};

	int 	fd;
	bool	ready;
	SSL*	ssl;
	timeval	timestamp;
	Request* request;
	std::string* buffer;
	BodyList* body;
	std::list<Response*> responses;
	std::string ip;
	size_t toRead;
	ChunkStates status;
	StreamJob* job;

	void handleChunked(char* buffer, size_t& offset, size_t ret);
	void handleHeaders(char* buffer, size_t& offset, size_t ret, bool& doRead);
public:
	operator int();
	operator int() const;
	operator timeval();
	operator timeval() const;

	Client();
	Client(int fd);
	Client(int fd, SSL_CTX* ctx);
	virtual ~Client();

	bool isReady(void) const;
	void setReady(bool val);
	void keepAlive(void);
	void pushResponse(Response* resp);
	void insertResponse(Response* resp);
	int read(char* buffer, std::size_t size);
	int send(const char* buffer, std::size_t size);
	bool isSSL(void) const;
	bool hasResponses(void) const;
	void setJob(StreamJob* job);
	Response const *nextResponse(void) const;
	int sendResponse(void);
	Request *returnRequest(void);
	Request *receive(bool& doRead, std::list<StreamJob*>& jobList);
	BodyList* getList() const;
	void deleteBody();
	void setIp(std::string const &ip);
	std::string const &getIp(void) const;

	class SSLException: public std::exception {
	private:
		const char* str;
	public:
		SSLException(const char*);
		virtual ~SSLException() throw() {};
		virtual const char*	what(void) const throw();
	};

	bool skip;
};

unsigned int operator-(const timeval& left, const timeval& rigth);

#endif /* CLIENT_HPP */
