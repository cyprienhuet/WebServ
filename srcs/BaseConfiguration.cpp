#include "BaseConfiguration.hpp"

std::vector<std::string> BaseConfiguration::createValidKeys() {
	std::vector<std::string> validKeys(18);
	validKeys.push_back("root");
	validKeys.push_back("cgi");
	validKeys.push_back("cgiExtension");
	validKeys.push_back("gzip");
	validKeys.push_back("logAccess");
	validKeys.push_back("accessLogFile");
	validKeys.push_back("logErrors");
	validKeys.push_back("errorLogFile");
	validKeys.push_back("autoindex");
	validKeys.push_back("allowFileUpload");
	validKeys.push_back("uploadedFileLocation");
	validKeys.push_back("index");
	validKeys.push_back("error");
	validKeys.push_back("user");
	validKeys.push_back("authBasic");
	validKeys.push_back("accept");
	validKeys.push_back("deny");
	validKeys.push_back("maxBodySize");
	return (validKeys);
}
std::vector<std::string> BaseConfiguration::validKeys = BaseConfiguration::createValidKeys();

BaseConfiguration::BaseConfiguration():
	root("www"), cgi(false), gzip(true),
	logAccess(false), accessLogFile("logs/access.log"),
	logErrors(false), errorLogFile("logs/error.log"),
	autoindex(true), allowFileUpload(true), uploadedFileLocation("tmp"),
	errorPages(), indexPages(),
	authBasic(false), users(), acceptedMethods(), cgiExtensions(),
	maxBodySize(100 * 1000 * 1000)
{
	this->indexPages.push_back("index.html");
	this->indexPages.push_back("index.htm");
	this->errorPages[RFC::BadRequest] = "errors/400.html";
	this->errorPages[RFC::Forbidden] = "errors/403.html";
	this->errorPages[RFC::NotFound] = "errors/404.html";
	this->errorPages[RFC::InternalServerError] = "errors/500.html";
	this->errorPages[RFC::BadGateway] = "errors/502.html";
	this->errorPages[RFC::ServiceUnavailable] = "errors/503.html";
	this->acceptedMethods[RFC::OPTIONS] = true;
	this->acceptedMethods[RFC::GET] = true;
	this->acceptedMethods[RFC::HEAD] = true;
	this->acceptedMethods[RFC::POST] = true;
	this->acceptedMethods[RFC::PUT] = true;
	this->acceptedMethods[RFC::DELETE] = true;
	this->acceptedMethods[RFC::TRACE] = false;
	this->acceptedMethods[RFC::CONNECT] = false;
}

BaseConfiguration::BaseConfiguration(BaseConfiguration const &other) {
	this->copy(other);
}

BaseConfiguration::~BaseConfiguration() {}

BaseConfiguration &BaseConfiguration::operator=(BaseConfiguration const &other) {
	this->copy(other);
	return (*this);
}

bool BaseConfiguration::isValidKey(std::string const &key) {
	return (std::find(BaseConfiguration::validKeys.begin(), BaseConfiguration::validKeys.end(), key)
			!= BaseConfiguration::validKeys.end());
}

bool BaseConfiguration::parseErrorFile(std::string const &value) {
	std::vector<std::string> const &params = this->splitParameter(value);
	if (params.size() != 2)
		return (false);
	if (params.front() == "*" || params.front() == "ALL")
		for (size_t i = RFC::BadRequest; i <= RFC::HTTPVersionNotSupported; ++i)
			this->errorPages[i] = params.back();
	else {
		try {
			int statusCode = std::stoi(params.front());
			int i = RFC::BadRequest;
			for ( ; i <= RFC::HTTPVersionNotSupported; ++i)
				if (i == statusCode)
					break ;
			if (i > RFC::HTTPVersionNotSupported)
				return (false);
			this->errorPages[statusCode] = params.back();
			this->asLocalFilePath(this->errorPages[statusCode]);
		} catch(const std::exception& e) {
			return (false);
		}
	}
	return (true);
}

bool BaseConfiguration::parseUser(std::string const &value) {
	std::vector<std::string> const &params = this->splitParameter(value);
	if (params.size() == 0 || params.size() > 2
		|| params.front().length() == 0)
		return (false);
	if (params.size() == 1 || params.back().length() == 0)
		this->users.push_back(RFC::base64_encode(params.front() + ":"));
	else
		this->users.push_back(RFC::base64_encode(params.front() + ":" + params.back()));
	return (true);
}

