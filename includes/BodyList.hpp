#ifndef BODYLIST_HPP
# define BODYLIST_HPP

# include <list>
# include <cstring>
# include "Utility.hpp"
struct BodyPair {
	char* data;
	size_t size;
};

class BodyList : public std::list<BodyPair> {
public:
	size_t totalReceivedSize;
	size_t currentSize;
	bool done;

	BodyList();
	virtual ~BodyList();

	void addBody(char* body, size_t size);
	char* join();
};


#endif /* BODYLIST_HPP */
