#ifndef BaseConfiguration_HPP
# define BaseConfiguration_HPP

# include <string>
# include <map>
# include <vector>
# include <list>
# include <sstream>
# include <algorithm>
# include <iostream> // TDOO: Remove debug
# include <cctype>
# include <algorithm>
# include "RFC.hpp"
# include "Logger.hpp"

class BaseConfiguration {
public:
	std::string	root;
	bool		cgi;
	bool		gzip;
	bool 		logAccess;
	std::string	accessLogFile;
	bool 		logErrors;
	std::string	errorLogFile;
	bool		autoindex;
	bool 		allowFileUpload;
	std::string uploadedFileLocation;
	std::map<int, std::string>	errorPages;
	std::vector<std::string>	indexPages;
	bool						authBasic;
	std::vector<std::string>	users;
	std::map<RFC::Method, bool>	acceptedMethods;
	std::map<std::string, std::vector<std::string> >	cgiExtensions;
	int maxBodySize;

	BaseConfiguration();
	BaseConfiguration(BaseConfiguration const &other);
	virtual ~BaseConfiguration();

	BaseConfiguration &operator=(BaseConfiguration const &other);

	static bool isValidKey(std::string const &key);
	virtual bool setParameter(std::string const &key, std::string const &value);

	class ValidationException: public std::exception {
	private:
		std::string str;
	public:
		ValidationException(std::string const &message);
		virtual ~ValidationException() throw() {};
		virtual const char*	what() const throw();
	};
	virtual void check(void) const = 0;

	void display(std::string const &prefix=std::string()) const;
protected:
	static std::vector<std::string>	createValidKeys(void);
	static std::vector<std::string> validKeys;
	void copy(BaseConfiguration const &other);

	bool parseErrorFile(std::string const &value);
	bool parseUser(std::string const &value);
	bool parseCgiExtension(std::string const &value);
	enum AcceptDeny {
		Deny,
		Accept
	};
	bool parseMethods(int acceptDeny, std::string const &value);

	void removeQuotes(std::string &value) const;
	std::vector<std::string> splitParameter(std::string const &value) const;
	bool parameterAsBoolean(std::string const &value) const;
	int parameterAsInt(std::string const &value) const;
	int parameterAsSize(std::string const &value) const;
	void asLocalFilePath(std::string &value) const;
};

#endif
