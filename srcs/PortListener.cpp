#include "PortListener.hpp"

PortListener::PortListener():
	type(TCPSocket), socket(0), ctx(nullptr) {
	ft_memset(&this->address, 0, sizeof(this->address));
	ft_memset(&this->addr_len, 0, sizeof(this->addr_len));
}

PortListener::~PortListener() {
	this->close();
	if (this->ctx) {
		SSL_CTX_free(this->ctx);
		this->ctx = nullptr;
	}
}

const char*	PortListener::InitException::what() const throw() {
	return (this->str);
}

PortListener::InitException::InitException(const char* str): str(str) {}

void PortListener::init(int port) {
	this->address.sin_port = htons(port);
	this->socket = File::socket(AF_INET, SOCK_STREAM, 0);
	this->addr_len = sizeof(this->address);
	if (this->socket < 0)
		throw InitException("Could not create socket");
	if (this->type == TCPSocket)
		fcntl(this->socket, F_SETFL, O_NONBLOCK);
	// int flags = 1;
	// if (setsockopt(this->socket, SOL_SOCKET, SO_REUSEADDR, (void *)&flags, sizeof(flags)) < 0)
	// 	Logger::error("setsockopt(SO_REUSEADDR) failed");
	int flags = 1;
	if (setsockopt(this->socket, SOL_SOCKET, SO_KEEPALIVE, (void *)&flags, sizeof(flags)) < 0)
		Logger::error("setsockopt(SO_KEEPALIVE) failed");
}

void PortListener::init(int sslPort, std::string const &certPath, std::string const &keyPath) {
	this->type = SSLSocket;
	this->init(sslPort);
	this->initSSL(certPath, keyPath);
}

void PortListener::initSSL(std::string const &certPath, std::string const &keyPath) {
	const SSL_METHOD *method = SSLv23_server_method();
	this->ctx = SSL_CTX_new(method);
	if (!this->ctx)
		throw InitException(ERR_lib_error_string(errno));
	SSL_CTX_set_ecdh_auto(this->ctx, 1);
	if (SSL_CTX_use_certificate_file(this->ctx, certPath.c_str(), SSL_FILETYPE_PEM) <= 0)
		throw InitException(std::string("SSL: Cannot use certificate: ").append(strerror(errno)).c_str());
    if (SSL_CTX_use_PrivateKey_file(this->ctx, keyPath.c_str(), SSL_FILETYPE_PEM) <= 0 )
		throw InitException(std::string("SSL: Cannot use privatekey: ").append(strerror(errno)).c_str());
}

void PortListener::bind(int maxClients) {
	if (::bind(this->socket, (struct sockaddr*)&this->address, this->addr_len) < 0)
		throw InitException("Cannot bind socket");
	if (listen(this->socket, maxClients) < 0)
		throw InitException("Cannot listen");
}

void PortListener::close(void) {
	File::close(this->socket);
	this->socket = -1;
}

void PortListener::fdSet(fd_set &set) {
	FD_SET(this->socket, &set);
}

bool PortListener::fdIsSet(fd_set &set) {
	return (FD_ISSET(this->socket, &set));
}

bool PortListener::isSSL(void) const {
	return (this->type == SSLSocket);
}

SSL_CTX *PortListener::getCtx(void){
	return (this->ctx);
}

std::pair<int, std::string> PortListener::accept(void) {
	std::pair<int, std::string> res;
	res.first = 0;
	struct sockaddr_in addr;
	socklen_t addr_size = sizeof(struct sockaddr_in);
	if ((res.first = File::accept(this->socket, (sockaddr*)&addr, &addr_size)) > 0
		&& this->type == TCPSocket)
		fcntl(res.first, F_SETFL, O_NONBLOCK);
	uint32_t ip = addr.sin_addr.s_addr;
	res.second = to_string((size_t) ip & 0xFF) + "."
		+ to_string((size_t)(ip >> 8)  & 0xFF) + "."
		+ to_string((size_t)(ip >> 16) & 0xFF) + "."
		+ to_string((size_t)(ip >> 24) & 0xFF) + ":" + to_string(ntohs(addr.sin_port));
	return (res);
}

int PortListener::max(int max_fd) const {
	if (this->socket > max_fd) max_fd = this->socket;
	return (max_fd);
}