bool BaseConfiguration::parseCgiExtension(std::string const &value) {
	std::vector<std::string> const &params = this->splitParameter(value);
	if (params.size() == 0 || params.front().length() < 1)
		return (false);
	this->cgiExtensions[params.front()].clear();
	auto ite = params.end();
	for (auto it = ++params.begin(); it != ite; ++it) {
		this->cgiExtensions[params.front()].push_back(*it);
	}
	return (true);
}

bool BaseConfiguration::parseMethods(int acceptDeny, std::string const &value) {
	std::vector<std::string> const &params = this->splitParameter(value);
	auto it = params.begin();
	auto ite = params.end();

	while (it != ite) {
		if (*it == "*" || *it == "ALL") {
			RFC::Method i = RFC::OPTIONS;
			for ( ; i < RFC::INVALID; i = static_cast<RFC::Method>(static_cast<size_t>(i) + 1))
				this->acceptedMethods[i] = acceptDeny;
		} else {
			RFC::Method method = RFC::methodFromString(*it);
			if (method == RFC::INVALID)
				return (false);
			this->acceptedMethods[method] = acceptDeny;
		}
		++it;
	}
	return (true);
}

bool BaseConfiguration::setParameter(std::string const &key, std::string const &value) {
	bool ret = true;
	if (key == "gzip")
		this->gzip = this->parameterAsBoolean(value);
	else if (key == "root") {
		this->root = value;
		this->asLocalFilePath(this->root);
	} else if (key == "cgi")
		this->cgi = this->parameterAsBoolean(value);
	else if (key == "cgiExtension")
		return (this->parseCgiExtension(value));
	else if (key == "logAccess")
		this->logAccess = this->parameterAsBoolean(value);
	else if (key == "accessLogFile") {
		this->accessLogFile = value;
		this->asLocalFilePath(this->accessLogFile);
	} else if (key == "logErrors")
		this->logErrors = this->parameterAsBoolean(value);
	else if (key == "errorLogFile") {
		this->errorLogFile = value;
		this->asLocalFilePath(this->errorLogFile);
	} else if (key == "autoindex")
		this->autoindex = this->parameterAsBoolean(value);
	else if (key == "index") {
		this->indexPages = this->splitParameter(value);
		for (auto &page : this->indexPages) {
			this->asLocalFilePath(page);
		}
	} else if (key == "allowFileUpload")
		this->allowFileUpload = this->parameterAsBoolean(value);
	else if (key == "uploadedFileLocation") {
		this->uploadedFileLocation = value;
		this->asLocalFilePath(this->uploadedFileLocation);
	} else if (key == "error")
		return (this->parseErrorFile(value));
	else if (key == "user")
		return (this->parseUser(value));
	else if (key == "authBasic")
		this->authBasic = this->parameterAsBoolean(value);
	else if (key == "accept")
		return (this->parseMethods(Accept, value));
	else if (key == "deny")
		return (this->parseMethods(Deny, value));
	else if (key == "maxBodySize")
		this->maxBodySize = this->parameterAsSize(value);
	else
		ret = false;
	return (ret);
}

void BaseConfiguration::copy(BaseConfiguration const &other) {
	this->root = other.root;
	this->cgi = other.cgi;
	this->gzip = other.gzip;
	this->logAccess = other.logAccess;
	this->accessLogFile = other.accessLogFile;
	this->logErrors = other.logErrors;
	this->errorLogFile = other.errorLogFile;
	this->autoindex = other.autoindex;
	this->allowFileUpload = other.allowFileUpload;
	this->uploadedFileLocation = other.uploadedFileLocation;
	this->errorPages = other.errorPages;
	this->indexPages = other.indexPages;
	this->authBasic = other.authBasic;
	this->users = other.users;
	this->acceptedMethods = other.acceptedMethods;
	this->cgiExtensions = other.cgiExtensions;
	this->maxBodySize = other.maxBodySize;
}

bool BaseConfiguration::parameterAsBoolean(std::string const &value) const {
	return (value == "true" || value == "on" || value == "enable");
}

int BaseConfiguration::parameterAsInt(std::string const &value) const {
	return (std::stoi(value));
}

