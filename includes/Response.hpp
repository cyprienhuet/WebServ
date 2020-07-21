#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <string>
# include <map>
# include <vector>
# include <deque>
# include <fcntl.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <unistd.h>
# include <Request.hpp>
# include <string.h>
# include "MimeTypes.hpp"
# include "RFC.hpp"
# include "zlib.h"
# include "RawBody.hpp"
# include "FileBody.hpp"
# include "Client.hpp"
# include "Resource.hpp"
# include "Logger.hpp"

# define R_BUFFER_SIZE 32768

class Client;
class Request;
class Response {
public:
	enum State {
		NOTHING_SENT,
		HEADER_SENT,
		CHUNKED_SEND_MORE,
		CHUNKED_SEND_MORE_RAW,
		CHUNKED_NEED_END
	};
protected:
	std::string	uri;
	std::string	version;
	RFC::Status	status;
	Body*		body;
	RFC::Encoding	compression;
	std::deque<std::pair<std::string, std::string> > headers;
	z_stream	stream;
	bool		strmOpen;
	State		progress;
	bool		dontSend;
	std::string	unsendBuffer;
	size_t		lastSendSize;
	State		nextInitialProgress;

	Response &operator=(Response const &other);

	int sendUnsend(Client& client);
	bool trySend(Client &client, char const *str, int size);
public:
	Response();
	Response(Response const &other);
	virtual ~Response();

	std::string const&	getURI(void) const;
	std::string const&	getVersion(void) const;
	RFC::Status 		getStatus(void) const;
	Body*				getBody(void) const;
	virtual bool		isBodyReady(void) const;
	RFC::Encoding 		getCompression(void) const;
	bool				getDontSend(void) const;
	std::pair<std::string, std::string> getHeader(std::string const &name) const;

	void setStatus(RFC::Status status);
	void setStatus(int status);
	void setUri(std::string const &uri);
	void setDontSend(bool);
	void setCompression(RFC::Encoding encoding);
	void setBody(Resource const &resource, int fd);
	void setBody(std::string const &strBody);
	void setBody(char const *cbody, size_t size);
	void setBody(Body *body);
	bool removeHeader(std::string const &name);
	bool addHeader(std::string const &name, std::string const &value);

	void outputLine(std::string &out) const;
	void outputHeaders(std::string &out) const;
	void defaultError(RFC::Status status);
	void redirectSSL(int portSSL, const Request& request);
	virtual int send(Client& client);
	void initCompression();

	virtual int getType(void) const;
};

#endif
