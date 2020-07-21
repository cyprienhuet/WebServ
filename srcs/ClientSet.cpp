#include "ClientSet.hpp"
#include <iostream>
#include <sys/time.h>

ClientSet::ClientSet():
	maxSize(0), timeToLive(0), array(nullptr), end(nullptr) {}

ClientSet::~ClientSet() {
	delete[] this->array;
	this->array = nullptr;
	this->end = nullptr;
	this->maxSize = 0;
	this->timeToLive = 0;
}

void ClientSet::init(size_t size, unsigned int TTL) {
	this->array = new Client[size];
	this->end = this->array;
	this->maxSize = size;
	this->timeToLive = TTL;
}

Client* ClientSet::findEmpty() const {
	Client *it = this->array;
	while (it != this->array + this->maxSize && *it != -1)
		it++;
	if (it == this->array + this->maxSize)
		throw std::exception();
	return it;
}

Client* ClientSet::begin() const { return this->array; }

Client* ClientSet::next(Client* it) const {
	if (it == nullptr) {
		it = this->array;
		if (*it > 0)
			return it;
	}
	else it++;

	while (it != this->end && *it == -1)
		it++;

	if (it == this->end)
		return nullptr;
	return it;
}

void ClientSet::addClient(std::pair<int, std::string> const &newClient) {
	try {
		Client* spot = this->findEmpty();
		new (spot) Client(newClient.first);
		if (spot == this->end) this->end++;
		spot->setIp(newClient.second);
		Logger::debug("New client ", spot->getIp(), " (", newClient.first, ")");
	} catch (std::exception) {
		Logger::error("Failed to accept new connection");
		File::close(newClient.first);
	}
}

void ClientSet::addClient(std::pair<int, std::string> const &newClient, SSL_CTX* ctx) {
	try {
		Client* spot = this->findEmpty();
		new (spot) Client(newClient.first, ctx);
		if (spot == this->end) this->end++;
		spot->setIp(newClient.second);
		Logger::debug("New client ", spot->getIp(), " (", newClient.first, ")");
	} catch (std::exception) {
		Logger::error("Failed to accept new connection");
		File::close(newClient.first);
	}
}

void ClientSet::updateEnd() {
	while (this->end != this->array && *(this->end-1) == -1)
		this->end--;
}

/**
 * Adds all clients to the fd_set set, and return the higest fd
 **/
int ClientSet::addToFdSet(fd_set* read, fd_set* write) {
	int max = 0, fd;
	this->updateEnd();
	Client* it = nullptr;
	while ((it = this->next(it))) {
		fd = *it;
		if (!it->skip) {
			FD_SET(fd, read);
			if (fd > max)
				max = fd;
		}
		if (it->hasResponses() && !it->isReady()) {
			FD_SET(fd, write);
			if (fd > max)
				max = fd;
		}
	}
	return (max);
}

/**
 * Remove clients whose TTL has expired
 **/
void ClientSet::removeTimedOut() {
	// Logger::verbose("Checking for timed out...");
	timeval now;
	gettimeofday(&now, NULL);
	Client* it = nullptr;
	while ((it = this->next(it))) {
		if (!it->hasResponses() && now - *it > timeToLive) {
			int since = now - *it;
			Logger::debug("Client ", it->getIp() , " disconnect: TTL expired after ", since, "s");
			it->~Client();
		}
	}
}
