#ifndef CONFIGURATION_HPP
# define CONFIGURATION_HPP

# include <exception>
# include <string>
# include <map>
# include <set>
# include <vector>
# include <list>
# include <sstream>
# include <fstream>
# include "RFC.hpp"
# include "BaseConfiguration.hpp"
# include "ServerConfiguration.hpp"

class Configuration:
	virtual public BaseConfiguration
{
public:
	// Global server configuration
	int workers;
	int maxClients;
	int timeToLive;
	// List of servers with their own ports and server_name
	std::vector<ServerConfiguration> servers;

	Configuration();
	Configuration(Configuration const &other);
	virtual ~Configuration();

	Configuration &operator=(Configuration const &other);

	static bool isValidKey(std::string const &key);

	void loadDefault();

	class ParsingException: public std::exception {
	private:
		std::string str;
	public:
		ParsingException(std::string const &message, int line=-1);
		virtual ~ParsingException() throw() {};
		virtual const char*	what() const throw();
	};
	bool loadFromFile(std::string const &filename);
	virtual void check(void) const;

	void display(void) const;
private:
	enum Context {
		Global,
		Server,
		Location
	};
	static std::vector<std::string>	createValidKeys(void);
	static std::vector<std::string> validKeys;
	static bool isValidKey(Context context, std::string const &key);
	virtual bool setParameter(std::string const &key, std::string const &value);

	void copy(Configuration const &other);
	bool processKeyValue(Context context, std::string const &key, std::string const &value);
	bool parseConfigurationBuffer(std::string const &buffer);
};

#endif
