#include "Client.hpp"
#include <iostream>
#include <sys/time.h>

Client::operator int() { return this->fd; };
Client::operator int() const { return this->fd; };
Client::operator timeval() { return this->timestamp; };
Client::operator timeval() const { return this->timestamp; };

Client::Client():
	fd(-1), ready(false),ssl(nullptr),
	request(nullptr), buffer(nullptr), body(nullptr),
	ip(), job(nullptr), skip(false) {}

Client::Client(int fd) {
	if (fd < 0) throw std::exception();
	this->fd = fd;
	this->ready = false;
	this->ssl = nullptr;
	this->request = nullptr;
	this->buffer = nullptr;
	this->body = nullptr;
	this->ip.clear();
	gettimeofday(&(this->timestamp), NULL);
	this->skip = false;
};

Client::Client(int fd, SSL_CTX* ctx) {
	if (fd < 0) throw std::exception();
	this->fd = fd;
	this->ready = false;
	gettimeofday(&(this->timestamp), NULL);
	this->request = nullptr;
	this->buffer = nullptr;
	this->body = nullptr;
	this->ip.clear();
	this->ssl = SSL_new(ctx);
	SSL_set_fd(ssl, fd);
	if (SSL_accept(ssl) <= 0) {
		this->~Client();
		throw SSLException(ERR_lib_error_string(errno));
	}
	this->skip = false;
};

Client::~Client() {
	if (this->ssl) {
		SSL_shutdown(this->ssl);
		SSL_free(this->ssl);
		this->ssl = nullptr;
	}
	if (this->fd != -1) {
		Logger::debug("Closing client ", this->ip, " (", this->fd, ")");
		File::close(this->fd);
	}
	this->fd = -1;
	delete this->request;
	delete this->buffer;
	delete this->body;
	if (this->job) this->job->expire();
	this->ready = false;
	this->job = nullptr;
	this->request = nullptr;
	this->buffer = nullptr;
	this->body = nullptr;
	this->ip.clear();
	while (!this->responses.empty()) { // Without this the program would segfault as the list would delete itself several times
		delete this->responses.front();
		this->responses.pop_front();
	}
}

void Client::setIp(std::string const &ip) {
	this->ip = ip;
}

int Client::read(char* buffer, std::size_t size) {
	if (this->ssl)
		return SSL_read(this->ssl, buffer, size);
	return ::recv(*this, buffer, size, MSG_DONTWAIT);
}

int Client::send(const char* buffer, std::size_t size) {
	if (this->ssl)
		return SSL_write(this->ssl, buffer, size);
	return ::send(*this, buffer, size, 0);
}

Request *Client::returnRequest(void) {
	Request* tmp = this->request;
	this->request = nullptr;
	return tmp;
}

void Client::handleChunked(char* buffer, size_t& offset, size_t ret) {
	char *ptr = buffer + offset;

	while (ptr < buffer + ret && status != GO_END) {
		if (status == NEED_SIZE) {
			ptr = ft_atoi_hex_chunked(ptr, this->toRead);
			if (*ptr == '\r') {
				status = NEED_N_1;
				ptr++;
			}
		} else if (status == NEED_N_1 && *ptr == '\n') {
				status = (toRead) ? GO_READ : NEED_R_END;
				ptr++;
		} else if (status == GO_READ) {
			size_t toAdd = buffer + ret - ptr;
			if (toAdd > toRead) toAdd = toRead;
			this->body->addBody(ptr, toAdd);
			if (toRead == toAdd)
				status = NEED_R_2;
			toRead -= toAdd;
			ptr += toAdd;
		} else if (status == NEED_R_2 && *ptr == '\r') {
				status = NEED_N_2;
				ptr++;
		} else if (status == NEED_N_2 && *ptr == '\n') {
				status = NEED_SIZE;
				ptr++;
		} else if (status == NEED_R_END && *ptr == '\r') {
				status = NEED_N_END;
				ptr++;
		} else if (status == NEED_N_END && *ptr == '\n') {
				status = GO_END;
				ptr++;
		}
	}
	offset = ptr - buffer;
}

void Client::handleHeaders(char *buffer, size_t &offset, size_t ret, bool& doRead) {
	static const char CLRF[] = "\r\n\r\n";
	if (!this->buffer)
	{
		// * find is {ABCD[\r]\n\r\n}
		char *find = ::ft_strnstr(buffer + offset, CLRF, ret - offset, 4);
		// If all the headers have been read, parse them
		if (find)
		{
			size_t headerSize = (find - (buffer + offset)) + 4;
			this->request = new Request(std::string(buffer + offset, headerSize), this->isSSL());
			Logger::verbose("########################### Request Headers (Local buffer)\n",
							std::string(buffer + offset, headerSize),
							"###########################");
			// * Move offset so next received will start there
			// * There will be one call since a response is returned
			offset += headerSize;
		}
		else
		{
			this->buffer = new std::string(buffer + offset, ret - offset);
			offset = ret;
		}
	}
	else
	{
		if (!(doRead))
			this->buffer->append(buffer, ret);
		size_t clrfPos = this->buffer->find(CLRF);
		if (clrfPos != std::string::npos)
		{
			Logger::verbose("########################### Request Headers (Client buffer)\n",
							*this->buffer,
							"###########################");
			this->request = new Request(*this->buffer, this->isSSL());
			bool deleteBuffer = true;
			if (clrfPos + 4 < this->buffer->length())
			{
				// If there is no body the following after CLRF is a new Request
				if (this->request && this->request->contentLength() == 0 && this->request->header("Transfer-Encoding").length() == 0)
				{
					this->buffer->erase(0, clrfPos + 3);
					deleteBuffer = false;
					// Set offset of current local buffer to after CLRF if there is a body
				}
				char *find = ::ft_strnstr(buffer + offset, CLRF, ret - offset, 4);
				if (find)
					offset = (find - buffer) + 4;
				else
					offset = ret - (this->buffer->length() - clrfPos - 4);
			}
			else
				offset = ret;
			if (deleteBuffer)
			{
				delete this->buffer;
				this->buffer = nullptr;
			}
		}
		else
			offset = ret;
	}
}

