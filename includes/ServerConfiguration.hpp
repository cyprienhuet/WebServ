#ifndef SERVERCONFIGURATION_HPP
# define SERVERCONFIGURATION_HPP

# include <string>
# include <map>
# include <vector>
# include "BaseConfiguration.hpp"

class Configuration;

class ServerConfiguration:
	virtual public BaseConfiguration
{
public:
	// Location parameters
	bool		isLocation;
	std::string	parentRoot;
	std::string	path;
	// Server parameters
	int			port;
	bool		advertiseServer;
	bool		SSL;
	int			portSSL;
	bool		forceSSL;
	std::string	certPath;
	std::string	keyPath;
	std::vector<std::string>	server_name;
	// List of LocationConfiguration
	std::vector<ServerConfiguration> locations;

	ServerConfiguration(Configuration const &parent);
	ServerConfiguration(ServerConfiguration const &parent, std::string const &path);
	ServerConfiguration(ServerConfiguration const &other);
	virtual ~ServerConfiguration();

	ServerConfiguration &operator=(ServerConfiguration const &other);

	static bool isValidKey(std::string const &key);
	bool parseSSLConfig(std::string const &value);
	virtual bool setParameter(std::string const &key, std::string const &value);
	virtual void check(void) const;

	bool matchHost(std::string const &hostname) const;
	bool matchPort(int port) const;

	void display(void) const;
private:
	static std::vector<std::string>	createValidKeys(void);
	static std::vector<std::string> validKeys;

	void copy(ServerConfiguration const &other);
};

# include "Configuration.hpp"

#endif
