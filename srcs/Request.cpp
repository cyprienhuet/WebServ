#include "Request.hpp"
#include <iostream>

/**
 * Create a Request Object with all properties defined
 * Headers in .headers with default values or the parsed value from .addHeader
 * Non-RFC headers are in a list of (raw) string in .otherHeaders
 **/
Request::Request(std::string const &buffer, bool isSSL):
	uri(), relUri(), fileUri(), version(), method(RFC::INVALID), stringMethod(),
	isSSL(isSSL),
	validRequest(true), valid(),
	contentLengthHeader(0), hostHeader(), contentTypeHeader(),
	allowHeader(), acceptCharsetHeader(), acceptEncodingHeader(),
	acceptLanguageHeader(), authorizationHeader(), headers(), validHeader(true),
	confPointer(nullptr), streamBody(nullptr) {
	size_t offset = 0;
	if (buffer.length() == 0)
		return ;
	this->valid.first = true;
	this->valid.second = true;
	// Parse RequestLine
	size_t pos = buffer.find(RFC::CRLF);
	this->defaultHeaders();
	this->parseRequestLine(buffer.substr(0, pos));
	// Advance buffer
	offset = pos + 2;
	pos = buffer.find(RFC::CRLF + RFC::CRLF, offset);
	// Parse Headers
	while ((pos = buffer.find(RFC::CRLF, offset)) > offset && pos != std::string::npos) {
		std::string const &line = buffer.substr(offset, pos - offset);
		if (line.length() > 0)
			this->addHeader(line);
		offset = pos + 2;
	}
}

Request::~Request() {
	delete this->streamBody;
	this->streamBody = nullptr;
}

void Request::defaultHeaders(void) {
	this->contentLengthHeader = 0;
	this->hostHeader.port = 80;
}

void Request::setDefaultHost(ServerConfiguration const &configuration) {
	this->hostHeader.name = configuration.server_name[0];
	if (this->isSSL)
		this->hostHeader.port = configuration.portSSL;
	else
		this->hostHeader.port = configuration.port;
}

// (method)(,\s*(method))*
void Request::parseAllowHeader(std::string const &value) {
	std::vector<std::string> methods = split(value, ",");
	auto it = methods.begin();
	auto ite = methods.end();
	for ( ; it != ite; ++it) {
		trim(*it);
		RFC::Method method = RFC::methodFromString(*it);
		if (method != RFC::INVALID)
			this->allowHeader.push_back(method);
	}
}

// (mimeType)(;\s*charset=(charset))?
void Request::parseContentTypeHeader(std::string const &value) {
	this->contentTypeHeader.mediaType = value;
	size_t semiPos = value.find(";");
	size_t cPos = value.find("charset=", semiPos);
	bool doBreak = false;
	if (semiPos != 0 && semiPos != std::string::npos
		&& cPos != 0 && cPos != std::string::npos) {
		for (size_t i = semiPos + 1; !doBreak && i < cPos; i++)
			if (value[i] != RFC::SP[0] && value[i] != RFC::TAB[0])
				doBreak = true;
		if (doBreak)
			return ;
		this->contentTypeHeader.charset = value.substr(cPos + 8);
		this->contentTypeHeader.mediaType.erase(semiPos);
		trim(this->contentTypeHeader.mediaType);
	}
}

// accept = (type(;\s*q=(quality))?)(,accept)*
void Request::parseAcceptHeader(RFC::Header header, std::string const &value) {
	std::vector<AcceptQuality> &c =
		(header == RFC::AcceptEncoding) ? this->acceptEncodingHeader :
		(header == RFC::AcceptCharset) ? this->acceptCharsetHeader : this->acceptLanguageHeader;

	std::vector<std::string> methods = split(value, ",");
	auto it = methods.begin();
	auto ite = methods.end();
	for ( ; it != ite; ++it) {
		trim(*it);
		AcceptQuality tmp;
		tmp.value = *it;
		tmp.q = 1;
		// Find quality if there is
		size_t semiPos = it->find(";");
		size_t qPos = it->find("q=", semiPos);
		bool doBreak = false;
		if (semiPos != 0 && semiPos != std::string::npos
			&& qPos != 0 && qPos != std::string::npos) {
			for (size_t i = semiPos + 1; !doBreak && i < qPos; i++) {
				if ((*it)[i] != RFC::SP[0] && (*it)[i] != RFC::TAB[0])
					doBreak = true;
			}
			if (doBreak)
				break ;
			try {
				tmp.q = std::stof(it->substr(qPos + 2));
			} catch(const std::exception& e) {
				break ;
			}
			tmp.value.erase(semiPos);
			trim(tmp.value);
		}
		// Sorted insert
		auto insertIt = c.begin();
		auto insertIte = c.end();
		for ( ; insertIt != insertIte; ++insertIt) {
			if (tmp.q > (*insertIt).q) {
				c.insert(insertIt, tmp);
				break ;
			}
		}
		if (insertIt == insertIte)
			c.push_back(tmp);
	}
}

