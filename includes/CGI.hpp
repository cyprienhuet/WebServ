#ifndef CGI_HPP
# define CGI_HPP

# include <map>
# include <string>
# include <iostream>
# include "HeaderList.hpp"
# include "Client.hpp"
# include "Request.hpp"
# include "Resource.hpp"

class cgi {
private:
	std::map<std::string, std::string> env;
public:
	cgi(Client const &client, Resource const &resource, Request const &request);
	virtual ~cgi();

	void addHeader(std::pair<std::string, std::string> const &header);
	std::string getEnv(const std::string &key) const;
	char **getEnv() const;
};

#endif
