#include "ServerConfiguration.hpp"

std::vector<std::string> ServerConfiguration::createValidKeys() {
	std::vector<std::string> validKeys = BaseConfiguration::validKeys;
	validKeys.push_back("port");
	validKeys.push_back("server_name");
	validKeys.push_back("root");
	validKeys.push_back("advertiseServer");
	validKeys.push_back("SSL");
	validKeys.push_back("forceSSL");
	return (validKeys);
}
std::vector<std::string> ServerConfiguration::validKeys = ServerConfiguration::createValidKeys();

ServerConfiguration::ServerConfiguration(Configuration const &parent):
	BaseConfiguration(parent), isLocation(false), parentRoot(""), path("/"),
	port(80), advertiseServer(true),
	SSL(false), portSSL(443), forceSSL(true), certPath(), keyPath(),
	server_name() {
	// this->server_name.push_back("localhost");
}

ServerConfiguration::ServerConfiguration(ServerConfiguration const &parent, std::string const &path):
	BaseConfiguration(parent) {
	this->copy(parent);
	this->isLocation = true;
	this->path = path;
	this->parentRoot = parent.root;
	this->removeQuotes(this->path);
	if (this->path.front() != '/') {
		this->path.insert(0, "/");
	}
}

ServerConfiguration::ServerConfiguration(ServerConfiguration const &other) {
	this->copy(other);
}

ServerConfiguration::~ServerConfiguration() {}

ServerConfiguration &ServerConfiguration::operator=(ServerConfiguration const &other) {
	this->copy(other);
	return (*this);
}

bool ServerConfiguration::isValidKey(std::string const &key) {
	return (std::find(ServerConfiguration::validKeys.begin(), ServerConfiguration::validKeys.end(), key)
			!= ServerConfiguration::validKeys.end());
}

void ServerConfiguration::check(void) const {
	if (this->isLocation) {
		if (this->path.length() == 0)
			throw ValidationException(this->server_name[0] + " location must not be empty");
		return ;
	}
	if (this->server_name.size() == 0)
		throw ValidationException("Server must have at least one name");
	else if (this->root.length() == 0)
		throw ValidationException(this->server_name[0] + " no root");
	else if (this->port < 80 || this->port >= 65535)
		throw ValidationException(this->server_name[0] + " port must be between 80 and 65535");
	else if (this->SSL && (this->portSSL < 443 || this->portSSL >= 65535 || this->port == this->portSSL))
		throw ValidationException(this->server_name[0] + " SSL Port must be between 443 and 65535 and different than port");
	else if (this->maxBodySize <= 0)
		throw ValidationException("Webserv.maxBodySize: must be between >= 1 byte");
	for (auto it = this->locations.begin(), ite = this->locations.end(); it != ite; ++it)
		(*it).check();
}

bool ServerConfiguration::parseSSLConfig(std::string const &value) {
	std::vector<std::string> const &params = this->splitParameter(value);
	if (params.size() != 3)
		return (false);
	auto it = params.begin();
	this->SSL = true;
	this->portSSL = std::stoi(*it++);
	this->certPath = (*it++);
	this->keyPath = (*it);
	if (this->certPath.length() == 0 || this->keyPath.length() == 0)
		return (false);
	this->asLocalFilePath(this->certPath);
	this->asLocalFilePath(this->keyPath);
	return (true);
}

bool ServerConfiguration::setParameter(std::string const &key, std::string const &value) {
	bool ret = this->BaseConfiguration::setParameter(key, value);
	if (ret)
		return (ret);
	ret = true;
	if (key == "port")
		this->port = this->parameterAsInt(value);
	else if (key == "server_name") {
		this->server_name = this->splitParameter(value);
	} else if (key == "advertiseServer")
		this->advertiseServer = this->parameterAsBoolean(value);
	else if (key == "SSL")
		return (this->parseSSLConfig(value));
	else if (key == "forceSSL")
		this->forceSSL = this->parameterAsBoolean(value);
	else
		ret = false;
	return (ret);
}

bool ServerConfiguration::matchHost(std::string const &hostname) const {
	auto ite = this->server_name.end();
	for (auto it = this->server_name.begin(); it != ite; ++it)
		if (insensitive_equal(*it, hostname)) // ? insensitive ?
			return (true);
	return (false);
}

bool ServerConfiguration::matchPort(int port) const {
	return (this->port == port || (this->SSL && this->portSSL == port));
}

void ServerConfiguration::copy(ServerConfiguration const &other) {
	this->BaseConfiguration::copy(other);
	this->isLocation = other.isLocation;
	this->path = other.path;
	this->port = other.port;
	this->root = other.root;
	this->parentRoot = other.parentRoot;
	this->server_name = other.server_name;
	this->advertiseServer = other.advertiseServer;
	this->SSL = other.SSL;
	this->portSSL = other.portSSL;
	this->certPath = other.certPath;
	this->keyPath = other.keyPath;
	this->forceSSL = other.forceSSL;
	this->locations = other.locations;
}

void ServerConfiguration::display(void) const {
	std::string prefix = "\t";
	if (this->isLocation) {
		Logger::debug("\t  # Location <", this->path, ">");
		prefix = "\t\t";
	} else {
		Logger::debug("  # Server ", join(this->server_name, ", "));
	}
	Logger::debug(prefix, "port: ", this->port);
	Logger::debug(prefix, "root: ", this->root);
	Logger::debug(prefix, "advertiseServer: ", ((this->advertiseServer) ? "true" : "false"));
	Logger::debug(prefix, "SSL enabled: ", (this->SSL ? "true" : "false"));
	Logger::debug(prefix, "SSL port: ", this->portSSL);
	Logger::debug(prefix, "SSL certificat: ", this->certPath);
	Logger::debug(prefix, "SSL keys: ", this->keyPath);
	Logger::debug(prefix, "forceSSL: ", (this->forceSSL ? "true" : "false"));
	this->BaseConfiguration::display(prefix);
	if (!this->isLocation) {
		Logger::debug("\tGot ", this->locations.size(), " location", ((this->locations.size() > 1) ? "s" : ""));
		auto it = this->locations.begin();
		auto ite = this->locations.end();
		while (it != ite)
			(*it++).display();
	}
}