// (host)(:(port))?
void Request::parseHostHeader(std::string const &value) {
	size_t pos = value.find(":");
	if (pos != 0 && pos != std::string::npos) {
		this->hostHeader.name = value.substr(0, pos);
		try {
			this->hostHeader.port = std::stoi(value.substr(pos + 1));
		} catch(const std::exception& e) {
			this->hostHeader.port = 80;
		}
		if (this->hostHeader.port <= 0 || this->hostHeader.port >= 65535) {
			this->hostHeader.port = 80;
			return ;
		}
	} else {
		this->hostHeader.name = value;
	}
}

// (type)\s(credentials)
void Request::parseAuthorization(std::string const &value) {
	size_t pos = value.find(" ");
	if (pos == 0 || pos == std::string::npos)
		return ;
	this->authorizationHeader.first = value.substr(0, pos++);
	this->authorizationHeader.second = value.substr(pos);
}

/**
 * Parse an header "{name}: {value}" from a buffer.
 * If the first char is not alpha, line is skipped (https://tools.ietf.org/html/rfc7230#section-3)
 * If there is whitespace or something else before the colon
 * 	validHeader is set to false (https://tools.ietf.org/html/rfc7230#section-3.2.4)
 * Values of some headers are parsed and set to their attributes if needed.
 * Invalid values in parsed headers are ignored (nginx)
 * Invalid but not parsed headers are considered valid.
 **/
void Request::addHeader(std::string const &line) {
	if (!std::isalpha(line.front()))
		return ;
	size_t pos = line.find(':');
	if (pos == 0 || pos == std::string::npos)
		return ;
	if (!std::isalpha(line[pos - 1])) {
		this->validHeader = false;
		return ;
	}
	// Name
	std::string const &name = line.substr(0, pos);
	// Value - ltrim rtrim
	std::string value = line.substr(++pos);
	trim(value);
	// Set value
	RFC::Header type = RFC::headerFromString(name);
	if (type == RFC::Allow)
		this->parseAllowHeader(value);
	else if (type == RFC::ContentLength) {
		try {
			this->contentLengthHeader = std::stoi(value);
		} catch(const std::exception& e) {
			this->contentLengthHeader = -1;
			this->validHeader = false;
		}
	} else if (type == RFC::ContentType)
		this->parseContentTypeHeader(value);
	else if (type == RFC::AcceptCharset)
		this->parseAcceptHeader(RFC::AcceptCharset, value);
	else if (type == RFC::AcceptEncoding)
		this->parseAcceptHeader(RFC::AcceptEncoding, value);
	else if (type == RFC::AcceptLanguage)
		this->parseAcceptHeader(RFC::AcceptLanguage, value);
	else if (type == RFC::Authorization)
		this->parseAuthorization(value);
	else if (type == RFC::Host) {
		if (this->hostHeader.name.length() > 0)
			this->validHeader = false; // Return BadRequest if there is multiple Host header
		this->parseHostHeader(value);
	} else
		this->headers[name] = value;
}

/**
 * "{method} {uri} {version}"
 * OPTIONS requests can have an empty URI, it's set to "/" by default
 * There MUST be a single space between attributes
 **/
void Request::parseRequestLine(std::string line) {
	if (line.front() == '\n')
		line = line.substr(1);
	std::vector<std::string> params = split(line, " ");
	if (params.size() == 2) {
		params.insert(++params.begin(), "/"); // OPTIONS empty URI
	} else if (params.size() != 3) {
		this->validRequest = false;
		return ;
	}
	this->stringMethod = params[0];
	this->uri = params[1];
	this->version = params[2];
	// Method
	RFC::Method method = RFC::methodFromString(this->stringMethod);
	if (method == RFC::INVALID || method == RFC::CONNECT || method == RFC::TRACE) {
		this->valid.first = false;
	}
	this->method = method;
	if (this->uri.find("://") != std::string::npos) {
		if (this->isSSL && this->uri.find("https:") == std::string::npos)
			this->validRequest = false;
		if (!this->isSSL && this->uri.find("http:") == std::string::npos)
			this->validRequest = false;
		this->uri = this->uri.substr(this->uri.find("://") + 3);
		if (this->uri.find(":") != std::string::npos) {
			this->hostHeader.port = std::stoi(this->uri.substr(this->uri.find(":") + 1));
			this->hostHeader.name = this->uri.substr(0, this->uri.find(":"));
		} else {
			this->hostHeader.name = this->uri.substr(0, this->uri.find("/"));
		}
		this->uri = this->uri.substr(this->uri.find("/"));
	}
	// Decode
	RFC::decodeURIComponent(this->uri);
	// Remove Last /
	if (this->uri.size() > 1 && this->uri.back() == '/')
		this->uri.pop_back();
	if (this->uri.length() == 0)
		return ;
	// Add first /
	if (this->uri == "*") {
		this->uri = "/";
	} else if (this->uri.front() != '/') {
		this->uri.insert(0, "/");
	}
	if (this->version != "HTTP/1.1" && this->version != "HTTP/1.0") {
		this->valid.second = false;
	}
}

