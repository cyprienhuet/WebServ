#ifndef CLIENTSET_HPP
# define CLIENTSET_HPP

# include <vector>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <unistd.h>
# include "openssl/ssl.h"
# include "openssl/err.h"
# include "Client.hpp"
# include "Logger.hpp"

class ClientSet {
private:
	size_t			maxSize;
	unsigned int	timeToLive;
	Client*			array;
	Client*			end;
public:
	ClientSet();
	virtual ~ClientSet();

	void init(size_t size, unsigned int TTL);
	Client* findEmpty() const;
	Client* begin() const;
	Client* next(Client* it) const;

	void addClient(std::pair<int, std::string> const &newClient);
	void addClient(std::pair<int, std::string> const &newClient, SSL_CTX* ctx);
	void updateEnd();
	int addToFdSet(fd_set* read, fd_set* write);
	void removeTimedOut();
};

#endif /* CLIENTSET_HPP */
