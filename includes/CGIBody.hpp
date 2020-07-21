#ifndef CGIBODY_HPP
# define CGIBODY_HPP

# include <sys/stat.h>
# include <sys/wait.h>
# include <unistd.h>
# include "FileBody.hpp"
# include "File.hpp"

class CGIBody:
	public FileBody
{
private:
    std::string	buffer;
public:
	CGIBody(int fd);
	virtual ~CGIBody();

	void setBuffer(std::string const &strBuffer);
	size_t getBody(char* dest, size_t size);
};

#endif /* CGIBODY_HPP */
