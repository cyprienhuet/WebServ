#include "RawBody.hpp"

RawBody::RawBody(const char* buffer, size_t size) : data(buffer), size(size), index(0) {}

RawBody::~RawBody() {
	delete this->data;
}

size_t RawBody::getBody(char* dest, size_t size) {
	if (this->index == this->size)
		return 0;
	if (this->index + size < this->size) {
		ft_memcpy(dest, this->data + index, size);
		index += size;
		return size;
	} else {
		size = this->size - this->index;
		ft_memcpy(dest, this->data + index, size);
		this->index = this->size;
		return size;
	}
}

size_t RawBody::getBodySize() {
	return this->size;
}
