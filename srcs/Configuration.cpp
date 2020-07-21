#include "Configuration.hpp"

std::vector<std::string> Configuration::createValidKeys() {
	std::vector<std::string> validKeys = BaseConfiguration::validKeys;
	validKeys.push_back("workers");
	validKeys.push_back("maxClients");
	validKeys.push_back("timeToLive");
	return (validKeys);
}
std::vector<std::string> Configuration::validKeys = Configuration::createValidKeys();

Configuration::Configuration():
	BaseConfiguration(),
	workers(5), maxClients(256), timeToLive(30), servers() {}

Configuration::Configuration(Configuration const &other) {
	this->copy(other);
}

Configuration::~Configuration() {}

Configuration &Configuration::operator=(Configuration const &other) {
	this->copy(other);
	return (*this);
}

void Configuration::loadDefault(void) {
	this->servers.push_back(ServerConfiguration(*this));
}

Configuration::ParsingException::ParsingException(std::string const &str, int line):
	str(str) {
	this->str.insert(0, "Configuration error: Line " + to_string(line) + ": ");
}
const char*	Configuration::ParsingException::what() const throw() { return this->str.c_str(); }

/**
 * Configurations files kind of follow NGINX format
 * See conf/default.conf for an example
 **/
bool Configuration::loadFromFile(std::string const &filename) {
	Logger::debug("Loading configuration file ", filename);
	if (filename.length() == 0) {
		Logger::error("Empty configuration filename");
		return (false);
	}
	std::ifstream infile(filename);
	if (!infile.is_open() || !infile.good()) {
		Logger::error("Invalid configuration file ", filename);
		infile.close();
		return (false);
	}
	std::string sadStream;
	std::string buffer;
	while (std::getline(infile, buffer)) {
		sadStream += buffer;
		sadStream += '\n';
	}
	infile.close();
	this->parseConfigurationBuffer(sadStream);
	return (true);
}

bool Configuration::isValidKey(std::string const &key) {
	return (std::find(Configuration::validKeys.begin(), Configuration::validKeys.end(), key)
			!= Configuration::validKeys.end());
}

bool Configuration::isValidKey(Context context, std::string const &key) {
	if (context == Context::Location)
		return (BaseConfiguration::isValidKey(key));
	else if (context == Context::Server)
		return (ServerConfiguration::isValidKey(key));
	return (Configuration::isValidKey(key));
}

bool Configuration::setParameter(std::string const &key, std::string const &value) {
	bool ret = this->BaseConfiguration::setParameter(key, value);
	if (ret)
		return (ret);
	if (key == "workers")
		this->workers = this->parameterAsInt(value);
	else if (key == "maxClients")
		this->maxClients = this->parameterAsInt(value);
	else if (key == "timeToLive")
		this->timeToLive = this->parameterAsInt(value);
	return (ret);
}

bool Configuration::processKeyValue(Context context, std::string const &key, std::string const &value) {
	bool ret = false;
	if (context == Context::Location)
		ret = this->servers.back().locations.back().setParameter(key, value);
	else if (context == Context::Server)
		ret = this->servers.back().setParameter(key, value);
	else
		ret = this->setParameter(key, value);
	return (true);
}

static std::string::const_iterator skipSpaces(std::string::const_iterator begin, std::string::const_iterator end, size_t *line) {
	while (begin != end && (RFC::isRFCSpace(*begin) || *begin == '\n')) {
		if (*begin == '\n')
			++line;
		++begin;
	}
	return (begin);
}