int BaseConfiguration::parameterAsSize(std::string const &value) const {
	int size = 0;
	auto it = value.begin();
	auto ite = value.end();
	// Convert size to int
	while (it != ite) {
		if (!std::isdigit(*it))
			break ;
		else
			size = (size * 10) + (*it - '0');
		++it;
	}
	// Skip spaces
	while (it != ite && (*it == ' ' || *it == '\t'))
		++it;
	// Get Unit
	std::string unit;
	while (it != ite) {
		unit.append(1, *it);
		++it;
	}
	if (unit == "G" || unit == "Gb" || unit == "GB")
		return (size * 1000 * 1000 * 1000);
	else if (unit == "M" || unit == "Mb" || unit == "MB")
		return (size * 1000 * 1000);
	else if (unit == "K" || unit == "Kb" || unit == "KB")
		return (size * 1000);
	return (size); // "B", "b", "O", "o"
}

void BaseConfiguration::removeQuotes(std::string &value) const {
	value.erase(std::remove(value.begin(), value.end(), '"'), value.end());
}

std::vector<std::string> BaseConfiguration::splitParameter(std::string const &value) const {
	std::vector<std::string> out;
	out.push_back(std::string());
	bool inQuotes = false;
	auto it = value.begin();
	auto ite = value.end();
	for ( ; it != ite; ++it) {
		if (*it == '"')
			inQuotes = !inQuotes;
		else if (!inQuotes && *it == ' ') {
			if (out.back().length() > 0)
				out.push_back(std::string());
		} else {
			out.back().append(1, *it);
		}
	}
	return (out);
}

void BaseConfiguration::asLocalFilePath(std::string &value) const {
	this->removeQuotes(value);
	if (value.front() == '/' && value != "/") {
		value.erase(0, 1);
	}
}

BaseConfiguration::ValidationException::ValidationException(std::string const &message):
	str(message) {}
const char*	BaseConfiguration::ValidationException::what() const throw() { return this->str.c_str(); }


void BaseConfiguration::display(std::string const &prefix) const {
	Logger::debug(prefix, "cgi: ", ((this->cgi) ? "true" : "false"));
	Logger::debug(prefix, "cgiExtensions:");
	auto eit = this->cgiExtensions.begin();
	auto eite = this->cgiExtensions.end();
	while (eit != eite) {
		Logger::debug(prefix, "  <", (*eit).first, ">(");
		auto argIte = (*eit).second.end();
		for (auto argIt = (*eit).second.begin(); argIt != argIte; ) {
			Logger::debug(*argIt);
			if (++argIt != argIte) {
				Logger::debug(", ");
			}
		}
		Logger::debug(")");
		++eit;
	}
	Logger::debug(prefix, "gzip: ", (this->gzip ? "true" : "false"));
	Logger::debug(prefix, "logAccess: ", (this->logAccess ? "true" : "false"));
	Logger::debug(prefix, "accessLogFile: ", this->accessLogFile);
	Logger::debug(prefix, "logErrors: ", (this->logErrors ? "true" : "false"));
	Logger::debug(prefix, "errorLogFile: ", this->errorLogFile);
	Logger::debug(prefix, "autoindex: ", (this->autoindex ? "true" : "false"));
	Logger::debug(prefix, "allowFileUpload: ", (this->allowFileUpload ? "true" : "false"));
	Logger::debug(prefix, "uploadedFileLocation: ", this->uploadedFileLocation);
	Logger::debug(prefix, "authBasic: ", (this->authBasic ? "true" : "false"));
	Logger::debug(prefix, "users:");
	auto uit = this->users.begin();
	auto uite = this->users.end();
	while (uit != uite) {
		Logger::debug(prefix, "  base64 encoded: ", (*uit));
		++uit;
	}
	Logger::debug(prefix, "errorPages:");
	auto mit = this->errorPages.begin();
	auto mite = this->errorPages.end();
	while (mit != mite) {
		Logger::debug(prefix, "  ", (*mit).first, ": ", (*mit).second);
		++mit;
	}
	Logger::debug(prefix, "indexPages:");
	auto it = this->indexPages.begin();
	auto ite = this->indexPages.end();
	while (it != ite)
		Logger::debug(prefix, "  ", *it++);
	Logger::debug(prefix, "acceptedMethods:");
	auto ait = this->acceptedMethods.begin();
	auto aite = this->acceptedMethods.end();
	while (ait != aite) {
		Logger::debug(prefix, "  ", RFC::MethodString[(*ait).first], ": ", (((*ait).second) ? "ACCEPT" : "DENY"));
		++ait;
	}
}
