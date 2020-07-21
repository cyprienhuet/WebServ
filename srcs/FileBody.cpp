#include "FileBody.hpp"

FileBody::FileBody(int fd, size_t size):
	fd(fd), size(size) {}

FileBody::~FileBody() {
	File::close(this->fd);
}

int FileBody::getFd(void) const {
	return (this->fd);
}

size_t FileBody::getBody(char* dest, size_t size) {
	return read(this->fd, dest, size);
}

size_t FileBody::getBodySize() {
	return this->size;
}
