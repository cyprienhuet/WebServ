#include "CGIBody.hpp"

CGIBody::CGIBody(int fd):
    FileBody(fd, 0), buffer() {}

CGIBody::~CGIBody() {}

void CGIBody::setBuffer(std::string const &strBuffer) {
    this->buffer = strBuffer;
}

size_t CGIBody::getBody(char* dest, size_t size) {
    if (this->buffer.length() > 0) {
        if (this->buffer.length() > size) {
            ft_memcpy(dest, this->buffer.c_str(), size);
            this->buffer.erase(0, size);
            return (size);
        } else {
            ft_memcpy(dest, this->buffer.c_str(), this->buffer.length());
            size_t ret = this->buffer.length();
            this->buffer.clear();
            return (ret);
        }
    }
	return (read(this->fd, dest, size));
}