std::string const &Request::getUri(void) const {
	return (this->uri);
}

std::string const &Request::getRelUri(void) const {
	return (this->relUri);
}

std::string const &Request::getFileUri(void) const {
	return (this->fileUri);
}

std::string const &Request::getVersion(void) const {
	return (this->version);
}

RFC::Method Request::getMethod(void) const {
	return (this->method);
}

std::string const &Request::getStringMethod(void) const {
	return (this->stringMethod);
}

bool Request::getIsSSL(void) const {
	return (this->isSSL);
}

bool Request::hasHeader(std::string const &name) const {
	auto ite = this->headers.end();
	for (auto it = this->headers.begin(); it != ite; ++it)
		if (insensitive_equal((*it).first, name))
			return (true);
	return (false);
}

std::string const &Request::header(std::string const &name) const {
	static std::string const &boii = std::string();
	auto ite = this->headers.end();
	for (auto it = this->headers.begin(); it != ite; ++it)
		if (insensitive_equal((*it).first, name))
			return ((*it).second);
	return (boii);
}

std::map<std::string, std::string> const &Request::getHeaders(void) const {
	return (this->headers);
}

void Request::removeHeader(std::string const &name) {
	auto ite = this->headers.end();
	for (auto it = this->headers.begin(); it != ite; ++it)
		if (insensitive_equal((*it).first, name)) {
			this->headers.erase(it);
			return ;
		}
}

size_t const &Request::contentLength(void) const {
	return (this->contentLengthHeader);
}

Host const &Request::host(void) const {
	return (this->hostHeader);
}

ContentType const &Request::contentType(void) const {
	return (this->contentTypeHeader);
}

std::vector<RFC::Method> const &Request::allow(void) const {
	return (this->allowHeader);
}

std::vector<AcceptQuality> const &Request::acceptCharset(void) const {
	return (this->acceptCharsetHeader);
}

std::vector<AcceptQuality> const &Request::acceptEncoding(void) const {
	return (this->acceptEncodingHeader);
}

std::vector<AcceptQuality> const &Request::acceptLanguage(void) const {
	return (this->acceptLanguageHeader);
}

std::pair<std::string, std::string> const &Request::authorization(void) const {
	return (this->authorizationHeader);
}

bool Request::hasValidRequest(void) const {
	return (this->validRequest);
}

bool Request::hasValidHeaders(void) const {
	return (this->validHeader);
}

bool Request::hasValidMethod(void) const {
	return (this->valid.first);
}

bool Request::hasValidVersion(void) const {
	return (this->valid.second);
}

RFC::Encoding Request::getAcceptedCompression(void) const {
	RFC::Encoding cmp = RFC::NONE;
	auto it = this->acceptEncodingHeader.begin();
	auto end = this->acceptEncodingHeader.end();
	if (it != end && it->value == "*")
		cmp = RFC::DEFLATE;
	else
		for ( ; it != end; it++)
			if ((cmp = (it->value == "gzip")	? RFC::GZIP		: RFC::NONE) ||
				(cmp = (it->value == "deflate") ? RFC::DEFLATE	: RFC::NONE))
				break;
	return (cmp);
}

void Request::setConf(ServerConfiguration const *conf) {
	this->confPointer = conf;
	this->findRelUri();
}

void Request::findRelUri(void) {
	// Relative to root URI
	if (this->confPointer->isLocation
		&& this->confPointer->parentRoot != this->confPointer->root
		&& this->confPointer->path.length() > 1)
		this->relUri = this->uri.substr(this->confPointer->path.length());
	else
		this->relUri = this->uri;
	if (this->relUri.length() == 0)
		this->relUri = "/";
	// File URI
	if (this->confPointer->isLocation && this->confPointer->path.length() > 1) {
		size_t pos = this->fileUri.find(this->confPointer->path);
		if (pos == 0)
			this->fileUri.erase(0, this->confPointer->path.length());
	}
}

ServerConfiguration const &Request::conf(void) const {
	return (*this->confPointer);
}

void Request::setUri(std::string uri) {
	this->uri = uri;
}