Request* Client::receive(bool& doRead, std::list<StreamJob*>& jobList) {
	static char buffer[R_BUFFER_SIZE];
	static size_t ret = 0;
	// Offset to the local buffer if there is a whole request + some part of the body
	static size_t offset = 0;
	if (!(doRead)) {
		memset(buffer, 0, R_BUFFER_SIZE);
		ret = this->read(buffer, R_BUFFER_SIZE);
		offset = 0;
	}

	// Return nothing on -1 or 0 since the connection is closed or waiting
	if (ret == 0 || ret == static_cast<size_t>(-1)) {
		return doRead = false, this->~Client(), nullptr;
	}
	if (offset >= ret)
		return doRead = false, (nullptr);

	if (!this->request)
		this->handleHeaders(buffer, offset, ret, doRead);

	if (this->request
		&& this->request->header("Transfer-Encoding").length() == 0
		&& this->request->contentLength() == 0) {
		doRead = (ret != offset);
		return (this->returnRequest());
	// * Return if there is both chunked and Content-Length
	} else if (this->request
			&& this->request->header("Transfer-Encoding") == "chunked"
			&& this->request->contentLength() > 0) {
		doRead = (ret != offset);
		return (this->returnRequest());
	// * Return on a Expect: 100-Continue
	} else if (this->request && this->request->header("Expect") == "100-continue") {
		doRead = (ret != offset);
		return (this->request);
	}

	if (this->request) {
		if (!this->body) {
			size_t size = ret - offset;
			if (this->request->header("Transfer-Encoding") == "chunked") {
				this->body = new BodyList();
				this->toRead = 0;
				this->status = Client::NEED_SIZE;
				this->handleChunked(buffer, offset, ret);
				if (this->status == Client::GO_END) {
					this->body->done = true;
					doRead = (ret != offset);
					if (this->body->totalReceivedSize > 0) {
						this->request->streamBody = new StreamBody(*this, jobList);
						return (this->request);
					} else {
						this->deleteBody();
						return (this->returnRequest());
					}
				}
			}
			else if (size >= this->request->contentLength()) {
				if (size < ret)
					this->buffer = new std::string(buffer + offset + size, ret - offset - size);
				this->body = new BodyList();
				this->body->done = true;
				this->body->addBody(buffer + offset, std::min(size, this->request->contentLength()));
				this->request->streamBody = new StreamBody(*this, jobList);
				offset += std::min(size, this->request->contentLength());
				doRead = (ret != offset);
				return (this->request);
			} else {
				this->body = new BodyList();
				this->body->addBody(buffer + offset, size);
				offset += size;
			}
		} else {
			if (this->request->header("Transfer-Encoding") == "chunked") {
				this->handleChunked(buffer, offset, ret);
				if (this->status == Client::GO_END) {
					this->body->done = true;
					if (this->body->totalReceivedSize == 0) {
						this->deleteBody();
						doRead = (ret != offset);
						return (this->returnRequest());
					}
				}
			} else {
				size_t toAdd = this->request->contentLength() - this->body->totalReceivedSize;
				if (toAdd > ret)
					toAdd = ret;
				this->body->addBody(buffer, toAdd);
				offset += toAdd;
				if (this->body->totalReceivedSize >= this->request->contentLength()) {
					this->body->done = true;
				}
			}
			if (!this->request->streamBody && this->body->totalReceivedSize > 0) {
				this->request->streamBody = new StreamBody(*this, jobList);
				return (this->request);
			}
			if (this->body->currentSize > 8388608) {
				this->skip = true;
			}
		}
	}

	doRead = (ret != offset && !this->skip);
	return (nullptr);
}

bool Client::isSSL(void) const { return this->ssl ? true : false; }

bool Client::isReady(void) const {
	return (this->ready);
}

void Client::setReady(bool val) {
	this->ready = val;
}

/**
 * Updates the last-seen timestamp of the Client
 **/
void Client::keepAlive(void) {
	// std::cout << "Keeping alive client " << this->getIp() << std::endl;
	gettimeofday(&(this->timestamp), NULL);
}

unsigned int operator-(const timeval&left, const timeval& rigth) {
	return (left.tv_sec - rigth.tv_sec);
}

Client::SSLException::SSLException(const char* str) : str(str) {}
const char*	Client::SSLException::what() const throw() { return this->str; }

void Client::pushResponse(Response* resp) {
	this->responses.push_back(resp);
}

void Client::insertResponse(Response* resp) {
	this->responses.push_front(resp);
}

bool Client::hasResponses(void) const {
	return (!this->responses.empty() && !(this->responses.front()->getDontSend()));
}

Response const *Client::nextResponse(void) const {
	return (this->responses.front());
}

int Client::sendResponse(void) {
	int ret = this->responses.front()->send(*this);
	if (ret) {
		delete this->responses.front();
		this->responses.pop_front();
	}
	return (ret);
}

std::string const &Client::getIp(void) const {
	return (this->ip);
}

BodyList* Client::getList() const {
	return this->body;
}

void Client::deleteBody() {
	delete this->body;
	this->body = nullptr;
}

void Client::setJob(StreamJob* job) {
	this->job = job;
}