bool Configuration::parseConfigurationBuffer(std::string const &buffer) {
	// Parsing state
	Context context = Global;
	std::map<Context, std::set<std::string> > contextKeys;
	size_t line = 1;
	// Go...
	std::string::const_iterator it = buffer.begin();
	std::string::const_iterator ite = buffer.end();
	// Loop through each chars
	while (it != ite) {
		// Skip Comments
		if (*it == '\n') {
			++line;
			++it;
		} else {
			// Skip tabs and spaces at the beginning
			it = skipSpaces(it, ite, &line);
			// Get the key
			std::string key;
			while (it != ite && !RFC::isRFCSpace(*it) && *it != '\n') {
				key.append(1, *it);
				++it;
			}
			// Remove single line comment
			size_t singleLineComment = key.find("//");
			if (singleLineComment != std::string::npos) {
				key.erase(singleLineComment);
				while (it != ite && *it != '\n')
					++it;
				++it;
				++line;
				if (singleLineComment == 0)
					continue ;
			}
			if (key.length() == 0) {
				if (it != ite)
					++it;
				continue ;
			// Check if context change
			} else if (key.find("/*") == 0) {
				int previous = 0;
				while (it != ite && (previous != '*' || *it != '/')) {
					if (*it == '\n')
						++line;
					previous = *it;
					++it;
				}
				++it;
				previous = 0;
			} else if (key == "}") {
				if (context == Context::Global)
					throw ParsingException("Bad Context", line);
				context = (context == Context::Server) ? Context::Global : Context::Server;
			} else if (key == "server") {
				if (context != Context::Global)
					throw ParsingException("Bad Context", line);
				it = skipSpaces(it, ite, &line);
				if (it == ite || *it != '{')
					throw ParsingException("Missing context opening", line);
				context = Context::Server;
				contextKeys[context].clear();
				this->servers.push_back(ServerConfiguration(*this));
				++it;
			// Get the key value if it's valid
			} else {
				bool isLocation = (key == "location");
				if (it != ite && *it != ' ' && *it != '\t' && *it != '\n')
					throw ParsingException("Unexpected character after key", line);
				else if (isLocation && context != Context::Server)
					throw ParsingException("Bad Context", line);
				else if (!isLocation && !this->isValidKey(context, key))
					throw ParsingException("Invalid key name", line);
				else if (!isLocation
						&& key != "error" && key != "cgiExtension" && key != "user"
						&& contextKeys[context].find(key) != contextKeys[context].end())
					throw ParsingException("Duplicate key", line);
				if ((it = skipSpaces(it, ite, &line)) == ite)
					throw ParsingException("Missing value after key", line);
				contextKeys[context].insert(key);
				// Get value
				std::string value;
				bool inQuotes = false;
				while (it != ite && (inQuotes || (*it != ';' && (!isLocation || (!RFC::isRFCSpace(*it) && *it != '\n'))))) {
					if (*it == '\n')
						++line;
					else if (*it == '"')
						inQuotes = !inQuotes;
					value.append(1, *it);
					++it;
				}
				if (inQuotes)
					throw ParsingException("Missing \"", line);
				else if (it == ite)
					throw ParsingException("Missing ;", line);
				if (isLocation) {
					it = skipSpaces(it, ite, &line);
					if (it == ite || *it != '{')
						throw ParsingException("Missing context opening", line);
					context = Context::Location;
					contextKeys[context].clear();
					this->servers.back().locations.push_back(ServerConfiguration(this->servers.back(), value));
				} else this->processKeyValue(context, key, value);
				if (it != ite)
					++it;
			}
		}
	}
	if (context != Context::Global)
		throw ParsingException("Bad context", line);
	return (true);
}

void Configuration::copy(Configuration const &other) {
	this->BaseConfiguration::copy(other);
	this->workers = other.workers;
	this->maxClients = other.maxClients;
	this->timeToLive = other.timeToLive;
	this->servers = other.servers;
}

void Configuration::check(void) const {
	// Check if there is at least 1 server
	if (this->servers.size() < 1)
		throw ValidationException("Webserv: No Server definition");
	else if (this->workers <= 0)
		throw ValidationException("Webserv.workers: must be > 0");
	else if (this->maxClients <= 0 || this->maxClients > FD_SETSIZE)
		throw ValidationException("Webserv.maxClients: must be more than 0 and less than " + to_string(FD_SETSIZE));
	else if (this->timeToLive <= 0)
		throw ValidationException("Webserv.maxClients: must be between > 1");
	// Check if each locations are valid
	std::set<std::string> serverNames;
	for (auto it = this->servers.begin(), ite = this->servers.end(); it != ite; ++it) {
		for (auto const &serverName : (*it).server_name) {
			if (serverNames.find(serverName) != serverNames.end())
				throw ValidationException("Webserv.server_name: duplicate server_name");
			serverNames.insert(serverName);
		}
		(*it).check();
	}
}

void Configuration::display(void) const {
	Logger::debug("workers: ", this->workers);
	Logger::debug("maxClients: ", this->maxClients);
	Logger::debug("timeToLive: ", this->timeToLive);
	this->BaseConfiguration::display();
	Logger::debug("Got ", this->servers.size(), " server", ((this->servers.size() > 1) ? "s" : ""));
	auto it = this->servers.begin();
	auto ite = this->servers.end();
	while (it != ite)
		(*it++).display();
}
