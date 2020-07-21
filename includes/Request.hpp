#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <map>
# include <iostream>

# include "Utility.hpp"
# include "RFC.hpp"
# include "HeaderList.hpp"
# include "MimeTypes.hpp"
# include "ServerConfiguration.hpp"
# include "StreamBody.hpp"

class StreamBody;

class Request {
protected:
	std::string	uri;
	std::string	relUri;
	std::string	fileUri;
	std::string	version;
	RFC::Method	method;
	std::string	stringMethod;
	bool		isSSL;
	bool		validRequest;
	std::pair<bool, bool>	valid;

	size_t		contentLengthHeader;
	Host		hostHeader;
	ContentType	contentTypeHeader;
	std::vector<RFC::Method>	allowHeader;
	std::vector<AcceptQuality>	acceptCharsetHeader;
	std::vector<AcceptQuality>	acceptEncodingHeader;
	std::vector<AcceptQuality>	acceptLanguageHeader;
	std::pair<std::string, std::string>	authorizationHeader;
	std::map<std::string, std::string>	headers;
	bool								validHeader;

	ServerConfiguration const *confPointer;

	void defaultHeaders(void);
	void parseRequestLine(std::string line);
	void parseHeaderField(std::string field);
	void parseAllowHeader(std::string const &value);
	void parseContentTypeHeader(std::string const &value);
	void parseAcceptHeader(RFC::Header header, std::string const &value);
	void parseHostHeader(std::string const &value);
	void parseAuthorization(std::string const &value);
	void addHeader(std::string const &line);
	void findRelUri(void);

	Request();
public:
	StreamBody* streamBody;
	Request(std::string const &buffer, bool isSSL);
	virtual ~Request();

	bool hasHeader(std::string const &name) const;
	std::string const &header(std::string const &name) const;
	std::map<std::string, std::string> const &getHeaders(void) const;
	void removeHeader(std::string const &name);
	size_t const &contentLength(void) const;
	Host const &host(void) const;
	ContentType const &contentType(void) const;
	std::vector<RFC::Method> const &allow(void) const;
	std::vector<AcceptQuality> const &acceptCharset(void) const;
	std::vector<AcceptQuality> const &acceptEncoding(void) const;
	std::vector<AcceptQuality> const &acceptLanguage(void) const;
	std::pair<std::string, std::string> const &authorization(void) const;
	void setDefaultHost(ServerConfiguration const &configuration);
	void setConf(ServerConfiguration const *conf);

	std::string const &getUri(void) const;
	std::string const &getRelUri(void) const;
	std::string const &getFileUri(void) const;
	void setUri(std::string);
	std::string const &getVersion(void) const;
	RFC::Method getMethod(void) const;
	std::string const &getStringMethod(void) const;
	bool getIsSSL(void) const;
	RFC::Encoding getAcceptedCompression(void) const;
	ServerConfiguration const &conf(void) const;

	bool isValid(void) const;
	bool hasValidRequest(void) const;
	bool hasValidHeaders(void) const;
	bool hasValidMethod(void) const;
	bool hasValidVersion(void) const;
};

#endif
