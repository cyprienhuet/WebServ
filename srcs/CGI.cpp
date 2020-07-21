#include "CGI.hpp"

cgi::cgi(Client const &client, Resource const &resource, Request const &request) {
	if (request.authorization().first.size()) {
		this->env.insert(std::make_pair("AUTH_TYPE", request.authorization().first));
		this->env.insert(std::make_pair("REMOTE_USER", RFC::base64_decode(request.authorization().second).substr(0, RFC::base64_decode(request.authorization().second).find(':'))));
	}

	this->env.insert(std::make_pair("CONTENT_LENGTH", to_string(request.contentLength())));
	if (request.contentType().mediaType.size()) {
		this->env.insert(std::make_pair("CONTENT_TYPE", request.contentType().mediaType + (request.contentType().charset.size() ? "; charset=" + request.contentType().charset : "" )));
	}

	this->env.insert(std::make_pair("DOCUMENT_ROOT", resource.absoluteRoot()));
	this->env.insert(std::make_pair("REQUEST_URI", resource.relativePath(resource.pathInfo())));
	// * this->env.insert(std::make_pair("REQUEST_URI", resource.relativePath()); // But we can't :)
	this->env.insert(std::make_pair("PATH_INFO", resource.relativePath(resource.pathInfo())));
	// * this->env.insert(std::make_pair("PATH_INFO", resource.pathInfo())); // But we can't :)
	this->env.insert(std::make_pair("PATH_TRANSLATED", resource.absolutePath(resource.pathInfo())));
	this->env.insert(std::make_pair("QUERY_STRING", resource.query()));
	this->env.insert(std::make_pair("REQUEST_METHOD", RFC::MethodString[request.getMethod()]));
	this->env.insert(std::make_pair("SCRIPT_FILENAME", resource.absolutePath()));
	this->env.insert(std::make_pair("SCRIPT_NAME", resource.fullFilename()));
	this->env.insert(std::make_pair("SERVER_NAME", request.conf().server_name[0]));
	if (request.getIsSSL()) {
		this->env.insert(std::make_pair("SERVER_PORT", to_string(request.conf().portSSL)));
	} else {
		this->env.insert(std::make_pair("SERVER_PORT", to_string(request.conf().port)));
	}
	this->env.insert(std::make_pair("GATEWAY_INTERFACE", "CGI/1.1"));
	this->env.insert(std::make_pair("SERVER_PROTOCOL", "HTTP/1.1"));
	this->env.insert(std::make_pair("SERVER_SOFTWARE", "webserv/1.0"));
	this->env.insert(std::make_pair("REDIRECT_STATUS", "200"));

	std::vector<std::string> const &ipPort = split(client.getIp(), ":");
	this->env.insert(std::make_pair("REMOTE_ADDR", ipPort[0]));
	this->env.insert(std::make_pair("REMOTE_PORT", ipPort[1]));
}

cgi::~cgi() {}

/**
 * Nice-Heade-You-Got-There: thanks
 * -> "HTTP_NICE_HEADER_YOU_GOT_THERE=thanks"
 **/
void cgi::addHeader(std::pair<std::string, std::string> const &header) {
	std::string upperName = "HTTP_";
	for (size_t i = 0; i < header.first.length(); ++i) {
		if (header.first[i] == '-')
			upperName.append(1, '_');
		else
			upperName.append(1, std::toupper(header.first[i]));
	}
	this->env.insert(std::make_pair(upperName, header.second));
}

char **cgi::getEnv() const {
	char **envp = new char*[this->env.size() + 1];
	if (!envp)
		return NULL;
	int i = 0;
	for (const auto &pair : this->env) {
		envp[i++] = ft_strdup((pair.first + "=" + pair.second).c_str());
		// Logger::debug(pair.first, "=", pair.second);
	}
	envp[i] = 0;
	return (envp);
}

std::string cgi::getEnv(const std::string &key) const {
	return this->env.at(key);
}
