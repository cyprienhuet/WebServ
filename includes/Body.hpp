#ifndef BODY_HPP
# define BODY_HPP

# include <cstddef>
# include "Utility.hpp"

class Body {
public:
	virtual size_t getBody(char*, size_t) = 0;
	virtual size_t getBodySize() = 0;
	virtual ~Body() {};
};

#endif /* BODY_HPP */
