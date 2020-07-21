#include "BodyList.hpp"

BodyList::BodyList():
	std::list<BodyPair>::list(),
	totalReceivedSize(0), currentSize(0), done(false) {}

BodyList::~BodyList() {
	for (auto it = this->begin(), end = this->end(); it != end; it++)
		delete it->data;
	this->clear();
}

void BodyList::addBody(char* body, size_t size) {
	if (!this->empty() && this->back().size < 4096) {
		BodyPair &back = this->back();
		char* copy = (char*)operator new (back.size + size);
		ft_memcpy(copy, back.data, back.size);
		ft_memcpy(copy + back.size, body, size);
		delete back.data;
		this->back().data = copy;
		this->back().size += size;
	} else {
		char* copy = (char*)operator new (size);
		ft_memcpy(copy, body, size);
		this->push_back(BodyPair());
		this->back().data = copy;
		this->back().size = size;
	}
	this->totalReceivedSize += size;
	this->currentSize += size;
}

char* BodyList::join() {
	char* ret = (char*)operator new (this->totalReceivedSize);
	char* ptr = ret;
	for (auto it = this->begin(), end = this->end(); it != end; ++it) {
		ft_memcpy(ptr, it->data, it->size);
		ptr += it->size;
	}
	return ret;
}
