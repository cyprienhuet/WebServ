#ifndef FILEBODY_HPP
# define FILEBODY_HPP

# include "Body.hpp"
# include <sys/stat.h>
# include <unistd.h>
# include "File.hpp"

class FileBody:
	public Body
{
protected:
	int		fd;
	size_t	size;

	FileBody();
public:
	FileBody(int fd, size_t size);
	virtual ~FileBody();

	int getFd(void) const;
	size_t getBody(char* dest, size_t size);
	size_t getBodySize();
};

#endif /* FILEBODY_HPP */
