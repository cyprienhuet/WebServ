#ifndef RAWBODY_HPP
# define RAWBODY_HPP

# include "Body.hpp"

class RawBody:
	public Body
{
private:
	const char*	data;
	size_t		size;
	size_t		index;

	RawBody();
public:
	RawBody(const char* buffer, size_t size);
	virtual ~RawBody();

	size_t getBody(char* dest, size_t size);
	size_t getBodySize();
};

#endif /* RAWBODY_HPP */
