#ifndef PORTLISTENER_HPP
# define PORTLISTENER_HPP

# include <unistd.h>
# include <sys/socket.h>
# include <sys/select.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <string>
# include <string.h>
# include "openssl/ssl.h"
# include "openssl/err.h"
# include "Utility.hpp"
# include "File.hpp"
# include "Logger.hpp"

class PortListener {
private:
	enum SocketType {
		TCPSocket = 0,
		SSLSocket = 1
	};
	SocketType	type;

	int			socket;
	SSL_CTX		*ctx;
	struct sockaddr_in	address;
	socklen_t			addr_len;
public:
	PortListener();
	virtual ~PortListener();

	class InitException : public std::exception {
		const char* str;
		const char*	what() const throw();
	public:
		InitException(const char*);
	};

	void init(int port);
	void init(int sslPort, std::string const &certPath, std::string const &keyPath);
	void initSSL(std::string const &certPath, std::string const &keyPath);
	void bind(int maxClients);
	void close(void);

	void fdSet(fd_set &set);
	bool fdIsSet(fd_set &set);
	bool isSSL(void) const;
	SSL_CTX *getCtx(void);
	std::pair<int, std::string> accept(void);
	int max(int max_fd) const;
};

#endif
